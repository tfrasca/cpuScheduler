CC=gcc
CFLAGS=-I -I rb_tree -Wall -g
# DEPS = rb_tree/red_black_tree.h
OBJS = scheduler.o rb_tree/red_black_tree.o rb_tree/stack.o rb_tree/misc.o

# rb_tree: 
# 	$(MAKE) -C rb_tree/

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

scheduler:  $(OBJS)
	$(CC) -o scheduler $(OBJS)

processes: processes.o
	$(CC) -o processes processes.o $(lib)

clean:
	rm -f *.o core* *~ scheduler processes
