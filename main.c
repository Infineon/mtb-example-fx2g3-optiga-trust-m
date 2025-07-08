/***************************************************************************//**
* \file main.c
* \version 1.0
*
* \brief Main source file of the FX2G3 Optiga Trust M application.
*
*******************************************************************************
* \copyright
* (c) (2021-2023), Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.
*
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/* FreeRTOS includes */
#include "FreeRTOS.h"

/* Library stack includes */
#include "cy_debug.h"
#include "cybsp.h"
#include "../app_version.h"
#include "cy_pdl.h"

/* Optiga related includes */
#include "optiga_app.h"
#include <stdint.h>

#if DEBUG_INFRA_EN
/* Debug log related initilization */
TaskHandle_t printLogTaskHandle;
#endif /* DEBUG_INFRA_EN */

/* Global variables */
cy_stc_usb_usbd_ctxt_t usbdCtxt;
cy_stc_usb_app_ctxt_t appCtxt;
cy_stc_usb_cal_ctxt_t hsCalCtxt;
uint32_t hfclkFreq = BCLK__BUS_CLK__HZ;

/* extern functions */
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);
extern void vPortSVCHandler(void);

/* RAM buffer used to hold debug log data. */
#define LOGBUF_RAM_SZ           (1024U)

/* Select SCB interface used for UART based logging. */
#define LOGGING_SCB             (SCB4)
#define LOGGING_SCB_IDX         (4)

void SysTickIntrWrapper (void)
{
    Cy_USBD_TickIncrement(&usbdCtxt);
    xPortSysTickHandler();
}

void vPortSetupTimerInterrupt(void)
{
    /* Register the exception vectors. */
    Cy_SysInt_SetVector(PendSV_IRQn, xPortPendSVHandler);
    Cy_SysInt_SetVector(SVCall_IRQn, vPortSVCHandler);
    Cy_SysInt_SetVector(SysTick_IRQn, SysTickIntrWrapper);

    /* Start the SysTick timer with a period of 1 ms. */
    Cy_SysTick_SetClockSource(CY_SYSTICK_CLOCK_SOURCE_CLK_CPU);
    Cy_SysTick_SetReload(hfclkFreq / 1000U);
    Cy_SysTick_Clear();
    Cy_SysTick_Enable();
}

