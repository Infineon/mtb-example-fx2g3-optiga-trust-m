/***************************************************************************//**
* \file usb_i2c.c
* \version 1.0
*
* \details Implements the I2C data handling
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

/* Includes */
#include "cy_pdl.h"
#include "cy_debug.h"
#include "usb_i2c.h"
#include "pal_custom.h"

/* Global variables */
cy_stc_scb_i2c_context_t I2C_context;
uint8_t scb0_i2c_buffer[I2C_BUFF_SIZE];

cy_stc_scb_i2c_master_xfer_config_t write_i2c_slave = {
        .slaveAddress = FPGASLAVE_ADDR,
        .buffer = &scb0_i2c_buffer[0],
        .bufferSize = I2C_BUFF_SIZE,
        .xferPending = false
};


/* Functions */

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
                        uint16_t size, bool send_stop)
{
    uint32_t timeout = 0;

    cy_en_scb_i2c_command_t ack = CY_SCB_I2C_ACK;

    /* Start transaction, send dev_addr */
    cy_en_scb_i2c_status_t status = (I2C_context.state == CY_SCB_I2C_IDLE)
        ? Cy_SCB_I2C_MasterSendStart(base, dev_addr, CY_SCB_I2C_READ_XFER, timeout,  &I2C_context)
        : Cy_SCB_I2C_MasterSendReStart(base, dev_addr, CY_SCB_I2C_READ_XFER, timeout,  &I2C_context);

    if (status == CY_SCB_I2C_SUCCESS)
    {
        while (size > 0) {
            if (size == 1)
            {
                ack = CY_SCB_I2C_NAK;
            }
            status = Cy_SCB_I2C_MasterReadByte(base, ack, (uint8_t *)data, timeout,  &I2C_context);
            if (status != CY_SCB_I2C_SUCCESS)
            {
                break;
            }
            --size;
            ++data;
        }
    }

    if (send_stop)
    {
        /* SCB in I2C mode is very time sensitive. In practice we have to request STOP after */
        /* each block, otherwise it may break the transmission */
        Cy_SCB_I2C_MasterSendStop(base, timeout,  &I2C_context);
    }
    return status;
}

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
                                            uint16_t size, bool send_stop)
{
    /* Clean-up hardware before transfer. Note RX FIFO is empty at here. */
    Cy_SCB_ClearMasterInterrupt(base, CY_SCB_I2C_MASTER_INTR_ALL);
    Cy_SCB_ClearTxFifo(base);
    cy_en_scb_i2c_status_t status;
    status = (I2C_context.state == CY_SCB_I2C_IDLE)
        ? Cy_SCB_I2C_MasterSendStart(base, dev_addr, CY_SCB_I2C_WRITE_XFER, 0, &I2C_context)
        : Cy_SCB_I2C_MasterSendReStart(base, dev_addr, CY_SCB_I2C_WRITE_XFER,0, &I2C_context);

    if (status == CY_SCB_I2C_SUCCESS)
    {
        while (size > 0)
        {
            status = Cy_SCB_I2C_MasterWriteByte(base, *data, 0, &I2C_context);
            if (status != CY_SCB_I2C_SUCCESS)
            {
                break;
            }
            --size;
            ++data;
        }
    }

    if (send_stop)
    {
        /* SCB in I2C mode is very time sensitive. In practice we have to request STOP after */
        /* each block, otherwise it may break the transmission */
        Cy_SCB_I2C_MasterSendStop(base, 0, &I2C_context);
    }

    return status;
}

/**
 * \name I2CMaster_ISR
 * \details Does some init for running rtos based auto reload software timer
 *          which periodically enables ITP interrupt for LDM exchange mechanism
 * \retval None
 */
void I2CMaster_ISR(void)
{
    Cy_SCB_I2C_Interrupt(SCB0, &I2C_context);
}

/**
 * \name ConfigureSCB0Clock
 * \brief Configure the I2C clock for SCB0 with divider #3
 * \retval None
 */
void ConfigureSCB0Clock(uint8_t scbIndex)
{
    /* Get the PERI clock frequency for the platform. */
    uint32_t hfClkFreq = Cy_SysClk_ClkPeriGetFrequency();

    /* Configure PERI 16 bit clock divider#3 for 3 MHz operation and enable it. */
    switch (hfClkFreq)
    {
        case 50000000UL:
            /* Divide 50 MHz by 16 to get 3 MHz. */
            Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 3, 15);
            break;

        case 60000000UL:
            /* Divide 60 MHz by 20 to get 3 MHz. */
            Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 3, 19);
            break;

        case 75000000UL:
            /* Divide 75 MHz by 25 to get 3 MHz. */
            Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 3, 24);
            break;

        case 100000000UL:
            /* Divide 100 MHz by 33 to get 3 MHz. */
            Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 3, 32);
            break;

        default:
            break;
    }

    DBG_APP_INFO("scbIndex: %d, hfClkFreq: %d \n\r", 3, hfClkFreq);

    Cy_SysClk_PeriphEnableDivider (CY_SYSCLK_DIV_16_BIT, 3);
    Cy_SysLib_DelayUs (10);

    /* Connect the PERI clock to the SCB input. */
    Cy_SysClk_PeriphAssignDivider((en_clk_dst_t)(PCLK_SCB0_CLOCK + scbIndex), CY_SYSCLK_DIV_16_BIT, 3);
}

