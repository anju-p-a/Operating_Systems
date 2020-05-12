
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h> 
#include <sys/shm.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
char *semName2;
sem_t *sem2;
int shm_id;
char *shm_ptr;
        int shm_id2;        
        char *shm_ptr2;
	int shm_id3;        
        char *shm_ptr3;
void clearSharedMemory(){
	shmdt(shm_ptr);
	shmctl(shm_id,IPC_RMID, NULL);
	}
void userSignalHandle(int);
int clockSpeed(char speed);
long int generateByteAddress(int a);
int generateRandom(int min,int max);
char bitVector(char type,int index,int arrayValue);
float clockTime(int seconds, int nanoSeconds);
int childEndTime(int seconds,int nanoSeconds);
int main(int argc, char* argv[]) {
	
	signal(SIGKILL,userSignalHandle);
//SHARED MEMORY ALLOCATION FOR COMMUNICATING BETWEEN OSS AND CHILD
	struct msg{                                      
		long int byteAddress;
		int memoryOperation;
		int pid;
		int timeSeconds;
		int timeNanoSeconds;
		char msgType;
		char  semName[15];
		char isTerminated;
		int  isAllocated; 
	};
   	struct msg *childMessage;
   	key_t key3;                                                            
   	key3 =   ftok(".",'r');
        shm_id3 = shmget(key3,sizeof(struct msg) , IPC_CREAT|0666);
   	if(shm_id3 < 0){
   		perror("Error in shmget of message child\n");
		return 1;
   	} else { 
		shm_ptr3 = (char *) shmat(shm_id3 ,NULL,0);
   			if((uintptr_t) shm_ptr3 == -1) {
    				perror("Error in shmat of message child\n");
                		clearSharedMemory(shm_id3,shm_ptr3);
  			}
		}
	childMessage  = (struct msg *) shm_ptr3;         
	
   	/*************** SHARED MEMORY ALLOCATION FOR OSS CLOCK ***************/
   	struct clockShared {              
   		int seconds;
   		int  nanoSeconds;
   	};
   	struct clockShared *myClock;

	
	key_t key;
	key =   ftok(".",'b');
   	shm_id = shmget(key,sizeof(struct clockShared) ,IPC_CREAT|0666);
   	if(shm_id < 0){
   		perror("Error in shmget of clock child\n");
		return 1;
   	} else { 
   		shm_ptr = (char *) shmat(shm_id ,NULL,0);
   			if((uintptr_t) shm_ptr == -1) {
    				perror("Error in shmat of clock  child\n");
                		clearSharedMemory();
  			}
		}
   
   	myClock  = (struct clockShared *) shm_ptr;
   

   /*****SHARED MEMORY ALLOCATION FOR  PAGE TABLES  USING A STRUCT **********/
	
	
	struct page{                                      
	int bitArray[8];
	int validBit[8];
	int referenceBit[8];
	int dirtyBit[8];
	int pageCount;
	};
   	struct page  *pageTable;
   	key_t key2;                                                            
   	key2 =   ftok(".",'c');
        shm_id2 = shmget(key2,sizeof(struct page), IPC_CREAT|0666);
   	if(shm_id2 < 0){
   		perror("Error in shmget of page table  at child\n");
		return 1;
   	} else { 
		shm_ptr2 = (char *) shmat(shm_id2 ,NULL,0);
   			if((uintptr_t) shm_ptr2 == -1) {
    				perror("Error in shmat of page table  at child\n");
                		clearSharedMemory(shm_id2,shm_ptr2);
  			}
		}
  
	pageTable  = (struct page *) shm_ptr2;
	srand(getpid());
	/** SEMAPHORES USED******************/
	char *semName1 = "puthenveetil";
	sem_t *sem1 = sem_open(semName1, 0);	
	if (sem1 == SEM_FAILED) { perror( " Error at semopen of sem1 in child");}
        /*********/
	char *semName3 = "clock-puthenveetil";
	sem_t *sem3 = sem_open(semName3, 0);	
	if (sem3 == SEM_FAILED) { perror( " Error at semopen of sem1 in child");}
	/*********/
	int rem = getpid()%18;	
	char example[15];
	sprintf(example,"%d-puthen" , rem);
	semName2 = example;
        sem2 = sem_open(semName2, O_CREAT , 0666,111);    
        if (sem2 == SEM_FAILED) { perror( " Error at semopen of  sem2  in child ");}	
	/****PAGE TABLE   AND MANAGEMENT***/
	int pageT[32];
	int initialTime = (int) (myClock->seconds);
	// checking for the page 
	int currentTime = initialTime;
	float startTime = clockTime((int)myClock->seconds,(int)(myClock->nanoSeconds));
	char speed = 's';
	while(1){
	if(currentTime - initialTime < 1)
	{
		/**clock in child ***/ 
		sem_wait(sem3);
			int temp = clockSpeed('s');
			myClock->nanoSeconds =(int)( myClock->nanoSeconds) + temp; // initially as 10 and increased speed when queue gets filled
			if (myClock->nanoSeconds >= 1000000000){
			myClock->seconds =(int)( myClock->seconds)+1;
			myClock->nanoSeconds = 0;
				}
		sem_post(sem3);

		//generating a memory reference and what operation it wants to perform
		long int x = generateRandom(0,255);
			
		//communicating with the oss
		float actionTime = 0.015;
		startTime = clockTime((int)myClock->seconds,(int)(myClock->nanoSeconds));
		currentTime =  (int) (myClock->seconds);
		sem_wait(sem1);
		sprintf(childMessage->semName,"%s",example);
		childMessage->byteAddress = generateByteAddress(generateRandom(0,255));
		if(generateRandom(0,1) == 0){
			childMessage->memoryOperation = 'r';
		}else{
			childMessage->memoryOperation = 'w';
		} 
		childMessage->timeSeconds = (int) (myClock->seconds);
		childMessage->timeNanoSeconds = (int) (myClock->nanoSeconds);
		childMessage->msgType = 'r';		
		while(childMessage->isAllocated == -1);
		childMessage->msgType = 'm';
		while(childMessage->isAllocated != -1);
 		sem_post(sem1);
		sem_wait(sem2);		
		sem_post(sem2);
	}else {
		sem_wait(sem1);
		childMessage->isTerminated = 't';
		childMessage->pid = getpid();
		while(childMessage->msgType == 'r' || childMessage->msgType == 'm');
		childMessage->isTerminated = 'f';
		sem_post(sem1);
		break;
	}
	
	}
	sem_close(sem2);
	sem_unlink(semName2);
  	return 0;


}
/*** functions to calculate clockTime and childendTime in nanoseconds ****/

