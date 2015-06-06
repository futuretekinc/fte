/**HEADER********************************************************************
* 
* Copyright (c) 2008 Freescale Semiconductor;
* All Rights Reserved
*
* Copyright (c) 2004-2008 Embedded Access Inc.;
* All Rights Reserved
*
* Copyright (c) 1989-2008 ARC International;
* All Rights Reserved
*
*************************************************************************** 
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* $FileName: phy_fte.c$
* $Version : 3.8.8.0$
* $Date    : Jun-6-2012$
*
* Comments:
*
*  This file contains definitions that belongs to the FTE-E Serial PHY chip
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "bsp_prv.h"
#include "enet.h"
#include "enetprv.h"
#include "phy.h"

static boolean phy_fte_discover_addr(ENET_CONTEXT_STRUCT_PTR enet_ptr);
static boolean phy_fte_init(ENET_CONTEXT_STRUCT_PTR enet_ptr);
static uint_32 phy_fte_get_speed(ENET_CONTEXT_STRUCT_PTR enet_ptr);
static boolean phy_fte_get_link_status(ENET_CONTEXT_STRUCT_PTR enet_ptr);


const ENET_PHY_IF_STRUCT phy_fte_IF = {
  phy_fte_discover_addr,
  phy_fte_init,
  phy_fte_get_speed,
  phy_fte_get_link_status
};
  
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : phy_fte_discover_addr
*  Returned Value : none
*  Comments       :
*    Scan possible PHY addresses looking for a valid device
*
*END*-----------------------------------------------------------------*/

ENET_PHY_IF_STRUCT const * _pPhyIF = NULL;

static boolean phy_fte_discover_addr
   (
       ENET_CONTEXT_STRUCT_PTR     enet_ptr
   )
{ 
   uint_32              i;
   uint_32              id;

   for (i = 0; i < 1; i++) {
      id = 0;
      enet_ptr->PHY_ADDRESS = i;
      if ((*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->PHY_READ)(enet_ptr, PHY_MII_REG_PHYIR1, &id, MII_TIMEOUT)) 
      {
          if ((id != 0) && (id != 0xffff)) 
          {
              switch(id)
              {
              case  PHY_ID_RTL8201:                  
                  {
                      _pPhyIF = &phy_rtl8201_IF;
                      return TRUE;
                  }
                  break;
                  
              case  PHY_ID_MV88E6060:                  
                  {
                      _pPhyIF = &phy_mv88e6060_IF;
                      return TRUE;
                  }
                  break;
              }
          }
      } 
   } 
   
   return FALSE; 
} 


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : phy_fte_init
*  Returned Value : boolean
*  Comments       :
*    Wait for PHY to automatically negotiate its configuration
*
*END*-----------------------------------------------------------------*/

static boolean phy_fte_init
   (
       ENET_CONTEXT_STRUCT_PTR     enet_ptr
   )
{ 
   uint_32 data;
   boolean res = FALSE;
   
   if (_pPhyIF == NULL)
   {
        if (phy_fte_discover_addr(enet_ptr) == FALSE)
        {
            return  FALSE;
        }
   }
   
   return   _pPhyIF->INIT(enet_ptr);
}  


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : phy_fte_get_speed
*  Returned Value : uint_32 - connection speed
*  Comments       :
*    Determine if connection speed is 10 or 100 Mbit
*
*END*-----------------------------------------------------------------*/

static uint_32 phy_fte_get_speed
   (
       ENET_CONTEXT_STRUCT_PTR     enet_ptr
   )
{ 
   uint_32 data;
   boolean res = FALSE;
   
   if (_pPhyIF == NULL)
   {
        if (phy_fte_discover_addr(enet_ptr) == FALSE)
        {
            return  FALSE;
        }
   }
   
   return   _pPhyIF->SPEED(enet_ptr);
} 

  
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : phy_fte_get_link_status
*  Returned Value : TRUE if link active, FALSE otherwise
*  Comments       : 
*    Get actual link status.
*
*END*-----------------------------------------------------------------*/

static boolean phy_fte_get_link_status
   (
       ENET_CONTEXT_STRUCT_PTR     enet_ptr
   )
{ 
   uint_32 data;
   boolean res = FALSE;
   
   if (_pPhyIF == NULL)
   {
        if (phy_fte_discover_addr(enet_ptr) == FALSE)
        {
            return  FALSE;
        }
   }
   
   return   _pPhyIF->STATUS(enet_ptr);
}
