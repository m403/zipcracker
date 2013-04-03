CC=gcc
CFLAGS_32=-Wall -O -Izlib-1.2.7_32/
CFLAGS_64=-Wall -O -Izlib-1.2.7_64/

TARGET_FOLDER_32=zlib-1.2.7_32/contrib/minizip/
TARGET_FOLDER_64=zlib-1.2.7_64/contrib/minizip/

ZIPC_OBJS_32 = zipcracker.o $(TARGET_FOLDER_32)unzip.o $(TARGET_FOLDER_32)ioapi.o zlib-1.2.7_32/libz.a
ZIPC_OBJS_64 = zipcracker.o $(TARGET_FOLDER_64)unzip.o $(TARGET_FOLDER_64)ioapi.o zlib-1.2.7_64/libz.a


# modificare per 32
.c.o:
	$(CC) -c $(CFLAGS_32) $*.c -o $@

all: zipcracker32 zipcracker64

zipcracker32:  $(ZIPC_OBJS_32)
	$(CC) $(CFLAGS_32) -o $@ $(ZIPC_OBJS_32)

zipcracker64:  $(ZIPC_OBJS_64)
	$(CC) $(CFLAGS_64) -o $@ $(ZIPC_OBJS_64)
clean:
	/bin/rm -f *.o *~ zipcracker32 zipcracker64
