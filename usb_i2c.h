/***************************************************************************//**
* \file usb_i2c.h
* \version 1.0
*
* \brief Defines I2C related macros and functions
*
*******************************************************************************
* \copyright
* (c) (2021-2025), Cypress Semiconductor Corporation (an Infineon company) or
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

#ifndef _CY_USB_I2C_H_
#define _CY_USB_I2C_H_

#include "cy_pdl.h"
#include "cy_debug.h"
#include "cy_usbhs_dw_wrapper.h"
#include "cy_device.h"
#include "FreeRTOS.h"
#include "timers.h"

#define FPGASLAVE_ADDR                 (0x0D)       //FPGA i2c address (Do not change)

/* I2C Related macro */
#define I2C_READ                       (1)
#define I2C_WRITE                      (0)
#define FPGA_I2C_ADDRESS_WIDTH         (2)
#define FPGA_I2C_DATA_WIDTH            (1)
#define I2C_BUFF_SIZE                  (10)
#define I2C_DATARATE                   (100000)
#define I2C_INCLK_TARGET_FREQ          (3200000)
/* Get the LS byte from a 16-bit number */
#define CY_GET_LSB(w)                              ((uint8_t)((w) & UINT8_MAX))

/* Get the MS byte from a 16-bit number */
#define CY_GET_MSB(w)                              ((uint8_t)((w) >> 8))

struct cy_stc_usb_app_ctxt_
{
    uint8_t firstInitDone;
    cy_en_usb_device_state_t devState;
    cy_en_usb_device_state_t prevDevState;
    cy_en_usb_speed_t devSpeed;
    uint8_t devAddr;
    uint8_t activeCfgNum;
    cy_en_usb_enum_method_t enumMethod;
    uint8_t prevAltSetting;
    cy_en_usb_speed_t desiredSpeed;

    cy_stc_app_endp_dma_set_t endpInDma[CY_USB_MAX_ENDP_NUMBER];
    cy_stc_app_endp_dma_set_t endpOutDma[CY_USB_MAX_ENDP_NUMBER];
    DMAC_Type *pCpuDmacBase;
    DW_Type *pCpuDw0Base;
    DW_Type *pCpuDw1Base;

    cy_stc_usb_usbd_ctxt_t *pUsbdCtxt;
    bool usbConnectDone;
    bool vbusChangeIntr;                        /* VBus change interrupt received flag. */
    bool vbusPresent;                           /* VBus presence indicator flag. */
    bool usbConnected;                          /* Whether USB connection is enabled. */
    TimerHandle_t vbusDebounceTimer;            /* VBus change debounce timer handle. */
    uint32_t *pUsbEvtLogBuf;
    TimerHandle_t evtLogTimer;                  /* Timer to print eventLog. */
};
typedef struct cy_stc_usb_app_ctxt_ cy_stc_usb_app_ctxt_t;

extern cy_stc_scb_i2c_context_t I2C_context;

/* Function prototypes */
/**
 * \name Cy_USB_I2CInit
 * \brief Initialize I2C on SCB0
 * \retval None
 */
void Cy_USB_I2CInit (void);

/**
 * \name Scb0i2cMasterEvent
 * \brief i2c master event callback function, handling various i2c master event
 * \retval None
 */
void Scb0i2cMasterEvent(uint32_t Events);

/**
 * \name cyi2c_master_read
 * \brief Read I2C
 * \param base
 * \param dev_addr
 * \param data
 * \param size
 * \param send_stop
 * \retval status I2C read operation exit code
 */
cy_en_scb_i2c_status_t cyi2c_master_read(CySCB_Type *base, uint16_t dev_addr, uint8_t *data, 
    uint16_t size, bool send_stop);

/**
 * \name cyi2c_master_write
 * \brief Write I2C
 * \param base
 * \param dev_addr
 * \param data
 * \param size
 * \param send_stop
 * \retval status I2C write operation exit code
 */
cy_en_scb_i2c_status_t cyi2c_master_write(CySCB_Type *base, uint16_t dev_addr, const uint8_t *data, 
    uint16_t size, bool send_stop);

#endif //End _CY_USB_i2C_H_
