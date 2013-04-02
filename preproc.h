#ifndef PREPROC_H
#define PREPROC_H

#define uLong unsigned long int

typedef struct
{
    uLong start;
    uLong end;
}chunks_a;

typedef struct
{
    int t_id;
    uLong start;
    uLong end;
    chunks_a *chunks;
}t_args;

uLong bytefilesize(FILE *);
void checklimit(FILE *, t_args *, int, int, int);
t_args *preproc(const char *, int, int);

#endif
