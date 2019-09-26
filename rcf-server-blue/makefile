CC		:= gcc
LINKER		:= gcc -o
CFLAGS		:= -c -Wall -O2
INCLUDEBLUE := -I .  -I  ../rcf-bbb-01/ -I /usr/include
LFLAGS		:= -L /usr/lib/ -lm -lrt -lpthread -lroboticscape -lm



rcf-serv-blue.o: rcf-serv-blue.c
	$(CC) $(INCLUDEBLUE) -c rcf-serv-blue.c -o rcf-serv-blue.o

rcf-serv-blue: rcf-serv-blue.o
	$(LINKER) $(CFLAGS) rcf-serv-blue.o  $(LFLAGS) -o $(@)


clean: 
	rm rcf-serv-blue.o \
	rm rcf-serv-blue

all: rcf-serv-blue
	

