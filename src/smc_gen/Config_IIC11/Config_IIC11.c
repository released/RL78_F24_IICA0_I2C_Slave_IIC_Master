/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2021, 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name        : Config_IIC11.c
* Component Version: 1.5.1
* Device(s)        : R7F124FPJ5xFB
* Description      : This file implements device driver for Config_IIC11.
***********************************************************************************************************************/
/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "Config_IIC11.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
volatile uint8_t   g_iic11_master_status_flag;    /* iic11 start flag for send address check */
volatile uint8_t * gp_iic11_tx_address;           /* iic11 send data pointer by master mode */
volatile uint16_t  g_iic11_tx_count;              /* iic11 send data size by master mode */
volatile uint8_t * gp_iic11_rx_address;           /* iic11 receive data pointer by master mode */
volatile uint16_t  g_iic11_rx_count;              /* iic11 receive data size by master mode */
uint16_t  g_iic11_rx_length;                      /* iic11 receive data length by master mode */
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Config_IIC11_Create
* Description  : This function initializes the IIC11 master module.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_IIC11_Create(void)
{
    SPS1 &= _00F0_SAU_CKM0_CLEAR;
    SPS1 |= _0001_SAU_CKM0_FCLK_1;
    ST1 |= _0002_SAU_CH1_STOP_TRG_ON;
    IICMK11 = 1U;    /* disable INTIIC11 interrupt */
    IICIF11 = 0U;    /* clear INTIIC11 interrupt flag */
    /* Set INTIIC11 low priority */
    IICPR111 = 1U;
    IICPR011 = 1U;
    /* clear error flag */
    SIR11 = _0004_SAU_FEF_CLEARED | _0002_SAU_PEF_CLEARED | _0001_SAU_OVF_CLEARED;
    SMR11 = _0020_SAU_SMRMN_INITIALVALUE | _0000_SAU_CLOCK_SELECT_CKM0 | _0000_SAU_CLOCK_MODE_CKS | 
            _0000_SAU_TRIGGER_SOFTWARE | _0000_SAU_EDGE_FALLING | _0004_SAU_MODE_IIC | _0000_SAU_TRANSFER_END;
    SCR11 = _0000_SAU_TIMING_1 | _0000_SAU_PARITY_NONE | _0000_SAU_MSB | _0010_SAU_STOP_1 | _0007_SAU_LENGTH_8;
    SDR11 = _C600_IIC11_DIVISOR;
    SO1 |= (_0200_SAU_CH1_CLOCK_OUTPUT_1 | _0002_SAU_CH1_DATA_OUTPUT_1);
    /* Set SCL11, SDA11 pin */
    POM7 |= 0x01U;
    PMC7 &= 0xFCU;
    P7 |= 0x03U;
    PM7 &= 0xFCU;

    R_Config_IIC11_Create_UserInit();
}

/***********************************************************************************************************************
* Function Name: R_Config_IIC11_Stop
* Description  : This function stops the IIC11 module.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_IIC11_Stop(void)
{
    ST1 |= _0002_SAU_CH1_STOP_TRG_ON;
    IICMK11 = 1U;    /* disable INTIIC11 interrupt */
    IICIF11 = 0U;    /* clear INTIIC11 interrupt flag */
}

/***********************************************************************************************************************
* Function Name: R_Config_IIC11_StartCondition
* Description  : This function starts the IIC11 condition.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_IIC11_StartCondition(void)
{
    volatile uint8_t w_count;

    SO1 &= (uint16_t)~_0002_SAU_CH1_DATA_OUTPUT_1;    /* clear IIC11 SDA */

    /* Set delay to secure a hold time after SDA output low. The delay time depend on slave device.
       Here set 5us as default base on current clock */
    for (w_count = 0U; w_count <= IIC11_WAITTIME; w_count++ )
    {
        NOP();
    }

    SO1 &= (uint16_t)~_0200_SAU_CH1_CLOCK_OUTPUT_1;    /* clear IIC11 SCL */
    SOE1 |= _0002_SAU_CH1_OUTPUT_ENABLE;    /* enable IIC11 output */
    SS1 |= _0002_SAU_CH1_START_TRG_ON;    /* enable IIC11 */

    /* Set delay to secure a hold time after SCL output low. The delay time depend on slave device.
       Here set 5us as default base on current clock */
    for (w_count = 0U; w_count <= IIC11_WAITTIME; w_count++ )
    {
        NOP();
    }
}

