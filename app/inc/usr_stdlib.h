/*
 * File:		stdlib.h
 * Purpose:		Function prototypes for standard library functions
 *
 * Notes:
 */

#ifndef _STDLIB_H
#define _STDLIB_H

/********************************************************************
 * Standard library functions
 ********************************************************************/

#ifndef FALSE
#define FALSE  0
#endif

#ifndef TRUE
#define TRUE   (!FALSE)
#endif

#ifndef NULL
#define NULL   ((void*)0)
#endif

int
isspace (int);

int
isalnum (int);

int
isdigit (int);

int
isupper (int);

int
strcasecmp_t (const char *, const char *);

int
stricmp_t (const char *, const char *);

int
strncasecmp_t (const char *, const char *, int);

int
strnicmp_t (const char *, const char *, int);

unsigned long
strtoul_t (char *, char **, int);

int
atoi_t (const char *str);

int
strlen_t (const char *);

char *
strcat_t (char *, const char *);

char *
strncat_t (char *, const char *, int);

char *
strcpy_t (char *, const char *);

char *
strncpy_t (char *, const char *, int);

int
strcmp_t (const char *, const char *);

int
strncmp_t (const char *, const char *, int);

char *
strstr_t (const char *s1, const char *s2);

char *
strchr_t (const char *str, int c);

void *
memcpy_t (void *, const void *, int);

void *
memset_t (void *, int, int);

int
memcmp_t (const void *, const void *s2, int);

void *
memmove_t (void *, const void *, int);

/********************************************************************/

#endif
