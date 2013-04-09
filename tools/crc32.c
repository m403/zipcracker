#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

int main(int argc, char *argv[])
{
    char *buf, *filename;
    FILE *f;
    unsigned long crc, filesize;
    
    crc = crc32(0L, Z_NULL, 0);
    filename = argv[1];

    f = fopen(filename, "r");

    fseek(f, 0L, SEEK_END);
    filesize = ftell(f);
    fseek(f, 0L, SEEK_SET);

    buf = (char *)calloc(filesize, sizeof(char));
    fread(buf, filesize, 1, f);

    crc = crc32(crc, buf, filesize);

    printf("%s -- %lu\n", filename, crc);

    return 0;
}
