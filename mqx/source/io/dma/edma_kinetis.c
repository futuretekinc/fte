/**HEADER********************************************************************
*
* Copyright (c) 2013 Freescale Semiconductor;
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
* Comments:
*
*   This file contains CPU specific eDMA functions.
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "edma.h"


static const uint_32 /*PSP_INTERRUPT_TABLE_INDEX*/ edma_done_vectors[][16] =
{
    { 
        INT_DMA0,
        INT_DMA1,
        INT_DMA2,
        INT_DMA3,
        INT_DMA4,
        INT_DMA5,
        INT_DMA6,
        INT_DMA7,
        INT_DMA8,
        INT_DMA9,
        INT_DMA10,
        INT_DMA11,
        INT_DMA12,
        INT_DMA13,
        INT_DMA14,
        INT_DMA15
    }
};



static const uint_32 /*PSP_INTERRUPT_TABLE_INDEX*/ edma_error_vectors[][1] =
{
    { 
        INT_DMA_Error
    }
};



/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_edma_done_vectors
* Returned Value   : Number of vectors associated with the peripheral
* Comments         :
*    This function returns desired interrupt vector table indices for specified module.
*
*END*----------------------------------------------------------------------*/

int _bsp_get_edma_done_vectors(uint_32 dev_num, const uint_32 _PTR_ _PTR_ vectors_ptr)
{
    if (dev_num < ELEMENTS_OF(edma_done_vectors)) {
        *vectors_ptr = edma_done_vectors[dev_num];
        return ELEMENTS_OF(edma_done_vectors[0]);
    }
    return 0;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_edma_error_vector
* Returned Value   : Number of vectors associated with the peripheral
* Comments         :
*    This function returns desired interrupt vector table indices for specified module.
*
*END*----------------------------------------------------------------------*/

int _bsp_get_edma_error_vectors(uint_32 dev_num, const uint_32 _PTR_ _PTR_ vectors_ptr)
{
    if (dev_num < ELEMENTS_OF(edma_error_vectors)) {
        *vectors_ptr = edma_error_vectors[dev_num];
        return ELEMENTS_OF(edma_error_vectors[0]);
    }
    return 0;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_edma_enable
* Returned Value   :
* Comments         :
*    The function performs necessary operations to enable eDMA module
*
*END*----------------------------------------------------------------------*/

int _bsp_edma_enable(uint_32 dev_num)
{
    /* Enable DMAMUX clock */
    SIM_SCGC6 |= SIM_SCGC6_DMAMUX_MASK;

    /* Enable DMA clock */
    SIM_SCGC7 |= SIM_SCGC7_DMA_MASK;

    return 0;
}
