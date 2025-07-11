/**
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
*
* \file
*
* \brief This file implements the platform abstraction layer APIs for timer.
*
* \ingroup  grPAL
* @{
*/

#include "pal_os_timer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cy_syslib.h"

static volatile uint32_t g_tick_count = 0;

/// @cond hidden 

/**
* ISR handler for counting 1 milliseconds ticks
*/
void delay_timer_isr(void)
{
    g_tick_count += 1u;
}

/// @endcond

/**
* Get the current time in milliseconds<br>
*
*
* \retval  uint32_t time in milliseconds
*/
uint32_t pal_os_timer_get_time_in_milliseconds(void)
{
    return (uint32_t)xTaskGetTickCount();
}

uint32_t pal_os_timer_get_time_in_microseconds(void)
{
    uint32_t time_in_ms = pal_os_timer_get_time_in_milliseconds();
    return (time_in_ms*1000);
}

/**
* Waits or delays until the given milliseconds time
*
* \param[in] milliseconds Delay value in milliseconds
*
*/
void pal_os_timer_delay_in_milliseconds(uint16_t milliseconds)
{
    Cy_SysLib_Delay(milliseconds);
}

/**
* @}
*/
