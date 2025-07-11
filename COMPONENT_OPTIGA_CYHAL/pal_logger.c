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
 * \file    pal_logger.c
 *
 * \brief   This file provides the prototypes declarations for pal logger.
 *
 * \ingroup grPAL
 *
 * @{
 */

#include "pal_logger.h"
#include "cy_debug.h"
#include "cy_pdl.h"

//lint --e{552,714} suppress "Accessed by user of this structure" 
pal_logger_t logger_console =
{
        .logger_config_ptr = NULL,
        .logger_rx_flag = 1,
        .logger_tx_flag = 1
};

pal_status_t pal_logger_init(void * p_logger_context)
{
    pal_status_t return_status = PAL_STATUS_FAILURE;

    return return_status;
}


pal_status_t pal_logger_deinit(void * p_logger_context)
{
    pal_status_t return_status = PAL_STATUS_FAILURE;

    return return_status;
}


pal_status_t pal_logger_write(void * p_logger_context, const uint8_t * p_log_data, uint32_t log_data_length)
{

    int32_t return_status = PAL_STATUS_FAILURE;

    return ((pal_status_t)return_status);
}

pal_status_t pal_logger_read(void * p_logger_context, uint8_t * p_log_data, uint32_t log_data_length)
{

    int32_t return_status = PAL_STATUS_FAILURE;

    return ((pal_status_t)return_status);
}
/**
 * @}
 */
