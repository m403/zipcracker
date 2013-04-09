#ifndef ZIPCRACKER_H
#define ZIPCRACKER_H

char *dictionary_mode(struct zip *, char *);
static int extract(struct zip *, char *);
static char *readline(FILE *); 
void print_err(const char *);

#endif
