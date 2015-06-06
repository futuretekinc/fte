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
* @file fnet_comp.h
*
* @author Andrey Butok
*
* @date Mar-14-2011
*
* @version 0.1.4.0
*
* @brief C-compiler-specific definitions.
*
***************************************************************************/


#ifndef _FNET_COMP_H_

#define _FNET_COMP_H_

#include "fnet_comp_config.h"

#if FNET_CFG_COMP_CW
    /* To use _Pragma(), enable C99 Extensions. */
    #pragma c99 on

    #if 1 /* Newest CWs pragma */
        #define FNET_COMP_PACKED_BEGIN  _Pragma("pack(1)")
        #define FNET_COMP_PACKED_END    _Pragma("pack()")
    #else /* Old CWs pragma */
        #define FNET_COMP_PACKED_BEGIN  _Pragma("options align = packed")
        #define FNET_COMP_PACKED_END    _Pragma("options align = reset")
    #endif
#endif

#if FNET_CFG_COMP_IAR
    /* Disable some warnings.*/
    #pragma diag_suppress=Pe009,Pa039,Pe177

    /* To use _Pragma(), enable C99 Extensions. */
    #define FNET_COMP_PACKED_BEGIN  _Pragma("pack(1)")
    #define FNET_COMP_PACKED_END    _Pragma("pack()")
#endif


/* Modifies the current alignment mode in the structure,
which causes that member to be packed on a 1-byte boundary
*/
#ifndef FNET_COMP_PACKED_BEGIN
    #define FNET_COMP_PACKED_BEGIN
#endif

/* Restore alignment mode.*/
#ifndef FNET_COMP_PACKED_END
    #define FNET_COMP_PACKED_END
#endif

/* Mark unused function argument. To avoid compiler warnings.*/
#ifndef FNET_COMP_UNUSED_ARG
    #define FNET_COMP_UNUSED_ARG(x) (void)(x)
#endif 


#endif /* _FNET_COMP_H_ */