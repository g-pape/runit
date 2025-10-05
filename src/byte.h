/* Public domain. */

#ifndef BYTE_H
#define BYTE_H

extern unsigned int byte_chr(char *,register unsigned int,int);
extern unsigned int byte_rchr(char *,register unsigned int,int);
extern void byte_copy(register char *,register unsigned int,register char *);
extern void byte_copyr(register char *,register unsigned int,register char *);
extern int byte_diff(register char *,register unsigned int,register char *);
extern void byte_zero(char *,register unsigned int);

#define byte_equal(s,n,t) (!byte_diff((s),(n),(t)))

#endif
