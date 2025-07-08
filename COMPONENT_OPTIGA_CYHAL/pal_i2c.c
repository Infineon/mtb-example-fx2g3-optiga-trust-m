/**
* \copyright
* MIT License
*
* Copyright (c) 2019 Infineon Technologies AG
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
* \file pal_i2c.c
*
* \brief   This file implements the platform abstraction layer(pal) APIs for I2C.
*
* \ingroup  grPAL
*
* @{
*/

#include "pal_i2c.h"
#include "usb_i2c.h"
#include "cy_scb_i2c.h"
#include "cy_pdl.h"
#include "cy_debug.h"
#include "pal_custom.h"
#include "pal_os_timer.h"


#define PAL_I2C_MASTER_MAX_BITRATE  (400U)

static volatile uint32_t g_entry_count = 0;
static pal_i2c_t * gp_pal_i2c_current_ctx;

static pal_status_t pal_i2c_acquire(const void * p_i2c_context)
{
    // To avoid compiler errors/warnings. This context might be used by a target 
    // system to implement a proper mutex handling
    (void)p_i2c_context;
    
    if (0 == g_entry_count)
    {
        g_entry_count++;
        if (1 == g_entry_count)
        {
            return PAL_STATUS_SUCCESS;
        }
    }
    return PAL_STATUS_FAILURE;
}

static void pal_i2c_release(const void * p_i2c_context)
{
    // To avoid compiler errors/warnings. This context might be used by a target 
    // system to implement a proper mutex handling
    (void)p_i2c_context;
    
    g_entry_count = 0;
}

void invoke_upper_layer_callback (const pal_i2c_t * p_pal_i2c_ctx, optiga_lib_status_t event)
{
    upper_layer_callback_t upper_layer_handler;

    upper_layer_handler = (upper_layer_callback_t)p_pal_i2c_ctx->upper_layer_event_handler;

    upper_layer_handler(p_pal_i2c_ctx->p_upper_layer_ctx, event);

    //Release I2C Bus
    pal_i2c_release(p_pal_i2c_ctx->p_upper_layer_ctx);
}

// The next 5 functions are required only in case you have interrupt based i2c implementation
void i2c_master_end_of_transmit_callback(void)
{
    invoke_upper_layer_callback(gp_pal_i2c_current_ctx, PAL_I2C_EVENT_SUCCESS);
}

void i2c_master_end_of_receive_callback(void)
{
    invoke_upper_layer_callback(gp_pal_i2c_current_ctx, PAL_I2C_EVENT_SUCCESS);
}

void i2c_master_error_detected_callback(void)
{
    invoke_upper_layer_callback(gp_pal_i2c_current_ctx, PAL_I2C_EVENT_ERROR);
}

void i2c_master_nack_received_callback(void)
{
    i2c_master_error_detected_callback();
}

void i2c_master_arbitration_lost_callback(void)
{
    i2c_master_error_detected_callback();
}

pal_status_t pal_i2c_init(const pal_i2c_t * p_i2c_context)
{
    (void)p_i2c_context;
    Cy_USB_I2CInit();
    return PAL_STATUS_SUCCESS;
}

pal_status_t pal_i2c_deinit(const pal_i2c_t * p_i2c_context)
{
    (void)p_i2c_context;

    /* A de-init is performed on every init, no need to explicitly de-init here. */
    return PAL_STATUS_SUCCESS;
}