/***********************************************************************************************************************
* Function Name: R_Config_IIC11_StopCondition
* Description  : This function stops the IIC11 condition.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_IIC11_StopCondition(void)
{
    volatile uint8_t w_count;

    ST1 |= _0002_SAU_CH1_STOP_TRG_ON;    /* disable IIC11 */
    SOE1 &= (uint16_t)~_0002_SAU_CH1_OUTPUT_ENABLE;    /* disable IIC11 output */
    SO1 &= (uint16_t)~_0002_SAU_CH1_DATA_OUTPUT_1;    /* clear IIC11 SDA */

    /* Set delay to secure a hold time after SDA output low. The delay time depend on slave device.
       Here set 5us as default base on current clock */
    for (w_count = 0U; w_count <= IIC11_WAITTIME; w_count++ )
    {
        NOP();
    }

    SO1 |= _0200_SAU_CH1_CLOCK_OUTPUT_1;    /* set IIC11 SCL */

    /* Set delay to secure a hold time after SCL output low. The delay time depend on slave device.
       Here set 5us as default base on current clock */
    for (w_count = 0U; w_count <= IIC11_WAITTIME; w_count++ )
    {
        NOP();
    }

    SO1 |= _0002_SAU_CH1_DATA_OUTPUT_1;    /* set IIC11 SDA */

    /* Set delay to secure a hold time after SDA output low. The delay time depend on slave device.
       Here set 5us as default base on current clock */
    for (w_count = 0U; w_count <= IIC11_WAITTIME; w_count++ )
    {
        NOP();
    }
}

/***********************************************************************************************************************
* Function Name: R_Config_IIC11_Master_Send
* Description  : This function starts transferring data for IIC11 in master mode.
* Arguments    : adr -
*                    set address for select slave
*                tx_buf -
*                    transfer buffer pointer
*                tx_num -
*                    buffer size
* Return Value : None
***********************************************************************************************************************/
void R_Config_IIC11_Master_Send(uint8_t adr, uint8_t * const tx_buf, uint16_t tx_num)
{
    g_iic11_master_status_flag = _00_SAU_IIC_MASTER_FLAG_CLEAR;    /* clear IIC11 master status flag */
    adr &= 0xFEU;    /* send mode */
    g_iic11_master_status_flag = _01_SAU_IIC_SEND_FLAG;    /* set master status flag */
    SCR11 &= (uint16_t)~_C000_SAU_RECEPTION_TRANSMISSION;
    SCR11 |= _8000_SAU_TRANSMISSION;
    /* Set paramater */
    g_iic11_tx_count = tx_num;
    gp_iic11_tx_address = tx_buf;
    /* Start condition */
    R_Config_IIC11_StartCondition();
    IICIF11 = 0U;    /* clear INTIIC11 interrupt flag */
    IICMK11 = 0U;    /* enable INTIIC11 */
    SDR11L = adr;
}

/***********************************************************************************************************************
* Function Name: R_Config_IIC11_Master_Receive
* Description  : This function starts receiving data for IIC11 in master mode.
* Arguments    : adr -
*                    set address for select slave
*                rx_buf -
*                    receive buffer pointer
*                rx_num -
*                    buffer size
* Return Value : None
***********************************************************************************************************************/
void R_Config_IIC11_Master_Receive(uint8_t adr, uint8_t * const rx_buf, uint16_t rx_num)
{
    g_iic11_master_status_flag = _00_SAU_IIC_MASTER_FLAG_CLEAR;    /* clear master status flag */
    adr |= 0x01U;    /* receive mode */
    g_iic11_master_status_flag = _02_SAU_IIC_RECEIVE_FLAG;    /* set master status flag */
    SCR11 &= (uint16_t)~_C000_SAU_RECEPTION_TRANSMISSION;
    SCR11 |= _8000_SAU_TRANSMISSION;
    /* Set parameter */
    g_iic11_rx_length = rx_num;
    g_iic11_rx_count = 0U;
    gp_iic11_rx_address = rx_buf;
    /* Start condition */
    R_Config_IIC11_StartCondition();
    IICIF11 = 0U;    /* clear INTIIC11 interrupt flag */
    IICMK11 = 0U;    /* enable INTIIC11 */
    SDR11L = adr;
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

