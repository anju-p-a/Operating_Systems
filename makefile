programs = oss child

all:$(programs)
child:child.o 
	gcc -lpthread -o $@ $<
oss:oss.o 
	gcc -lpthread -o $@ $<
%.o:%.c
	gcc -c -g $<  

clean:
	/bin/rm -f *.o $(programs)