#if DEBUG_INFRA_EN
void PrintTaskHandler(void *pTaskParam)
{
    while (1)
    {
        /* Print any pending logs to the output console. */
        Cy_Debug_PrintLog();

        /* Put the thread to sleep for 5 ms */
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
#endif /* DEBUG_INFRA_EN */

/**
 * \name PrintVersionInfo
 * \brief Function to print version information to UART console
 * \param type Type of version string
 * \param version Version number including major, minor, patch and build number
 * \retval None
 */
void PrintVersionInfo(const char *type, uint32_t version)
{
    char tString[32];
    uint16_t vBuild;
    uint8_t vMajor, vMinor, vPatch;
    uint8_t typeLen = strlen(type);

    vMajor = (version >> 28U);
    vMinor = ((version >> 24U) & 0x0FU);
    vPatch = ((version >> 16U) & 0xFFU);
    vBuild = (uint16_t)(version & 0xFFFFUL);

    memcpy(tString, type, typeLen);
    tString[typeLen++] = '0' + (vMajor / 10);
    tString[typeLen++] = '0' + (vMajor % 10);
    tString[typeLen++] = '.';
    tString[typeLen++] = '0' + (vMinor / 10);
    tString[typeLen++] = '0' + (vMinor % 10);
    tString[typeLen++] = '.';
    tString[typeLen++] = '0' + (vPatch / 10);
    tString[typeLen++] = '0' + (vPatch % 10);
    tString[typeLen++] = '.';
    tString[typeLen++] = '0' + (vBuild / 1000);
    tString[typeLen++] = '0' + ((vBuild % 1000) / 100);
    tString[typeLen++] = '0' + ((vBuild % 100) / 10);
    tString[typeLen++] = '0' + (vBuild % 10);
    tString[typeLen++] = '\r';
    tString[typeLen++] = '\n';
    tString[typeLen] = 0;

    DBG_APP_INFO("%s", tString);
}

/**
 * \name Logging_Init
 * \brief Initialize logging framework
 * \retval None
 */
void Logging_Init (void)
{
    /* Initialize the UART for logging. */
#define DEBUG_LEVEL 3u
    uint8_t logBuf[LOGBUF_RAM_SZ];
    cy_stc_debug_config_t dbgCfg;

#if USBFS_LOGS_ENABLE
    dbgCfg.pBuffer = logBuf;
    dbgCfg.traceLvl = DEBUG_LEVEL;
    dbgCfg.bufSize = LOGBUF_RAM_SZ;
    dbgCfg.dbgIntfce = CY_DEBUG_INTFCE_USBFS_CDC;
    dbgCfg.printNow = true;
#else
    dbgCfg.pBuffer = logBuf;
    dbgCfg.traceLvl = DEBUG_LEVEL;
    dbgCfg.bufSize = LOGBUF_RAM_SZ;
    dbgCfg.dbgIntfce = CY_DEBUG_INTFCE_UART_SCB4;
    dbgCfg.printNow = true;

    InitUart(LOGGING_SCB_IDX);
#endif /* USBFS_LOGS_ENABLE */
    
    Cy_Debug_LogInit(&dbgCfg);
}

/**
 * \name OptigaApplication
 * \brief A wrapper function to enable optiga application flow
 * \param nothing A dummy parameter, to satisfy xTaskCreate's function expectations
 * \retval None
 */
void OptigaApplication(void * nothing){
#if USBFS_LOGS_ENABLE
    vTaskDelay(5000);
#endif
    Cy_Optiga_Init();
    Cy_Optiga_Main();
    Cy_Optiga_Deinit();

    while(true);
}

/**
 * \name Optiga_App_Init
 * \brief Create task for optiga application
 * 
 */
void Optiga_App_Init(void){
        long status;
        TaskHandle_t xOptiga_App_InitHandle = NULL;
        status = xTaskCreate( OptigaApplication, "fx_opt_task", 2048, NULL, 12, &xOptiga_App_InitHandle);

        if (status != pdPASS) {
            DBG_APP_ERR("fx_opt_task - TaskCreateFail\r\n");
            return;
        }
}

/**
 * \name main
 * \brief Entry to the program
 * \retval None
 */
int main (void)
{
    /* Initialize the PDL driver library and set the clock variables. */
    Cy_PDL_Init (&cy_deviceIpBlockCfgFX3G2);

    /* Initialize the device and board peripherals */
	cybsp_init();

    /* Initialize appCtxt */
    memset((uint8_t *)&appCtxt, 0, sizeof(appCtxt));

    /* Initialize the PDL and register ISR for USB block. */
    Logging_Init();

#if DEBUG_INFRA_EN
    
    Cy_Debug_AddToLog(1, "********** FX2G3: Optiga Trust M Application **********\r\n");
    
    /* Print application, USBD stack and HBDMA version information. */
    PrintVersionInfo("APP_VERSION: ", APP_VERSION_NUM);

    /* Create task for printing logs and check status. */
    xTaskCreate(PrintTaskHandler, "PrintLogTask", 512, NULL, 5, &printLogTaskHandle);
#endif /* DEBUG_INFRA_EN */

    /* Initialize the HbDma IP and DMA Manager */
    Cy_Optiga_HbDmaInit();

    /* Initialise task for OptigaApplication function. */
    Optiga_App_Init();

    /* Invokes scheduler: Not expected to return. */
    vTaskStartScheduler();
    while (1);

    return 0;
}

/* [] END OF FILE */
