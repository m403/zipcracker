CC=gcc
CFLAGS=-Wall -O2

ZIPC_OBJS = zipcracker.o argparse.o /usr/lib/libzip.so 

.c.o:
	$(CC) -c $(CFLAGS) $*.c -o $@

all: zipcracker

zipcracker:  $(ZIPC_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ZIPC_OBJS)
clean:
	/bin/rm -f *.o *~ zipcracker
