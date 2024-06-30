/* FILENAME: stdlib.c
 *
 * Functions normally found in a standard C lib.
 *
 * 12/28/2005 - added memcmp and memmove
 *
 * Notes: These functions support ASCII only!!!
 */

#include "usr_stdlib.h"

/****************************************************************/
int
isspace (int ch)
{
	if ((ch == ' ') || (ch == '\t'))	/* \n ??? */
		return TRUE;
	else
		return FALSE;
}

/****************************************************************/
int
isalnum (int ch)
{
	/* ASCII only */
	if (((ch >= '0') && (ch <= '9')) ||
		((ch >= 'A') && (ch <= 'Z')) ||
		((ch >= 'a') && (ch <= 'z')))
		return TRUE;
	else
		return FALSE;
}

/****************************************************************/
int
isdigit (int ch)
{
	/* ASCII only */
	if ((ch >= '0') && (ch <= '9'))
		return TRUE;
	else
		return FALSE;
}

/****************************************************************/
int
isupper (int ch)
{
	/* ASCII only */
	if ((ch >= 'A') && (ch <= 'Z'))
		return TRUE;
	else
		return FALSE;
}

/****************************************************************/
int
strcasecmp_t (const char *s1, const char *s2)
{
	char	c1, c2;
	int		result = 0;

	while (result == 0)
	{
		c1 = *s1++;
		c2 = *s2++;
		if ((c1 >= 'a') && (c1 <= 'z'))
			c1 = (char)(c1 - ' ');
		if ((c2 >= 'a') && (c2 <= 'z'))
			c2 = (char)(c2 - ' ');
		if ((result = (c1 - c2)) != 0)
			break;
		if ((c1 == 0) || (c2 == 0))
			break;
	}
	return result;
}


/****************************************************************/
int
stricmp_t (const char *s1, const char *s2)//忽略大小写比较字符串
{
   return (strcasecmp_t(s1, s2));
}

/****************************************************************/
int
strncasecmp_t (const char *s1, const char *s2, int n)
{
	char	c1, c2;
	int		k = 0;
	int		result = 0;

	while ( k++ < n )
	{
		c1 = *s1++;
		c2 = *s2++;
		if ((c1 >= 'a') && (c1 <= 'z'))
			c1 = (char)(c1 - ' ');
		if ((c2 >= 'a') && (c2 <= 'z'))
			c2 = (char)(c2 - ' ');
		if ((result = (c1 - c2)) != 0)
			break;
		if ((c1 == 0) || (c2 == 0))
			break;
	}
	return result;
}

/****************************************************************/
int
strnicmp_t (const char *s1, const char *s2, int n)//忽略大小写比较字符串
{
   return (strncasecmp_t(s1, s2, n));
}

/****************************************************************/
unsigned long
strtoul_t (char *str, char **ptr, int base)
{
	unsigned long rvalue = 0;
	int neg = 0;
	int c;

	/* Validate parameters */
	if ((str != NULL) && (base >= 0) && (base <= 36))
	{
		/* Skip leading white spaces */
		while (isspace(*str))
		{
			++str;
		}

		/* Check for notations */
		switch (str[0])
		{
		case '0':
          	if (base == 0)
          	{
             	if ((str[1] == 'x') || (str[1] == 'X'))
				{
					base = 16;
					str += 2;
				}
             	else
             	{
                	base = 8;
                	str++;
			 	}
		   	}
		  	break;
    
		case '-':
			neg = 1;
          	str++;
          	break;

       	case '+':
          	str++;
			break;

		default:
			break;
		}

		if (base == 0)
			base = 10;

      	/* Valid "digits" are 0..9, A..Z, a..z */
      	while (isalnum(c = *str))
      	{
			/* Convert char to num in 0..36 */
	         if ((c -= ('a' - 10)) < 10)         /* 'a'..'z' */
	         {
	            if ((c += ('a' - 'A')) < 10)     /* 'A'..'Z' */
	            {
	               c += ('A' - '0' - 10);        /* '0'..'9' */
				}
			}
	
			/* check c against base */
			if (c >= base)
			{
				break;
			}
	
			if (neg)
			{
				rvalue = (rvalue * base) - c;
			}
			else
			{
				rvalue = (rvalue * base) + c;
	        }
	
	        ++str;
		}
	}

   /* Upon exit, 'str' points to the character at which valid info */
   /* STOPS.  No chars including and beyond 'str' are used.        */

	if (ptr != NULL)
		*ptr = str;
		
	return rvalue;
}

/****************************************************************/
int
atoi_t (const char *str)
{
   char *s = (char *)str;
   
   return ((int)strtoul_t(s, NULL, 10));
}

/****************************************************************/
int
strlen_t (const char *str)
{
	char *s = (char *)str;
	int len = 0;

	if (s == NULL)
		return 0;

	while (*s++ != '\0')
		++len;

	return len;
}

