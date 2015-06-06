/**************************************************************************
* 
* Copyright 2005-2010 by Freescale Semiconductor, Inc.
*
***************************************************************************
* This program is free software: you can redistribute it and/or modify
* it under the terms of either the GNU General Public License 
* Version 3 or later (the "GPL"), or the GNU Lesser General Public 
* License Version 3 or later (the "LGPL").
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License
* and the GNU Lesser General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*
**********************************************************************/ /*!
*
* @file fnet_stdlib.c
*
* @date May-24-2011
*
* @version 0.1.18.0
*
* @brief Standard functions implementation.
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet_stdlib.h"


#if !FNET_CFG_OVERLOAD_MEMCPY
/************************************************************************
* NAME: fnet_memcpy
*
* DESCRIPTION: Slowest but Smalest 
*************************************************************************/
#if 0 /* Slowest && Smallest */
void fnet_memcpy( void *dest, const void *src, unsigned n )
{
    register const char *p = (char *)src;
    register char *q = (char *)dest;

    for (n++; --n; )
        *q++ = *p++;

    //return dest;
}
#elif 0 /* Faster. */
void *fnet_memcpy (void *dest, const void *src, unsigned n)
{
	int longs, bytes;
	fnet_uint32 *dpl = (fnet_uint32 *)dest;
	fnet_uint32 *spl = (fnet_uint32 *)src;
	fnet_uint8  *dpb, *spb;

    bytes = (n & 0x3);
    longs = ((n - bytes) >> 2);

    for (longs++; --longs;)
        *dpl++ = *spl++;
		
    dpb = (fnet_uint8 *)dpl;
    spb = (fnet_uint8 *)spl;

    for (bytes++; --bytes;)
        *dpb++ = *spb++;

	return dest;
}
#else /* Fastest & Biggest. */
void fnet_memcpy(void *to_ptr, const void *from_ptr, unsigned number_of_bytes)
{
    unsigned char *from8_ptr = (unsigned char *) from_ptr;
    unsigned char *to8_ptr = (unsigned char *) to_ptr;
    unsigned short *from16_ptr = (unsigned short *) from_ptr;
    unsigned short * to16_ptr = (unsigned short *) to_ptr;
    register unsigned long *from32_ptr = (unsigned long *) from_ptr;
    register unsigned long *to32_ptr = (unsigned long *) to_ptr;
    register unsigned long loops;

    /*
    * The copying is optimized to avoid alignment problems, and attempts
    *               to copy 32bit numbers optimally.
    */
    if (number_of_bytes > 3)
    {
        /* Try to align source on word */
        if ((unsigned long)from_ptr & 1) 
        {
            from8_ptr = (unsigned char *) from_ptr;
            to8_ptr = (unsigned char *) to_ptr;

            *to8_ptr++ = *from8_ptr++;

            from_ptr = from8_ptr;
            to_ptr = to8_ptr;
            --number_of_bytes;
        }

        /* Try to align source on longword */
        if ((unsigned long)from_ptr & 2)
        {
            from16_ptr = (unsigned short *) from_ptr;
            to16_ptr = (unsigned short *) to_ptr;

            *to16_ptr++ = *from16_ptr++;

            from_ptr = from16_ptr;
            to_ptr = to16_ptr;
            number_of_bytes -= 2;
        }

        from32_ptr = (unsigned long *) from_ptr;
        to32_ptr = (unsigned long *) to_ptr;

        /* 
        ** To increase performance a bit, we will copy 64 bytes (16 * longwords) sequentially
        ** This gets less instruction cycles.
        */
        for (loops = (number_of_bytes >> 6); loops > 0; loops--)
        {
            /* copy 16 longwords */
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
            *to32_ptr++ = *from32_ptr++;
        }

        /* Now, write the rest of bytes */
        switch ((number_of_bytes >> 2) & 0xF)
        {
            case 15: *to32_ptr++ = *from32_ptr++;
            case 14: *to32_ptr++ = *from32_ptr++;
            case 13: *to32_ptr++ = *from32_ptr++;
            case 12: *to32_ptr++ = *from32_ptr++;
            case 11: *to32_ptr++ = *from32_ptr++;
            case 10: *to32_ptr++ = *from32_ptr++;
            case 9:  *to32_ptr++ = *from32_ptr++;
            case 8:  *to32_ptr++ = *from32_ptr++;
            case 7:  *to32_ptr++ = *from32_ptr++;
            case 6:  *to32_ptr++ = *from32_ptr++;
            case 5:  *to32_ptr++ = *from32_ptr++;
            case 4:  *to32_ptr++ = *from32_ptr++;
            case 3:  *to32_ptr++ = *from32_ptr++;
            case 2:  *to32_ptr++ = *from32_ptr++;
            case 1:  *to32_ptr++ = *from32_ptr++;
        } 

        from_ptr = from32_ptr;
        to_ptr = to32_ptr;
    } 

    /* Copy all remaining bytes */
    if (number_of_bytes & 2)
    {
        from16_ptr = (unsigned short *) from_ptr;
        to16_ptr = (unsigned short *) to_ptr;

        *to16_ptr++ = *from16_ptr++;

        from_ptr = from16_ptr;
        to_ptr = to16_ptr;
    } 

    if (number_of_bytes & 1) 
    {
        * (unsigned char *) to_ptr = * (unsigned char *) from_ptr;
    }
}
#endif