pal_status_t pal_i2c_write(const pal_i2c_t * p_i2c_context, uint8_t * p_data, uint16_t length) {
    pal_status_t status = PAL_STATUS_FAILURE;
    
    //Acquire the I2C bus before read/write
    if (PAL_STATUS_SUCCESS == pal_i2c_acquire(p_i2c_context)) {

        cy_en_scb_i2c_status_t i2c_status;
        for(int i=0; i<3; i++){ /* Attempt 3 times. */
            i2c_status = cyi2c_master_write(SCB0, OPTIGA_FX_ADDR, p_data, length, true);
            Cy_SysLib_DelayUs(100);
            if(!i2c_status) break;  /* stop attempts once succeeded */
        }

        if(i2c_status) {
            //If I2C Master fails to invoke the write operation, invoke upper layer event handler with error.
            ((upper_layer_callback_t)(p_i2c_context->upper_layer_event_handler))
                                                       (p_i2c_context->p_upper_layer_ctx , PAL_I2C_EVENT_ERROR);
            
            //Release I2C Bus
            pal_i2c_release((void * )p_i2c_context);
        }
        else {
            pal_i2c_release((void * )p_i2c_context);
            
            /**
            * Infineon I2C Protocol is a polling based protocol, if foo_i2c_write will fail it will be reported to the 
            * upper layers by calling 
            * (p_i2c_context->upper_layer_event_handler))(p_i2c_context->p_upper_layer_ctx , PAL_I2C_EVENT_ERROR);
            * If the function foo_i2c_write() will succedd then two options are possible
            * 1. if foo_i2c_write() is interrupt based, then you need to configure interrupts in the function 
            *    pal_i2c_init() so that on a succesfull transmit interrupt the callback i2c_master_end_of_transmit_callback(),
            *    in case of successfull receive i2c_master_end_of_receive_callback() callback 
            *    in case of not acknowedged, arbitration lost, generic error i2c_master_nack_received_callback() or
            *    i2c_master_arbitration_lost_callback()
            * 2. If foo_i2c_write() is a blocking function which will return either ok or failure after transmitting data
            *    you can handle this case directly here and call 
            *    invoke_upper_layer_callback(gp_pal_i2c_current_ctx, PAL_I2C_EVENT_SUCCESS);
            *    
            */

            ((upper_layer_callback_t)(p_i2c_context->upper_layer_event_handler))(p_i2c_context->p_upper_layer_ctx , PAL_I2C_EVENT_SUCCESS);

            status = PAL_STATUS_SUCCESS;
        }
    }
    else {
        status = PAL_STATUS_I2C_BUSY;
        ((upper_layer_callback_t)(p_i2c_context->upper_layer_event_handler))
                                                        (p_i2c_context->p_upper_layer_ctx , PAL_I2C_EVENT_BUSY);
    }
    return status;
}

pal_status_t pal_i2c_read(const pal_i2c_t * p_i2c_context, uint8_t * p_data, uint16_t length)
{
    pal_status_t status = PAL_STATUS_FAILURE;
    // Acquire the I2C bus before read/write
    
    if (PAL_STATUS_SUCCESS == pal_i2c_acquire(p_i2c_context))
    {
        
        cy_en_scb_i2c_status_t i2c_status;
        i2c_status = cyi2c_master_read(SCB0, OPTIGA_FX_ADDR, p_data, length, true);

        //Invoke the low level i2c master driver API to read from the bus
        if(i2c_status) {
            //If I2C Master fails to invoke the read operation, invoke upper layer event handler with error.
            ((upper_layer_callback_t)(p_i2c_context->upper_layer_event_handler))
                                                       (p_i2c_context->p_upper_layer_ctx , PAL_I2C_EVENT_ERROR);

            //Release I2C Bus
            
            pal_i2c_release((void * )p_i2c_context);
        }
        else {
            
            /**
            * Similar to the foo_i2c_write() case you can directly call 
            * invoke_upper_layer_callback(gp_pal_i2c_current_ctx, PAL_I2C_EVENT_SUCCESS);
            * if you have blocking (non-interrupt) i2c calls
            */
            ((upper_layer_callback_t)(p_i2c_context->upper_layer_event_handler))(p_i2c_context->p_upper_layer_ctx , PAL_I2C_EVENT_SUCCESS);

            status = PAL_STATUS_SUCCESS;
        }
    }
    else
    {
        status = PAL_STATUS_I2C_BUSY;
        ((upper_layer_callback_t)(p_i2c_context->upper_layer_event_handler))
                                                        (p_i2c_context->p_upper_layer_ctx , PAL_I2C_EVENT_BUSY);
    }

    pal_i2c_release((void * )p_i2c_context);
    return status;
}

pal_status_t pal_i2c_set_bitrate(const pal_i2c_t * p_i2c_context, uint16_t bitrate)
{
    /* NOTE: The baud rate for the SCB which is used for I2C is already set. User will not have to re-specify it. */
    pal_status_t return_status;

    return_status = PAL_STATUS_SUCCESS;

    return return_status;
}

/**
* @}
*/