/****************************************************************/
char *
strcat_t (char *dest, const char *src)
{
	char *dp;
	char *sp = (char *)src;

	if ((dest != NULL) && (src != NULL))
	{
		dp = &dest[strlen_t(dest)];

		while (*sp != '\0')
		{
			*dp++ = *sp++;
		}
		*dp = '\0';
	}
	return dest;
}

/****************************************************************/
char *
strncat_t (char *dest, const char *src, int n)
{
	char *dp;
	char *sp = (char *)src;

	if ((dest != NULL) && (src != NULL) && (n > 0))
	{
		dp = &dest[strlen_t(dest)];

		while ((*sp != '\0') && (n-- > 0))
		{
			*dp++ = *sp++;
		}
		*dp = '\0';
	}
	return dest;
}

/****************************************************************/
char *
strcpy_t (char *dest, const char *src)
{
	char *dp = (char *)dest;
	char *sp = (char *)src;

	if ((dest != NULL) && (src != NULL))
	{
		while (*sp != '\0')
		{
			*dp++ = *sp++;
		}
		*dp = '\0';
	}
	return dest;
}

/****************************************************************/
char *
strncpy_t (char *dest, const char *src, int n)
{
	char *dp = (char *)dest;
	char *sp = (char *)src;

	if ((dest != NULL) && (src != NULL) && (n > 0))
	{
		while ((*sp != '\0') && (n-- > 0))
		{
			*dp++ = *sp++;
		}
		*dp = '\0';
	}
	return dest;
}

/****************************************************************/
int
strcmp_t (const char *s1, const char *s2)
{
	/* No checks for NULL */
	char *s1p = (char *)s1;
	char *s2p = (char *)s2;

	while (*s2p != '\0')
	{
		if (*s1p != *s2p)
			break;

		++s1p;
		++s2p;
	}
	return (*s1p - *s2p);
}

/****************************************************************/
int
strncmp_t (const char *s1, const char *s2, int n)
{
	/* No checks for NULL */
	char *s1p = (char *)s1;
	char *s2p = (char *)s2;

	if (n <= 0)
		return 0;

	while (*s2p != '\0')
	{
		if (*s1p != *s2p)
			break;

		if (--n == 0)
			break;

		++s1p;
		++s2p;
	}
	return (*s1p - *s2p);
}

/****************************************************************/
char *
strstr_t(const char *s1, const char *s2)
{
   char *sp = (char *)s1;
   int  len1 = strlen_t(s1);
   int  len2 = strlen_t(s2);

   while (len1 >= len2) 
   {
      if (strncmp_t(sp, s2, len2) == 0)
      {
         return (sp);
      }
      ++sp;
      --len1;
   }

   return (NULL);
}

/****************************************************************/
char *
strchr_t(const char *str, int c)
{
   char *sp = (char *)str;
   char  ch = (char)(c & 0xff);

   while (*sp != '\0')
   {
      if (*sp == ch)
      {
         return (sp);
      }
      ++sp;
   }

   return (NULL);
}

/****************************************************************/
void *
memcpy_t (void *dest, const void *src, int n)
{
	unsigned char *dbp = (unsigned char *)dest;
	unsigned char *sbp = (unsigned char *)src;

	if ((dest != NULL) && (src != NULL) && (n > 0))
	{
      while (n--)
			*dbp++ = *sbp++;
	}
	return dest;
}

/****************************************************************/
void *
memset_t (void *s, int c, int n)
{
	/* Not optimized, but very portable */
	unsigned char *sp = (unsigned char *)s;

	if ((s != NULL) && (n > 0))
	{
		while (n--)
		{
			*sp++ = (unsigned char)c;
		}
	}
	return s;
}

/****************************************************************/
int
memcmp_t (const void *s1, const void *s2, int n)
{
   unsigned char *s1p, *s2p;

   if (s1 && s2 && (n > 0))
   {
      s1p = (unsigned char *)s1;
      s2p = (unsigned char *)s2;

      while (--n >= 0)//((--n >= 0) && (*s1p == *s2p))
      {
         if (*s1p != *s2p)
            return (*s1p - *s2p);
         ++s1p;
         ++s2p;
      }
   }

   return (0);
}

/****************************************************************/
void *
memmove_t (void *dest, const void *src, int n)
{
   unsigned char *dbp = (unsigned char *)dest;
   unsigned char *sbp = (unsigned char *)src;
   unsigned char *dend = dbp + n;
   unsigned char *send = sbp + n;

   if ((dest != NULL) && (src != NULL) && (n > 0))
   {
      /* see if a memcpy would overwrite source buffer */
      if ((sbp < dbp) && (dbp < send))
      {
         while (n--)
            *(--dend) = *(--send);
      }
      else
      {
         while (n--)
            *dbp++ = *sbp++;
      }
   }

   return dest;
}

/****************************************************************/
