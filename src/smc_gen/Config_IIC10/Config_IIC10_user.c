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
* File Name        : Config_IIC10_user.c
* Component Version: 1.5.1
* Device(s)        : R7F124FPJ5xFB
* Description      : This file implements device driver for Config_IIC10.
***********************************************************************************************************************/
/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "Config_IIC10.h"
/* Start user code for include. Do not edit comment generated here */
#include "custom_func.h"
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#pragma interrupt r_Config_IIC10_interrupt(vect=INTIIC10)
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
extern volatile uint8_t g_iic10_master_status_flag;      /* iic10 start flag for send address check */
extern volatile uint8_t * gp_iic10_tx_address;           /* iic10 send data pointer by master mode */
extern volatile uint16_t g_iic10_tx_count;               /* iic10 send data size by master mode */
extern volatile uint8_t * gp_iic10_rx_address;           /* iic10 receive data pointer by master mode */
extern volatile uint16_t g_iic10_rx_count;               /* iic10 receive data size by master mode */
extern uint16_t g_iic10_rx_length;                       /* iic10 receive data length by master mode */
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Config_IIC10_Create_UserInit
* Description  : This function adds user code after initializing IIC10.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_IIC10_Create_UserInit(void)
{
    /* Start user code for user init. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_IIC10_callback_master_sendend
* Description  : This function is a callback function when IIC10 finishes master transmission.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_Config_IIC10_callback_master_sendend(void)
{
    /* Start user code for r_Config_IIC10_callback_master_sendend. Do not edit comment generated here */
    IIC10_sendend_condition();
    IIC10_Master_flag.tx_end = 0U;
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_IIC10_callback_master_receiveend
* Description  : This function is a callback function when IIC10 finishes master reception.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_Config_IIC10_callback_master_receiveend(void)
{
    /* Start user code for r_Config_IIC10_callback_master_receiveend. Do not edit comment generated here */
    R_Config_IIC10_StopCondition();
    IIC10_Master_flag.rx_end = 0U;
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_IIC10_callback_master_error
* Description  : This function is a callback function when IIC10 master error occurs.
* Arguments    : flag -
*                    status flag
* Return Value : None
***********************************************************************************************************************/
static void r_Config_IIC10_callback_master_error(MD_STATUS flag)
{
    /* Start user code for r_Config_IIC10_callback_master_error. Do not edit comment generated here */
    drv_IIC10_callback_error(flag);
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_IIC10_interrupt
* Description  : This function is INTIIC10 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void __near r_Config_IIC10_interrupt(void)
{
    
    volatile uint16_t w_count;

    /* Set delay to secure a hold time after SDA, SDL output. The delay time depend on slave device.
       Here set 20us as default base on current clock */
    for (w_count = 0U; w_count <= IIC10_WAITTIME_2; w_count++ )
    {
        NOP();
    }

    if ((0x0002U == (SSR10 & _0002_SAU_PARITY_ERR_OCCUR)) && (0U != g_iic10_tx_count))
    {
        SIR10 |= _0002_SAU_PEF_CLEARED;    /* clear ACK error detection flag */
        R_Config_IIC10_StopCondition();
        r_Config_IIC10_callback_master_error(MD_NACK);
    }
    else if ((0x0001U == (SSR10 & _0001_SAU_OVERRUN_OCCUR)) && (0U != g_iic10_tx_count))
    {
        SIR10 |= _0001_SAU_OVF_CLEARED;    /* clear overrun error detection flag */
        R_Config_IIC10_StopCondition();
        r_Config_IIC10_callback_master_error(MD_OVERRUN);
    }
    else
    {
        /* Control for master send */
        if (1U == (g_iic10_master_status_flag & _01_SAU_IIC_SEND_FLAG))
        {
            if (g_iic10_tx_count > 0U)
            {
                SDR10L = *gp_iic10_tx_address;
                gp_iic10_tx_address++;
                g_iic10_tx_count--;
            }
            else
            {
                /* IIC master transmission finishes and a callback function can be called here. */
                r_Config_IIC10_callback_master_sendend();
            }
        }
        /* Control for master receive */
        else 
        {
            if (0U == (g_iic10_master_status_flag & _04_SAU_IIC_SENDED_ADDRESS_FLAG))
            {
                ST1 |= _0001_SAU_CH0_STOP_TRG_ON;
                SCR10 &= (uint16_t)~_C000_SAU_RECEPTION_TRANSMISSION;
                SCR10 |= _4000_SAU_RECEPTION;
                SS1 |= _0001_SAU_CH0_START_TRG_ON;
                g_iic10_master_status_flag |= _04_SAU_IIC_SENDED_ADDRESS_FLAG;

                if (1U == g_iic10_rx_length)
                {
                    SOE1 &= (uint16_t)~_0001_SAU_CH0_OUTPUT_ENABLE;    /* disable IIC10 out */
                }

                SDR10L = 0xFFU;
            }
            else
            {
                if (g_iic10_rx_count < g_iic10_rx_length)
                {
                    *gp_iic10_rx_address = SDR10L;
                    gp_iic10_rx_address++;
                    g_iic10_rx_count++;

                    if ((g_iic10_rx_length - 1U) == g_iic10_rx_count)
                    {
                        SOE1 &= (uint16_t)~_0001_SAU_CH0_OUTPUT_ENABLE;    /* disable IIC10 out */
                        SDR10L = 0xFFU;
                    }
                    else if (g_iic10_rx_count == g_iic10_rx_length)
                    {
                        /* IIC master reception finishes and a callback function can be called here. */
                        r_Config_IIC10_callback_master_receiveend();
                    }
                    else
                    {
                        SDR10L = 0xFFU;
                    }
                }
            }
        }
    }
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

