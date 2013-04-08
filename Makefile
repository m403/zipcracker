CC=gcc
CFLAGS_32=-Wall -O2 -Izlib-1.2.7_32/ -D HAVE_BZIP2 
CFLAGS_LIBZIP=-Wall -O2 -march=native

TARGET_FOLDER_32=zlib-1.2.7_32/contrib/minizip/

ZIPC_OBJS_32 = zipcracker.o $(TARGET_FOLDER_32)unzip.o $(TARGET_FOLDER_32)ioapi.o zlib-1.2.7_32/libz.so
ZIPC_OBJS_ULTIMATE = zipcracker_ultimate.o $(TARGET_FOLDER_32)unzip.o $(TARGET_FOLDER_32)ioapi.o zlib-1.2.7_32/libz.so $(TARGET_FOLDER_32)libbz2.a 

ZIPC_OBJS_LIBZIP = zipcracker_libzip.o /usr/lib/libzip.so 

# modificare per 32
.c.o:
	$(CC) -c $(CFLAGS_32) $*.c -o $@

all: zipcracker32 zipcracker_libzip zipcracker_ultimate

zipcracker32:  $(ZIPC_OBJS_32)
	$(CC) $(CFLAGS_32) -o $@ $(ZIPC_OBJS_32)

zipcracker_ultimate:  $(ZIPC_OBJS_ULTIMATE)
	$(CC) $(CFLAGS_32) -o $@ $(ZIPC_OBJS_ULTIMATE)

zipcracker_libzip:  $(ZIPC_OBJS_LIBZIP)
	$(CC) $(CFLAGS_LIBZIP) -o $@ $(ZIPC_OBJS_LIBZIP)
clean:
	/bin/rm -f *.o *~ zipcracker32 zipcracker_libzip zipcracker_ultimate
