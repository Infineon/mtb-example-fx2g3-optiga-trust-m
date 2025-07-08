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
* \file pal_gpio.c
*
* \brief   This file implements the platform abstraction layer APIs for GPIO.
*
* \ingroup  grPAL
*
* @{
*/

#include "pal_gpio.h"
#include "pal_custom.h"          /* Header located in `fx_pal_include` directory */
#include "cy_gpio.h"

pal_status_t pal_gpio_init(const pal_gpio_t * p_gpio_context) {

    /* Set status to success initially. */
    pal_status_t status = PAL_STATUS_SUCCESS;

    /* Hold return value of Cy_GPIO_Pin_Init. */
    uint32_t gpio_status = CY_GPIO_SUCCESS;

    /* Ensure that context and the struct aren't empty */
    if ((p_gpio_context != NULL) && (p_gpio_context->p_gpio_hw != NULL)) {

        /* Get the values from the struct.
         * [Members of pointer to struct]
         */
        cy_stc_gpio_pin_config_t * pinCfg = (cy_stc_gpio_pin_config_t *)(&(((cy_stc_gpio_ctx_t *)(p_gpio_context->p_gpio_hw))->fx_gpio_pinCfg));
        GPIO_PRT_Type * PORT = (GPIO_PRT_Type *)(&(((cy_stc_gpio_ctx_t *)(p_gpio_context->p_gpio_hw))->fx_gpio_port));
        uint32_t PIN = ((cy_stc_gpio_ctx_t *)(p_gpio_context->p_gpio_hw))->fx_gpio_pin;

        /* Drivemode is set in pinCfg->Drivemode */
        gpio_status = Cy_GPIO_Pin_Init(PORT, PIN, pinCfg);

        /* Set the return status based on Cy_GPIO_Pin_Init result */
        status = (gpio_status == CY_GPIO_SUCCESS) ? PAL_STATUS_SUCCESS : PAL_STATUS_FAILURE;
    }
    else {

        /* If struct or ctx are empty, return failure */
        status = PAL_STATUS_FAILURE;
    }

    return (status);
}

pal_status_t pal_gpio_deinit(const pal_gpio_t * p_gpio_context) {

    /* Set status to success initially. */
    pal_status_t status = PAL_STATUS_SUCCESS;

    return (status);
}

void pal_gpio_set_high(const pal_gpio_t * p_gpio_context) {

    /* Ensure that context and the struct aren't empty */
    if ((p_gpio_context != NULL) && (p_gpio_context->p_gpio_hw != NULL)) {

        GPIO_PRT_Type * PORT = (GPIO_PRT_Type *)(&(((cy_stc_gpio_ctx_t *)(p_gpio_context->p_gpio_hw))->fx_gpio_port));
        uint32_t PIN = ((cy_stc_gpio_ctx_t *)(p_gpio_context->p_gpio_hw))->fx_gpio_pin;

        Cy_GPIO_Set(PORT, PIN);
    }
}

void pal_gpio_set_low(const pal_gpio_t * p_gpio_context) {

    /* Ensure that context and the struct aren't empty */
    if ((p_gpio_context != NULL) && (p_gpio_context->p_gpio_hw != NULL)) {

        GPIO_PRT_Type * PORT = (GPIO_PRT_Type *)(&(((cy_stc_gpio_ctx_t *)(p_gpio_context->p_gpio_hw))->fx_gpio_port));
        uint32_t PIN = ((cy_stc_gpio_ctx_t *)(p_gpio_context->p_gpio_hw))->fx_gpio_pin;

        Cy_GPIO_Clr(PORT, PIN);
    }
}

/**
* @}
*/
