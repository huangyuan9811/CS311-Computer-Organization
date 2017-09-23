CC=gcc

HEADERS= parseline.h table.h

default: runfile

parseline.o: parseline.c $(HEADERS) 
	gcc -c parseline.c -o parseline.o

table.o: table.c $(HEADERS)
	gcc -c table.c -o table.o

runfile.o: main.c $(HEADERS)
	gcc -c main.c -o runfile.o 




runfile: runfile.o parseline.o table.o
	$(CC) runfile.o parseline.o table.o -o runfile

clean:
	rm -f *.o
	rm -f runfile
