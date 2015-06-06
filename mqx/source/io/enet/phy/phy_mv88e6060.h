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
* $FileName: phy_mv88e6060.h$
* $Version : 3.0.3.0$
* $Date    : Jul-8-2009$
*
* Comments:
*
*   This file contains definitions that belongs to the PHY chip
*   rtl8201.
*
*END************************************************************************/
#ifndef _phy_mv88e6060_h_
#define _phy_mv88e6060_h_ 1

#ifdef __cplusplus
extern "C" {
#endif

#define PHY_ID_MV88E6060    0x0141
    
extern const ENET_PHY_IF_STRUCT phy_mv88e6060_IF;

boolean phy_mv88e6060_get_reg(uint_32 ulPhyID, uint_32 ulRegID, uint_32_ptr pValue);
boolean phy_mv88e6060_set_reg(uint_32 ulPhyID, uint_32 ulRegID, uint_32 ulValue);

#ifdef __cplusplus
}
#endif

#endif /* _phy_mv88e6060_h_ */
/* EOF */
