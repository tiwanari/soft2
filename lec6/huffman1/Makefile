CC = gcc
CFLAGS = -Wall -g
OBJS = main.o encode.o decode.o util.o

a.out:	$(OBJS)
	$(CC) $(OBJS) $(LDLIBS)

clean:
	rm -f *~ *.o a.out