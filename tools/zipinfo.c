#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0) 

#define CENTRAL_DIRECTORY_HEADER_SIZE 50
#define END_CENTRAL_DIRECTORY_SIZE 25

unsigned long searchInZip(FILE *, char *);
void printCentralDirectory(FILE *, unsigned long offset);

int main(int argc, char *argv[])
{
    char *zip;
    FILE *f;
    unsigned long offset;
    char centralDirectory[] = {0x50, 0x4b, 0x01, 0x02};

    zip = argv[1];
    f = fopen(zip, "rb");

    offset = searchInZip(f, centralDirectory);
    printf("%02lX\n", offset);
}

unsigned long searchInZip(FILE *f, char *pattern)
{
    unsigned long size, len;
    int i, pos, csearch;

    fseek(f, 0L, SEEK_SET);
    fseek(f, 0L, SEEK_END);
    size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    len = strlen(pattern);
    pos = 0;
    csearch = 0;

    /*printf("strlen:%d\n", len);*/

    while(pos < size)
    {
        if(fgetc(f) == pattern[csearch])
        {
            if(csearch++ == len-1)
            {
                printCentralDirectory(f, ftell(f)-len);
                csearch = 0;
            }
        }
        else
            csearch = 0;

        pos++;
    }

    return ftell(f) - 4;
}

void printCentralDirectory(FILE *f, unsigned long offset)
{
    int i;
    char s, v;


    fseek(f, offset, SEEK_SET);

    /* HEADER */
    printf("Header: ");
    for(i = 0; i < 4; i++)
        printf("%02X", fgetc(f));

    /* VERSION MADE BY */
    printf("\nVersion made by: ");
    v  = fgetc(f);
    s  = fgetc(f);

    switch(s)
    {
        case 0:
            printf("FAT file system (DOS, OS/2, NT)");
            break;
        case 1:
            printf("Amiga");
            break;
        case 2:
            printf("OpenVNS");
            break;
        case 3:
            printf("Unix");
            break;
        case 4:
            printf("Macintosh");
            break;
        case 10:
            printf("Windows NTFS");
            break;
        case 19:
            printf("OS/X (Darwin)");
            break;
        default:
            printf("%2X", s);
            break;
    }
    printf(" v%d.%d ", v/10, v%10);

    /* VERSION */
    printf("\nVersion needed to extract: ");
    v = fgetc(f);
    s = fgetc(f);
    printf("%d.%d", v/10, v%10);

    /* GPBF */
    printf("\nGeneral purpose bit flag: ");
    v = fgetc(f);
    s = fgetc(f);
    printf(BYTETOBINARYPATTERN BYTETOBINARYPATTERN, BYTETOBINARY(v), BYTETOBINARY(s));

    /* COMPRESSION METHOD */
    printf("\nCompression method: ");
    fgetc(f);
    switch((v=fgetc(f)))
    {
        case 0:
            printf("Store");
            break;
        case 1:
            printf("Shrunk");
            break;
        case 2:
            printf("Reduced with compression factor 1");
            break;
        case 3:
            printf("Reduced with compression factor 2");
            break;
        case 4:
            printf("Reduced with compression factor 3");
            break;
        case 5:
            printf("Reduced with compression factor 4");
            break;
        case 6:
            printf("Implode");
            break;
        case 7:
            printf("Reserved for Tokenizing compression algorithm");
            break;
        case 8:
            printf("Deflate");
            break;
        case 9:
            printf("Deflate64(tm)");
            break;
        case 10:
            printf("PKWARE Data Compression Library Imploding");
            break;
        case 11:
            printf("Reserved by PKWARE");
            break;
        case 12:
            printf("BZIP2");
            break;
        default:
            printf("%02X", v);
            break;
    };

    /*FIXME*/
    printf("\nLast mod file time: ");
    v = fgetc(f);
    s = fgetc(f);
    printf("%02d/%02d/%04d", (v&0xf8)>>3, v&0x7, 0);

    printf("\nLast mod file data: ");
    v = fgetc(f);
    s = fgetc(f);
    printf("%02d", v&0xfe);

    
    printf("\nCRC-32: ");
    for(i = 0; i < 4; i++)
        printf("%02X", fgetc(f));


    /*printf("\nCompressed size: ");*/
    /*for(; i < 24; i++)*/
        /*printf("%02X", fgetc(f));*/
    
    /*printf("\nUncompressed size: ");*/
    /*for(; i < 28; i++)*/
        /*printf("%02X", fgetc(f));*/

    printf("\n");
    exit(0);
}