#endif
/****************************************************************/
void *fnet_memset( void *s, int c, unsigned n )
{
    /* Not optimized */
    register unsigned char *sp = (unsigned char *)s;
    
    for(n++;--n;)
    {
        *sp++ = (unsigned char)c;
    }

    return s;
}
/************************************************************************
* NAME: fnet_memcmp
*
* DESCRIPTION: Compare two memory regions and return zero if they are identical,
*              non-zero otherwise.  If count is zero, return zero.
*
*    Compare two memory regions and return less than,
*    equal to, or greater than zero, according to lexicographical
*    ordering of the compared regions.
*
*************************************************************************/

int fnet_memcmp( void *src1, register const void *src2, int count )
{
    register const unsigned char *p1;
    register const unsigned char *p2;

    for (p1 = (const unsigned char *)src1, p2 = (const unsigned char *)src2, count++; --count; )
      if(*p1++ != *p2++)
          return 1;

    return (0);
}

/****************************************************************/
int fnet_strcmp( const char *s1, const char *s2 )
{
    /* No checks for 0 */
    register char *s1p = (char *)s1;
    register char *s2p = (char *)s2;

    while(*s2p != '\0')
    {
        if(*s1p != *s2p)
            break;

        ++s1p;
        ++s2p;
    }

    return (*s1p - *s2p);
}


/****************************************************************/
unsigned long fnet_strlen (const char *str)
{
	char *s = (char *)str;
	unsigned long len = 0;

	if (s == 0)
		return 0;

	while (*s++ != '\0')
		++len;

	return len;
}

/****************************************************************/
char * fnet_strcat (char *dest, const char *src)
{
	char *dp;
	char *sp = (char *)src;

	if ((dest != 0) && (src != 0))
	{
		dp = &dest[fnet_strlen(dest)];

		while (*sp != '\0')
		{
			*dp++ = *sp++;
		}
		*dp = '\0';
	}
	return dest;
}

/****************************************************************/
char * fnet_strncat (char *dest, const char *src, int n)
{
	char *dp;
	char *sp = (char *)src;

	if ((dest != 0) && (src != 0) && (n > 0))
	{
		dp = &dest[fnet_strlen(dest)];

		for(n++;(*sp != '\0') && (--n > 0);)
		{
			*dp++ = *sp++;
		}
		*dp = '\0';
	}
	return dest;
}

