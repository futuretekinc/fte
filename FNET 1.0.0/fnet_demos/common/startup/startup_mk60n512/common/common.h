/*
 * File:        common.h
 * Purpose:     File to be included by all project files
 *
 * Notes:
 */

#ifndef _COMMON_H_
#define _COMMON_H_

/********************************************************************/

/*
 * Debug prints ON (#define) or OFF (#undef)
 */
#define DEBUG
#define DEBUG_PRINT

/* 
 * Include the generic CPU header file 
 */
#include "arm_cm4.h"

/* 
 * Include the platform specific header file 
 */
#include "fte.h"

/* 
 * Include the cpu specific header file 
 */
#if (defined(CPU_MK40N512VMD100))
  #include "MK40N512VMD100.h"
#elif (defined(CPU_MK60N512VMD100))
  #include "MK60N512VMD100.h"
#else
  #error "No valid CPU defined"
#endif


/* 
 * Include any toolchain specfic header files 
 */
#if (defined(__MWERKS__))
  #include "mwerks.h"
#elif (defined(__DCC__))
  #include "build/wrs/diab.h"
#elif (defined(__ghs__))
  #include "build/ghs/ghs.h"
#elif (defined(__GNUC__))
  #include "build/gnu/gnu.h"
#elif (defined(IAR))
  #include "iar.h"
#else
#warning "No toolchain specific header included"
#endif

/* 
 * Include common utilities
 */
#include "startup.h"
#include "stdlib.h"
#include "intrinsics.h"

/********************************************************************/

#endif /* _COMMON_H_ */
