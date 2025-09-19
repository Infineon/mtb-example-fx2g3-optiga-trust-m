/***************************************************************************//**
* \file optiga_app.c
*
* \version 1.0.1
*
* \details  This file provides the headers supporting operations such as generating
*           a keypair, signing a digest, and verifying the signature using the 
*           Optiga Trust M module, from an FX2G3 device.
*
* See \ref README.md ["README"]
*
*******************************************************************************
* \copyright
* The MIT License
*
* Copyright (c) 2021 Infineon Technologies AG
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
*******************************************************************************/

#ifndef _OPTIGA_APP_H_
#define _OPTIGA_APP_H_

/* FreeRTOS include */
#include "FreeRTOS.h"

/* Library stack include */
#include "cy_debug.h"

/* Optiga includes */
#include "optiga_crypt.h"
#include "optiga_util.h"
#include "pal_os_timer.h"
#include "pal_os_memory.h"
#include "pal_custom.h"

/* Macros */
#define START_TIMER                                 (TRUE)
#define STOPTIMER_AND_CALCULATE                     (FALSE)
#define OPTIGA_EXAMPLE_UTIL_DER_BITSTRING_TAG       (0x03)
#define OPTIGA_EXAMPLE_UTIL_DER_NUM_UNUSED_BITS     (0x00)
#define OPTIGA_FREE_ECC_KEY_ID                      OPTIGA_KEY_ID_E0F2
#define VBUS_DETECT_GPIO_PORT                       (P4_0_PORT)
#define VBUS_DETECT_GPIO_PIN                        (P4_0_PIN)
#define VBUS_DETECT_GPIO_INTR                       (ioss_interrupts_gpio_dpslp_4_IRQn)
#define VBUS_DETECT_STATE                           (0u)

#define START_PERFORMANCE_MEASUREMENT(time_taken) \
    optiga_app_performance_measurement(&time_taken, START_TIMER)

#define READ_PERFORMANCE_MEASUREMENT(time_taken) \
    optiga_app_performance_measurement(&time_taken, STOPTIMER_AND_CALCULATE)

#define OPTIGA_LOG_MESSAGE(msg, ...) \
{ \
    Cy_Debug_AddToLog(3, "[Optiga]: "msg"%s", ##__VA_ARGS__, "\r\n"); \
}

#define OPTIGA_LOG_ERROR(msg, ...) \
{ \
    Cy_Debug_AddToLog(3, "[Optiga][ERROR]: "msg"%s", ##__VA_ARGS__, "\r\n"); \
}

#define WAIT_AND_CHECK_STATUS(return_status, optiga_lib_status) \
{ \
    if (OPTIGA_LIB_SUCCESS != return_status) { \
        break; \
    } \
    while (OPTIGA_LIB_BUSY == optiga_lib_status) { \
    } \
    if (OPTIGA_LIB_SUCCESS != optiga_lib_status) { \
        return_status = optiga_lib_status; \
        break; \
    } \
}

#define OPTIGA_LOG_STATUS(msg, return_value) \
{ \
    if (OPTIGA_LIB_SUCCESS != return_value) { \
         Cy_Debug_AddToLog(3, "[Optiga][ERROR]: %s, Status - 0x%x%s", msg, return_value, "\r\n"); \
    } \
    else\
    { \
         Cy_Debug_AddToLog(3, "[Optiga]: %s, Status - 0x%x%s", msg, return_value, "\r\n"); \
    } \
}

#define OPTIGA_LOG_PERFORMANCE_VALUE(time_taken, return_value) \
{ \
    if (OPTIGA_LIB_SUCCESS == return_value) { \
        Cy_Debug_AddToLog(3, "[Optiga]: Time Taken - %dms, Status - 0x%x\r\n", time_taken, return_value); \
    } \
    else \
    { \
        Cy_Debug_AddToLog(3, "[Optiga][ERROR]: Time Taken - %dms, Status - 0x%x\r\n", time_taken, return_value); \
    } \
}

/* Functions Declarations */


/**
* \name Cy_Optiga_Init
* \brief Initialize the Optiga module
* \retval None
*/
void Cy_Optiga_Init(void);

/**
* \name Cy_Optiga_Init
* \brief De-initialize the Optiga module
* \retval None
 */
 void Cy_Optiga_Deinit(void);

/**
 * \name printHex
 * \brief Inserts leading zero to visually adjust padding in logs, and prints the hex number
 * \param hexnum
 * \retval None
 */
void printHex(uint8_t hexnum);

/**
 * \name printArray16
 * \brief Print an array with upto 16B per row
 * \param arrayName A name to use for the array in the print log
 * \param array
 * \param length in bytes
 * \param header Log upto 15 starting array elements in a row, before standard rows of 16B begin
 * \retval None
 */
void printArray16(char *arrayName, uint8_t *array, uint32_t length, bool header);

/**
 * \name optiga_app_util_encode_ecc_public_key_in_bit_string_format
 * \brief If and when using a pure pubkey that isn't provided by Optiga, this function will add required headers
 * \note This function is unused in this example, as the keypair is generated by the Optiga module includes headers
 * \param q_buffer
 * \param q_length
 * \param pub_key_buffer
 * \param pub_key_length
 * \retval None
 */
void optiga_app_util_encode_ecc_public_key_in_bit_string_format(const uint8_t *q_buffer, uint8_t q_length, uint8_t *pub_key_buffer, uint16_t *pub_key_length);

/**
 * \name optiga_app_performance_measurement
 * \brief Calculate time difference for performance measurements
 * \param time_value
 * \param time_reset_flag
 * \retval None
 */
void optiga_app_performance_measurement(uint32_t *time_value, uint8_t time_reset_flag);

/**
 * \name Cy_Optiga_HbDmaInit
 * \brief Initialize HBDMA block.
 * \retval None
 */
bool Cy_Optiga_HbDmaInit(void);

/**
 * \name Cy_Optiga_Main
 * \brief The main Optiga application logic
 * \retval None
 */
void Cy_Optiga_Main(void);

#endif /* _OPTIGA_APP_H_ */
