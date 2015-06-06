/**************************************************************************
* 
* Copyright 2005-2011 by Andrey Butok. Freescale Semiconductor, Inc.
*
***************************************************************************
* This program is free software: you can redistribute it and/or modify
* it under the terms of either the GNU General Public License 
* Version 3 or later (the "GPL"), or the GNU Lesser General Public 
* License Version 3 or later (the "LGPL").
*
* As a special exception, the copyright holders of the FNET project give you
* permission to link the FNET sources with independent modules to produce an
* executable, regardless of the license terms of these independent modules,
* and to copy and distribute the resulting executable under terms of your 
* choice, provided that you also meet, for each linked independent module,
* the terms and conditions of the license of that module.
* An independent module is a module which is not derived from or based 
* on this library. 
* If you modify the FNET sources, you may extend this exception 
* to your version of the FNET sources, but you are not obligated 
* to do so. If you do not wish to do so, delete this
* exception statement from your version.
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
* @file fnet_stdlib.h
*
* @date Apr-20-2011
*
* @version 0.1.14.0
*
* @brief Standard functions API.
*
***************************************************************************/

#ifndef _FNET_STDLIB_H_

#define _FNET_STDLIB_H_

#include "fnet_config.h"



/********************************************************************
* Standard library functions
*********************************************************************/
#if FNET_CFG_COMP_CW
    /* Type to hold information about variable arguments. */
    typedef char *fnet_va_list;
    /* Initialize a variable argument list.*/
    #define fnet_va_start(ap, parm) ap = (char *)(&parm + 1)
    /* End using variable argument list.*/
    #define fnet_va_end(ap)         ((void)0)
    /* Retrieve next argument.*/
    #define fnet_va_arg(ap, type)   (*(type *)((ap += sizeof(type) + 3U & ~3U) - (sizeof(type))))
#else //TBD
    #include <stdarg.h>
    #define fnet_va_list    va_list   
    #define fnet_va_start   va_start
    #define fnet_va_end     va_end
    #define fnet_va_arg     va_arg
#endif

void fnet_memcpy( void *, const void *, unsigned );
void *fnet_memset( void *, int, unsigned );
int fnet_memcmp( void *, const void *, int );

unsigned long fnet_strlen (const char *str);
char * fnet_strcat (char *dest, const char *src);
char * fnet_strncat (char *dest, const char *src, int n);
char * fnet_strcpy (char *dest, const char *src);
char *fnet_strncpy( char *dest, const char *src, unsigned long n );
char *fnet_strrchr( const char *str, int chr );
char *fnet_strchr( const char *str, int chr );
char *fnet_strstr( const char *str, const char *pat );
int fnet_strcmp( const char *, const char * );
int fnet_strncmp( const char *str1, const char *str2, unsigned int n );
unsigned long fnet_strtoul (char *str, char **ptr, int base);
int fnet_strcasecmp( const char *s1, const char *s2 );
int fnet_strcmp_splitter( const char *in_str, const char *name, char splitter);

#define FNET_TRUE       (1)
#define FNET_FALSE      (0)

#include "fnet_serial.h"

#endif
