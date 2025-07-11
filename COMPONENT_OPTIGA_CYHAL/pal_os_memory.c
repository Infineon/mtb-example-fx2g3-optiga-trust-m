/**
* \copyright
* MIT License
*
* Copyright (c) 2025 Infineon Technologies AG
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE
*
* \endcopyright
*
* \author Infineon Technologies AG
*
* \file pal_os_memory.c
*
* \brief   This file implements the platform abstraction layer APIs for memory.
*
* \ingroup  grProtectedUpdateTool
*
* @{
*/

#include "optiga_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern cy_stc_hbdma_buf_mgr_t HBW_BufMgr;      /* High BandWidth DMA buffer manager. */

void * pal_os_malloc(uint32_t block_size)
{
    return Cy_HBDma_BufMgr_Alloc(&HBW_BufMgr, block_size);
}

void * pal_os_calloc(uint32_t number_of_blocks , uint32_t block_size)
{
    return Cy_HBDma_BufMgr_Alloc(&HBW_BufMgr, block_size*number_of_blocks);;
}

void pal_os_free(void * p_block)
{
    Cy_HBDma_BufMgr_Free(&HBW_BufMgr, p_block);
}

void pal_os_memcpy(void * p_destination, const void * p_source, uint32_t size)
{
    memcpy(p_destination, p_source, size);
}

void pal_os_memset(void * p_buffer, uint32_t value, uint32_t size)
{
    memset(p_buffer, (int32_t)value, size);
}

/**
* @}
*/