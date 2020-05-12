#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h>
#include <unistd.h> 
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>

int shm_id2,shm_id1,shm_id3;        
char *shm_ptr2;
char *shm_ptr1;
char *shm_ptr3;
sem_t *sem1;
char *semName1;
sem_t *sem3;
char *semName3;
#define MAXCHILD 18
#define CHILDCOUNT 1
int count;
void  userSignalHandle(int);
int addressIndex(long int b);
int generateRandom(int min,int max);
long int clockTime(int seconds,int nanoSeconds);
int findSeconds(int time);
int findNanoSeconds(int time);
void clearSharedMemory(int shm_id,char *shm_ptr);
int forkchildren(int count);
int createdProcesses[100];
char bitVector(char type,int index,int arrayValue);
int throughputCalculator(int pageCount);
int clockSpeed(char speed);
struct queue{
	int pid;
	int resource;
	int instance;
	};
struct queue queue1 [20][18];                                                        // MAIN PROGRAM BEGINS 
int front[20],rear[20];
static int childCount1 = 0;
// MAIN PROGRAM BEGINS//
////////////////////////////////////////////////////////////////////////////////////////////////

 int main(int argc, char* argv[]) {
	/**** declaring variables to hold parameters passed in getopt and clock,initialisations*/
	signal(SIGINT,userSignalHandle);
	time_t start = time(NULL);
 	int  slaveNO,timeOut;
	char *logName;  
	int opt; 
	count = 0;
	extern int optopt;
	int l;
   	for(l = 0;l<20;l++){
	front[l] = -1;
	rear[l]= -1;
	}
   /*****PARSING OPTIONS USING GETOPTS*************/

   	while ((opt = getopt(argc, argv,":hs:")) != -1) {
                count ++;
     		switch(opt) {
               		case 'h' :
                	printf("oss -s argument1\n Note that argument 1  is an  integer specifying processes\n");
			return 0;
			break;
        		case 's' : 
            		slaveNO  = atoi(optarg);
            		break;
		
                	case ':' :
                	printf( "option is specified incorrectly ; specify that as oss -s int_NO\n");
                	break;

        		case '?' :
			default:
                	printf( "option is specified incorrectly ; specify that as oss -s int_NO\n");
             		break; 
                 }
      	}                              

    /****** default values assumed if no parameters are passed ****/    

	 if(count == 0) {                                                             
		printf(" No options were specified assuming default values \n") ;   
		logName = "ossLogFile";
		slaveNO = MAXCHILD;
		timeOut = 10;
 		}else {
	if(slaveNO <= 0){
		slaveNO = MAXCHILD;
	}
	if(slaveNO>MAXCHILD){
		slaveNO = MAXCHILD;
	}
		timeOut = 10;
		logName = "ossLogFile";
	}	
	 printf(" Slave nos as  %d and timeOut  as %d and logname is %s\n", slaveNO , timeOut,logName ); 


   /** SHARED MEMORY ALLOCATION BETWEEN OSS AND CHILD **/
 
 /** A MESSAGE STRUCT TO COMMUNICATE BETWEEN  OSS TO CHILD **/ 
	struct msg{                                      
		long int byteAddress;
		int memoryOperation;
		int pid;
		int timeSeconds;
		int timeNanoSeconds;
		char msgType;
		char semName[15];
		char isTerminated;
		int  isAllocated; // default it is zero; if resource is allocated it is set to one and -1 if not allocated;
	};
   	struct msg *childMessage;
        
   	key_t key3;                                                            
   	key3 =   ftok(".",'r');
        
        shm_id3 = shmget(key3,sizeof(struct msg) , IPC_CREAT|0666);

   	if(shm_id3 < 0){
   		perror("Error in shmget master\n");
		return 1;
   	} else { 
   		
		shm_ptr3 = (char *) shmat(shm_id3 ,NULL,0);
   			if((uintptr_t) shm_ptr3 == -1) {
    				perror("Error in shmat master\n");
                		clearSharedMemory(shm_id3,shm_ptr3);
  			}
		}
  
	childMessage  = (struct msg *) shm_ptr3;         
 	childMessage->isAllocated = 0; 
	childMessage->isAllocated = -1; 
 	
  /*****SHARED MEMORY ALLOCATION FOR CLOCK USING A STRUCT **********/
 
	struct clockShared {                                      
	int seconds;
	int nanoSeconds;
	};
   	struct clockShared *myClock;
        
   	key_t key1;                                                            
   	key1 =   ftok(".",'b');
        
        shm_id1 = shmget(key1,sizeof(struct clockShared) , IPC_CREAT|0666);

   	if(shm_id1 < 0){
   		perror("Error in shmget master\n");
		return 1;
   	} else { 
   		
		shm_ptr1 = (char *) shmat(shm_id1 ,NULL,0);
   			if((uintptr_t) shm_ptr1 == -1) {
    				perror("Error in shmat master\n");
                		clearSharedMemory(shm_id1,shm_ptr1);
  			}
		}
  
	myClock  = (struct clockShared *) shm_ptr1;         
  
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
   		perror("Error in shmget of page table  at master\n");
		return 1;
   	} else { 
		shm_ptr2 = (char *) shmat(shm_id2 ,NULL,0);
   			if((uintptr_t) shm_ptr2 == -1) {
    				perror("Error in shmat of page table  at master\n");
                		clearSharedMemory(shm_id2,shm_ptr2);
  			}
		}
  
	pageTable  = (struct page *) shm_ptr2;
	int i,j;
	int randomInstance;
	srand(time(NULL));	                                                                                    // creating required number of child processes
	for(j=0;j<8;j++){
		pageTable->bitArray[j] = 0;
		pageTable->validBit[j] = 0;
		pageTable->referenceBit[j] = 0;
		pageTable->dirtyBit[j] = 0;
			}

		
        /******* Initialising oss clock to zero *********/
	 
	myClock->seconds = 0;       
	myClock->nanoSeconds = 0; 
	/****initializations for semaphore ***/

	//semaphore to maintain synchronization for page references from child//	
	semName1 = "puthenveetil";
	sem1 = sem_open(semName1, O_CREAT , 0666,111);	
	if (sem1 == SEM_FAILED) { perror( " Error at semopen in parent ");}
	//semaphore for clock
	//
	semName3 = "clock-puthenveetil";
        sem3 = sem_open(semName3, O_CREAT , 0666,111);
        if (sem3 == SEM_FAILED) { perror( " Error at semopen in parent ");}
	
	//semaphore to keep the child on queue on pagefault
	char *semName2;
	sem_t *sem2;

       /**********Initialising the variables used by the parent in forking the children and terminating them *********/
	int returnChild;
        int childCount = 0; 
        int waitReturn;
	int status;
	pid_t pid;
	int pid_parent;
        int a ;
	createdProcesses[0] = 0;
	i = 0;
        count = 0;
	long int lastForkTime = 0;
	int timeGap = 0;
	char speed = 's';	
	int fileLineCount = 0;
	int timeForPrint = 0;
	/*** Opening the Log file ***/	
        FILE *fp = fopen(logName,"w");
        if(fp == 0) {
     		perror("File Error while opening");
        }
       /***** OSS CLOCK IMPLEMENTATION****/ 
	while(1){          
		
		if((time(NULL) - start)<timeOut){
			sem_wait(sem3);
			int temp = clockSpeed('s');
			myClock->nanoSeconds =(int)( myClock->nanoSeconds) + temp; // initially as 10 and increased speed when queue gets filled
			if (myClock->nanoSeconds >= 1000000000){
				myClock->seconds =(int)( myClock->seconds)+1;
				myClock->nanoSeconds = 0;
					}
			sem_post(sem3);	
			long int t1 = clockTime((int)(myClock->seconds),(int)(myClock->nanoSeconds));
		//********************forking the processes******************//  
			if(timeGap  < (t1 - lastForkTime) && (count< CHILDCOUNT)){
				lastForkTime = clockTime((int)(myClock->seconds),(int)(myClock->nanoSeconds));
				timeGap =  generateRandom(1,500) * 1000000;
				pid = fork();
				if(pid == 0){
                        		int returnChild = execl("child",(char *)NULL);                      //using exec and cheking for any errors
					return 0;
                        	        if (returnChild == -1){perror("exec failed");} 
					}else if(pid > 0) 
						{
						createdProcesses[count] = pid;
						count++;
							}
						else{
                        	               		perror(" Error creating child \n");
							}
				}
		
	//********************checking for page requests,placing processes in the queue,setting valid bit,dirty bit is done here *****//
		if(childMessage->msgType == 'r'){//there is a memory reference
			char example[15];
			sprintf(example,"%s",childMessage->semName);
			childMessage->isAllocated = 1;
			while(childMessage->msgType == 'r');
			semName2 = example;
       			sem2 = sem_open(semName2, 0);
        		if (sem2 == SEM_FAILED) { perror( " Error at semopen in parent sem2");}
			sem_wait(sem2);
			int reference = addressIndex(childMessage->byteAddress);
			int ind = reference/8;
			int arrayV = pageTable->validBit[ind];
			if(bitVector('t',reference,arrayV)=='t'){
				if(childMessage->memoryOperation = 'w'){
					arrayV = pageTable->dirtyBit[ind];
					bitVector('s',reference,arrayV);
				}
			
				arrayV = pageTable->referenceBit[ind];
				if(bitVector('t',reference,arrayV)=='f'){
					bitVector('s',reference,arrayV);
				}
				childMessage->isAllocated = -1;
				sem_post(sem2);
			}
		}
		childMessage->msgType = 'm';
		if(childMessage->isTerminated == 't'){
			fprintf(fp," child with pid %d got terminated here\n",childMessage->pid);
			childMessage->msgType = 'n';
			}
		
		//check for queue and set valid bit as true 
		
		
		childMessage->isAllocated = -1;
		//printing every second
		if(myClock->nanoSeconds - timeForPrint > 10){
			
			if(fileLineCount<100000){
				fprintf(fp,"Allocation of frames is ");
				fileLineCount++;
				for(i = 0 ;i< 255;i++){
					if(bitVector('t',i,pageTable->validBit[i/8]) == 't'){
						fprintf(fp,"A"); 
					}else {
						fprintf(fp,".");
					}
					
				}
				fprintf(fp,"\n");
			}
		}
		timeForPrint = myClock->nanoSeconds;
                        /**** forking new child processes  on termination of children *******/
			int k = 0;
			int iteration = 0;
			for( i = 0;i<count && count<slaveNO  ;i++){
				if((k = waitpid(createdProcesses[i], &status, WNOHANG)) > 0) {
					childCount++;
						if(childCount-- >0  ) {
							pid = fork();
								if(pid == 0){
                                        				int returnChild = execl("child",(char *)NULL);                      //using exec and cheking for any errors
								
                                        					if ( returnChild == -1) { perror("exec failed");} 
									}else if(pid > 0) {
										
										count++;
										int s = count-1;
										createdProcesses[s] = pid;
									}else {
										perror("Error creating child");
										}
							break;
						}}
				}
			
		}else {         /***terminating on condition failure***/
		//	fprintf(fp,"Termination occured as count is %d or timeOut occured and msg is %c \n",(int)myClock->seconds,count,j,childMessage->msgType);
				for(i = 0;i< count;i++){
        	   			kill(createdProcesses[i], SIGKILL);	
			}
			break;		
		}
	}
	fclose(fp);
        clearSharedMemory(shm_id1,shm_ptr1);
	clearSharedMemory(shm_id2,shm_ptr2);
	clearSharedMemory(shm_id3,shm_ptr3);
	sem_close(sem1);
	sem_close(sem3);
	sem_unlink(semName1);
	sem_unlink(semName3);
	
		

  return 0;
}
  
