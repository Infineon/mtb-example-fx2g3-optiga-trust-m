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
 * \file pal_os_event.c
 *
 * \brief   This file implements the platform abstraction layer APIs for os event/scheduler.
 *
 * \ingroup  grPAL
 *
 * @{
 */

/* Includes */
#include "pal_os_event.h"

/* RTOS includes */
#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"

/* Includes for logging */
#include "optiga_app.h"

/* Global timer variables */
TimerHandle_t fx_optiga_timer;
volatile bool timer_created = false;

/* Global event instance */
pal_os_event_t pal_os_event_0 = {0};

void pal_os_event_start(pal_os_event_t * p_pal_os_event, register_callback callback, void * callback_args) {
    if (0 == p_pal_os_event->is_event_triggered) {
        p_pal_os_event->is_event_triggered = TRUE;
        pal_os_event_register_callback_oneshot(p_pal_os_event, callback, callback_args, 1000);
    }
}

void pal_os_event_stop(pal_os_event_t * p_pal_os_event) {
    p_pal_os_event->is_event_triggered = FALSE;
}

pal_os_event_t * pal_os_event_create(register_callback callback, void * callback_args) {
    if (( NULL != callback )&&( NULL != callback_args ))
    {
        pal_os_event_start(&pal_os_event_0, callback, callback_args);
    }
    return (&pal_os_event_0);
}

void Cy_PAL_CbkWrapper(TimerHandle_t xTimer){
    pal_os_event_trigger_registered_callback();
}

void pal_os_event_trigger_registered_callback(void) {

    register_callback callback;

    if (pal_os_event_0.callback_registered) {
        callback = pal_os_event_0.callback_registered;
        callback((void * )pal_os_event_0.callback_ctx);
    }

}

void pal_os_event_register_callback_oneshot(pal_os_event_t * p_pal_os_event,
                                             register_callback callback,
                                             void * callback_args,
                                             uint32_t time_us) {
    p_pal_os_event->callback_registered = callback;
    p_pal_os_event->callback_ctx = callback_args;

    if(!timer_created){
        uint32_t time_ms = time_us<1000?1:(uint32_t)(time_us/1000);
        /** \note A wrapper `Cy_PAL_CbkWrapper` is used because xTimerCreate expects a cbk function with an xTimerHandle_t param. */
        fx_optiga_timer = xTimerCreate("fx_optiga_timer_n", time_ms, pdFALSE, p_pal_os_event, Cy_PAL_CbkWrapper);
        timer_created = true;
    }
    xTimerStart(fx_optiga_timer, 0);
}

void pal_os_event_destroy(pal_os_event_t * pal_os_event) {
    (void)pal_os_event;

    uint32_t timerStopStatus = pdPASS;
    uint32_t timerDeleteStatus = pdPASS;

    timerStopStatus = xTimerStop(fx_optiga_timer, 0);
    timerDeleteStatus = xTimerDelete(fx_optiga_timer, 0);

    if(timerStopStatus==pdPASS && timerDeleteStatus==pdPASS){
        timer_created = false;
    } else {
        OPTIGA_LOG_ERROR("Event Destroy Failed. Status - [xTimerStop: 0x%x, xTimerDelete: 0x%x]", timerStopStatus, timerDeleteStatus);
    }
}

/**
* @}
*/
