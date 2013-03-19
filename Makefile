CC=gcc
CFLAGS=-Wall -O -Izlib-1.2.7/

TARGET_FOLDER=zlib-1.2.7/contrib/minizip/

ZIPC_OBJS = zipcracker.o $(TARGET_FOLDER)unzip.o $(TARGET_FOLDER)ioapi.o $(TARGET_FOLDER)libz.a

.c.o:
	$(CC) -c $(CFLAGS) $*.c -o $@ $(TARGET_FOLDER)

all: zipcracker

zipcracker:  $(ZIPC_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ZIPC_OBJS)

clean:
	/bin/rm -f *.o *~ zipcracker
