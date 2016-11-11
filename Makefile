CC=gcc
CFLAGS=-I -Wall -g

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

scheduler:  scheduler.o
	$(CC) -o scheduler scheduler.o

processes: processes.o
	$(CC) -o processes processes.o $(lib)

clean:
	rm -f *.o core* *~ scheduler processes
