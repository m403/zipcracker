CC=gcc

CFLAGS_32_t=-Wall -O -Izlib-1.2.7_32/ -pthread
CFLAGS_LIBZIP=-Wall -O2 -pthread

TARGET_FOLDER_32=zlib-1.2.7_32/contrib/minizip/
TARGET_FOLDER_64=zlib-1.2.7_64/contrib/minizip/

ZIPCTHREADS_OBJS_32 = zipcracker_threads.o $(TARGET_FOLDER_32)unzip.o $(TARGET_FOLDER_32)ioapi.o zlib-1.2.7_32/libz.a
ZIPC_OBJS_32_t=zipcracker_preproc.o preproc.o $(TARGET_FOLDER_32)unzip.o $(TARGET_FOLDER_32)ioapi.o zlib-1.2.7_32/libz.a

.c.o:
	$(CC) -c $(CFLAGS_32_t) $*.c -o $@

all: zipcracker_threads zipcracker_preproc

zipcracker_threads:  $(ZIPCTHREADS_OBJS_32)
	$(CC) $(CFLAGS_32_t) -o $@ $(ZIPCTHREADS_OBJS_32)

zipcracker_preproc:  $(ZIPC_OBJS_32_t)
	$(CC) $(CFLAGS_32_t) -o $@ $(ZIPC_OBJS_32_t)
clean:
	/bin/rm -f *.o *~ zipcracker_threads zipcracker_preproc preproc