/****************************************************************/
char * fnet_strcpy (char *dest, const char *src)
{
	char *dp = (char *)dest;
	char *sp = (char *)src;

	if ((dest != 0) && (src != 0))
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
char *fnet_strncpy( char *dest, const char *src, unsigned long n )
{
    char *dp = (char *)dest;
    char *sp = (char *)src;

    if((dest != 0) && (src != 0) && (n > 0))
    {
        while((*sp != '\0') && (n-- > 0))
        {
            *dp++ = *sp++;
        }

        *dp = '\0';
    }

    return dest;
}

/****************************************************************
* The function fnet_strrchr() returns a pointer to the last 
* occurrence of chr in str, or NULL if no match is found.
****************************************************************/
char *fnet_strrchr( const char *str, int chr )
{
    const char *p = str;
    const char *q = 0;
    char c = (char)chr;
    char ch = *p++;

    while(ch)
    {
        if(ch == c)
            q = p - 1;

        ch = *p++;
    }

    if(q)
        return ((char *)q);

    return (c ? 0 : (char *)(p - 1));
}

/****************************************************************
* The function fnet_strchr() returns a pointer to the first 
* occurence of chr in str, or 0 if chr is not found.
****************************************************************/
char *fnet_strchr( const char *str, int chr )
{
    const char *p = str;
    char c = (char)chr;
    char ch = *p++;

    while(ch)
    {
        if(ch == c)
            return ((char *)(p - 1));

        ch = *p++;
    }

    return (c ? 0 : (char *)(p - 1));
}


/****************************************************************
* The function fnet_strstr() returns a pointer to the first 
* occurrence of pat in str, or 0 if no match is found. 
* If the length of pat is zero, then fnet_strstr() will 
* simply return str.  
****************************************************************/
char *fnet_strstr( const char *str, const char *pat )
{
    unsigned char *s1 = (unsigned char *)str;
    unsigned char *p1 = (unsigned char *)pat;
    unsigned char firstc, c1, c2;

    if((pat == 0) || (!(firstc = *p1++)))
        return ((char *)str);

    c1 = *s1++;

    while(c1)
    {
        if(c1 == firstc)
        {
            const unsigned char *s2 = s1;
            const unsigned char *p2 = p1;

            while((c1 = *s2++) == (c2 = *p2++) && c1)
            { };

            if(!c2)
                return ((char *)s1 - 1);
        }

        c1 = *s1++;
    }

    return (0);
}


/****************************************************************
* The fnet_strncmp() function compares at most count characters
* of str1 and str2.  
****************************************************************/
int fnet_strncmp( const char *str1, const char *str2, unsigned int n )
{
    const unsigned char *p1 = (unsigned char *)str1;
    const unsigned char *p2 = (unsigned char *)str2;
    unsigned char c1, c2;

    n++;

    while(--n)
      if((c1 = *p1++) != (c2 = *p2++))
          return (c1 - c2);

      else if(!c1)
          break;

    return (0);
}

/****************************************************************/
static int isspace (int ch)
{
	if ((ch == ' ') || (ch == '\t'))	
		return 1;
	else
		return 0;
}
/****************************************************************/
static int isalnum (int ch)
{
	/* ASCII only */
	if (((ch >= '0') && (ch <= '9')) ||
		((ch >= 'A') && (ch <= 'Z')) ||
		((ch >= 'a') && (ch <= 'z')))
		return 1;
	else
		return 0;
}
/****************************************************************/
static int isdigit (int ch)
{
	/* ASCII only */
	if ((ch >= '0') && (ch <= '9'))
		return 1;
	else
		return 0;
}
/****************************************************************/
static int isupper (int ch)
{
	/* ASCII only */
	if ((ch >= 'A') && (ch <= 'Z'))
		return 1;
	else
		return 0;
}
/****************************************************************/
unsigned long fnet_strtoul (char *str, char **ptr, int base)
{
	unsigned long rvalue;
	int c, err, neg;
	char *endp;
	char *startp;

	rvalue = 0;  err = 0;  neg = 0;

	/* Check for invalid arguments */
	if ((str == 0) || (base < 0) || (base == 1) || (base > 36))
	{
		if (ptr != 0)
		{
			*ptr = str;
		}
		return 0;
	}

	/* Skip leading white spaces */
	for (startp = str; isspace(*startp); ++startp)
		;

	/* Check for notations */
	switch (startp[0])
	{
		case '0':
			if ((startp[1] == 'x') || (startp[1] == 'X'))
			{
				if ((base == 0) || (base == 16))
				{
					base = 16;
					startp = &startp[2];
				}
			}
			break;
		case '-':
			neg = 1;
			startp = &startp[1];
			break;
		default:
			break;
	}

	if (base == 0)
		base = 10;

	/* Check for invalid chars in str */
	for ( endp = startp; (c = *endp) != '\0'; ++endp)
	{
		/* Check for 0..9,Aa-Zz */
		if (!isalnum(c))
		{
			err = 1;
			break;
		}

		/* Convert char to num in 0..36 */
		if (isdigit(c))
		{
			c = c - '0';
		}
		else
		{
			if (isupper(c))
			{
				c = c - 'A' + 10;
			}
			else
			{
				c = c - 'a' + 10;
			}
		}

		/* check c against base */
		if (c >= base)
		{
			err = 1;
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
	}

	/* Upon exit, endp points to the character at which valid info */
	/* STOPS.  No chars including and beyond endp are used.        */

	if (ptr != 0)
		*ptr = endp;

	if (err)
	{
		if (ptr != 0)
			*ptr = str;
		
		return 0;
	}
	else
	{
		return rvalue;
	}
}

/****************************************************************
* This function converts an uppercase letter to the corresponding 
* lowercase letter.
****************************************************************/
static char fnet_tolower( const char to_lower )
{
    if((to_lower >= 'A') && (to_lower <= 'Z'))
        return (char)(to_lower + 0x20);

    return to_lower;
}


/****************************************************************
* The fnet_strcasecmp() function compares the two strings s1 
* and s2, ignoring the case of the characters. It returns an 
* integer less than, equal to, or greater than zero if s1 is found, 
* respectively, to be less than, to match, or be greater than s2. 
****************************************************************/
int fnet_strcasecmp( const char *s1, const char *s2 )
{
    char c1, c2;

    while(1)
    {
        c1 = fnet_tolower(*s1++);
        c2 = fnet_tolower(*s2++);

        if(c1 < c2)
            return -1;

        if(c1 > c2)
            return 1;

        if(c1 == 0)
            return 0;
    }
}


/****************************************************************/
int fnet_strcmp_splitter( const char *in_str, const char *name, char splitter)
{
    int result;
    
    /* No checks for 0 */
    register char *s1p = (char *)in_str;
    register char *s2p = (char *)name;

    while (*s1p == ' ') s1p++;	            /* Strip leading spaces */
	while (*s1p == splitter) s1p++;	/* Strip heading slash */

    while(*s2p != '\0')
    {
        if(*s1p != *s2p)
            break;

        ++s1p;
        ++s2p;
        
        if (*s1p == splitter)
            break; /* next element */
    }
    
    if(*s1p == splitter)
    {
        result = 0;
    }
    else
    {
        result = (*s1p - *s2p);
    }

    return result;
}