//function to clear shared memory
void clearSharedMemory(int shm_id,char *shm_ptr){         
  	shmdt(shm_ptr);
  	shmctl(shm_id,IPC_RMID, NULL);
	}

//function to handle ctl+c interrupt
void userSignalHandle(int ctrlC){
	int i;
	sem_close(sem1);
	sem_close(sem3);
	sem_unlink(semName1);
	sem_unlink(semName3);
	clearSharedMemory(shm_id1,shm_ptr1);
  	clearSharedMemory(shm_id2,shm_ptr2);
	clearSharedMemory(shm_id3,shm_ptr3);
	for(i = 0;i<count;i++){
	kill(createdProcesses[i],SIGKILL);
	}
	exit(0);
}
//function to give the seconds

int  findSeconds(int time){
	int t = (int) (time/1000000000);
	return t;
	}
//function to give nanoSeconds	                                                         // MAIN PROGRAM BEGINS 
int  findNanoSeconds(int time){
	int t = (int) (time % 1000000000);
	return t;
	}
//function to generate random numbers
int generateRandom(int min,int max){
	int total = max-min+1;
	int number = (rand() % total) + min;
//	printf(" random number is %d\n", number);
	return number;

}

//function to give time as sum of seconds and nanoSeconds
long int clockTime(int seconds,int nanoSeconds){
	long int time = seconds + nanoSeconds;
	return time;
}


int addressIndex(long int b){
	int mod,index=0;
	int k = 1;
	 while (b > 0)
   	 {
        mod = b % 10;
        index = index + mod*k;
        b =b/10 ;
        k = k * 2;
    }
return index;

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

int clockSpeed(char speed){
	if(speed == 'f'){
		return 1000;
	}
	if(speed == 's'){
		return 10;
	}
}


