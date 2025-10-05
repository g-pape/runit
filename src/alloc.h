/* Public domain. */

#ifndef ALLOC_H
#define ALLOC_H

extern /*@null@*//*@out@*/char *alloc(unsigned int);
extern void alloc_free(char *);
extern int alloc_re(char **,unsigned int,unsigned int);

#endif
