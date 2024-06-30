#ifndef __USR_PRINTF_STDARG_H__
#define __USR_PRINTF_STDARG_H__

int PRINTF(const char *format, ...);
int SPRINTF(char *out, const char *format, ...);
int SNPRINTF( char *buf, unsigned int count, const char *format, ... );

#endif