//function to generate random numbers
int generateRandom(int min,int max){
	int total = max-min+1;
	int number = (rand() % total) + min;
	//printf(" random number is %d\n", number);
	return number;

}

//function to give time as sum of seconds and nanoSeconds
float  clockTime(int seconds,int nanoSeconds){
	float time = (float) (seconds)  + ((float)(nanoSeconds))/1000000000;
	return time;
}
int childEndTime(int seconds,int nanoSeconds) {
	int timeLeft = clockTime(seconds,nanoSeconds);
	return timeLeft + ((rand() % 1000000)) + 1;
}
long int generateByteAddress(int a){
	if(a==0){
		return 0;
	}else {
		return 10*generateByteAddress(a/2)+(a%2);
		}

}

void userSignalHandle(int ctrlC){
	sem_close(sem2);
	sem_unlink(semName2);
	exit(0);
}

int clockSpeed(char speed){
	if(speed == 'f'){
		return 1000;
	}
	if(speed == 's'){
		return 10;
	}
}
char bitVector(char type,int index,int arrayValue){
	int ind = index/8;
	int pos = index%32;
	unsigned int flag = 1;
	flag = flag << pos;
//set the bit
	if(type == 's'){ 
		arrayValue = arrayValue|flag;		
	}
//clear the bit
	 if(type == 'c'){ 
		flag = ~flag;
		arrayValue = arrayValue&flag;	
        }
//test if set
	 if(type == 't'){ 
		if(arrayValue & flag){
			return 't';
        	}else{
			return 'f';
			}
	}
	return 'b';
}
