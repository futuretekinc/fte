/**HEADER********************************************************************
* 
* Copyright (c) 2008 Freescale Semiconductor;
* All Rights Reserved
*
* Copyright (c) 2004-2008 Embedded Access Inc.;
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
* $FileName: phy_fte.h$
* $Version : 3.0.3.0$
* $Date    : Jul-8-2009$
*
* Comments:
*
*   This file contains definitions that belongs to the PHY chip
*   fte.
*
*END************************************************************************/
#ifndef _phy_fte_h_
#define _phy_fte_h_ 1

#ifdef __cplusplus
extern "C" {
#endif

#define PHY_MII_REG_PHYC2_PAGE7             0x0007
    
#define PHY_MII_REG_PHYC2_PAGE7_INT_WOL_LED 0x0013
 
extern const ENET_PHY_IF_STRUCT phy_fte_IF;

#ifdef __cplusplus
}
#endif

#endif /* _phy_fte_h_ */
/* EOF */