/**
 * \name Scb0i2cMasterEvent
 * \brief i2c master event callback function, handling various i2c master event
 * \retval None
 */
void Scb0i2cMasterEvent(uint32_t Events)
{
    switch (Events)
    {
    case CY_SCB_I2C_MASTER_WR_IN_FIFO_EVENT:
        break;
    case CY_SCB_I2C_MASTER_WR_CMPLT_EVENT:
        break;
    case CY_SCB_I2C_MASTER_RD_CMPLT_EVENT:
        break;
    case CY_SCB_I2C_MASTER_ERR_EVENT:
        break;
    default:
        break;
    }
}

void Cy_USB_AppInitIntr(cy_israddress userIsr)
{
    cy_stc_sysint_t intrCfg;
#if (!CY_CPU_CORTEX_M4)
    intrCfg.intrSrc = NvicMux3_IRQn;
    intrCfg.intrPriority = 1;
    intrCfg.cm0pSrc = scb_0_interrupt_IRQn;
    Cy_SysInt_Init(&intrCfg, I2CMaster_ISR);
#else
    intrCfg.intrSrc = (IRQn_Type) scb_0_interrupt_IRQn;
    intrCfg.intrPriority = 1u;
    Cy_SysInt_Init(&intrCfg, I2CMaster_ISR);
#endif /* (!CY_CPU_CORTEX_M4) */

    if (userIsr != NULL)
    {
        /* If an ISR is provided, register it and enable the interrupt. */
        Cy_SysInt_Init(&intrCfg, userIsr);
        NVIC_EnableIRQ(intrCfg.intrSrc);
    }
    else
    {
        /* ISR is NULL. Disable the interrupt. */
        NVIC_DisableIRQ(intrCfg.intrSrc);
    }

}

/**
 * \name Cy_USB_I2CInit
 * \brief Initialize I2C on SCB0
 * \retval None
 */
void Cy_USB_I2CInit (void)
{
    cy_stc_gpio_pin_config_t    pinCfg;
    cy_stc_scb_i2c_config_t     i2cCfg;
    static uint32_t dataRate;
    static uint32_t dataClock;

    Cy_SCB_I2C_DeInit(SCB0);
    Cy_USB_AppInitIntr(NULL);
    ConfigureSCB0Clock(0);
    memset ((void *)&pinCfg, 0, sizeof(pinCfg));
    memset ((void *)&i2cCfg, 0, sizeof(i2cCfg));

    /* Configure SCB0 pins (P10.0 and P10.1) in Open-Drain drive mode. */
    pinCfg.driveMode = CY_GPIO_DM_OD_DRIVESLOW;
    pinCfg.hsiom     = P10_0_SCB0_I2C_SCL;
    Cy_GPIO_Pin_Init(P10_0_PORT, P10_0_PIN, &pinCfg);

    pinCfg.hsiom     = P10_1_SCB0_I2C_SDA;
    Cy_GPIO_Pin_Init(P10_1_PORT, P10_1_PIN, &pinCfg);

    i2cCfg.i2cMode = CY_SCB_I2C_MASTER;
    i2cCfg.useRxFifo = true;
    i2cCfg.useTxFifo = true;
    i2cCfg.slaveAddress = OPTIGA_FX_ADDR;
    i2cCfg.slaveAddressMask = 0x0;
    i2cCfg.lowPhaseDutyCycle = 7;
    i2cCfg.highPhaseDutyCycle = 5;


    Cy_SCB_I2C_Init (SCB0, &i2cCfg, &I2C_context);
    
    dataClock = Cy_SysClk_PeriphGetFrequency(CY_SYSCLK_DIV_16_BIT, 3);
    dataRate = Cy_SCB_I2C_SetDataRate(SCB0, I2C_DATARATE, I2C_INCLK_TARGET_FREQ);

    /* Register interrupt handler for SCB-I2C. */
    DBG_APP_INFO("I2C dataClock: %d\r\n", dataClock);
    DBG_APP_INFO("I2C dataRate: %d\r\n", dataRate);
    DBG_APP_INFO("I2C Slave Address: 0x%x\r\n", i2cCfg.slaveAddress);
    if ((dataRate > I2C_DATARATE) || (dataRate == 0U))
    {
        /* Can not reach desired data rate */
        while(1);
    }
    Cy_SCB_ClearRxFifo(SCB0);
    Cy_SCB_ClearTxFifo(SCB0);
    
    Cy_SCB_I2C_RegisterEventCallback(SCB0,(cy_cb_scb_i2c_handle_events_t) Scb0i2cMasterEvent,
                                    &I2C_context);  

    Cy_USB_AppInitIntr(I2CMaster_ISR);                              

    Cy_SCB_I2C_Enable (SCB0);

}

/* End of File */
