/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>

#include "retarget.h"
#include "misc_config.h"
#include "custom_func.h"

#include "Config_IIC10.h"

#include "IIC10_simple_i2c_driver.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

struct flag_32bit flag_IIC10_CTL;
#define FLAG_IIC10_REVERSE0                 		    (flag_IIC10_CTL.bit0)
#define FLAG_IIC10_REVERSE1                  		    (flag_IIC10_CTL.bit1)
#define FLAG_IIC10_REVERSE2                 			(flag_IIC10_CTL.bit2)
#define FLAG_IIC10_REVERSE3                      		(flag_IIC10_CTL.bit3)
#define FLAG_IIC10_REVERSE4                      		(flag_IIC10_CTL.bit4)
#define FLAG_IIC10_REVERSE5                            	(flag_IIC10_CTL.bit5)
#define FLAG_IIC10_REVERSE6                          	(flag_IIC10_CTL.bit6)
#define FLAG_IIC10_REVERSE7                           	(flag_IIC10_CTL.bit7)

/*_____ D E F I N I T I O N S ______________________________________________*/

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
// extern void R_Config_IIC10_StopCondition (void);
// extern void R_Config_IIC10_Master_Send (uint8_t adr, uint8_t * const tx_buf, uint16_t tx_num);
// extern void R_Config_IIC10_Master_Receive (uint8_t adr, uint8_t * const rx_buf, uint16_t rx_num);


// for driver level
MD_STATUS drv_IIC10_read(unsigned char adr, unsigned char * const rx_buf, unsigned short rx_num)
{
	MD_STATUS ret = MD_OK;

	R_Config_IIC10_Master_Receive(adr, rx_buf, rx_num);

    return ret;
}


MD_STATUS drv_IIC10_write(unsigned char adr, unsigned char * const tx_buf, unsigned short tx_num)
{
	MD_STATUS ret = MD_OK;

    R_Config_IIC10_Master_Send(adr, tx_buf , tx_num);

    return ret;
}

void drv_IIC10_callback_error(unsigned char err)
{
    // R_Config_IIC10_StopCondition();
    printf("I2C MD_STATUS:0x%02X\r\n",err);
}


// for application

void R_Config_IIC10_StopCondition_II(void)
{
    volatile uint8_t w_count;

    ST1 |= _0001_SAU_CH0_STOP_TRG_ON;    /* disable IIC10 */
    SOE1 &= (uint16_t)~_0001_SAU_CH0_OUTPUT_ENABLE;    /* disable IIC10 output */
    // SO1 &= (uint16_t)~_0001_SAU_CH0_DATA_OUTPUT_1;    /* clear IIC10 SDA */
    SO1 |= _0100_SAU_CH0_CLOCK_OUTPUT_1;

    /* Set delay to secure a hold time after SDA output low. The delay time depend on slave device.
       Here set 5us as default base on current clock */
    for (w_count = 0U; w_count <= IIC10_WAITTIME; w_count++ )
    {
        NOP();
    }

    //SO1 |= _0100_SAU_CH0_CLOCK_OUTPUT_1;    /* set IIC10 SCL */
    SO1 &= (uint16_t)~_0001_SAU_CH0_DATA_OUTPUT_1;    /* clear IIC10 SDA */
    
    /* Set delay to secure a hold time after SCL output low. The delay time depend on slave device.
       Here set 5us as default base on current clock */
    for (w_count = 0U; w_count <= IIC10_WAITTIME; w_count++ )
    {
        NOP();
    }

    //SO1 |= _0001_SAU_CH0_DATA_OUTPUT_1;    /* set IIC10 SDA */
    SO1 &= (uint16_t)~_0100_SAU_CH0_CLOCK_OUTPUT_1;    /* clear IIC10 SCL */
    
    /* Set delay to secure a hold time after SDA output low. The delay time depend on slave device.
       Here set 5us as default base on current clock */
    for (w_count = 0U; w_count <= IIC10_WAITTIME; w_count++ )
    {
        NOP();
    }

    SOE1 |= _0001_SAU_CH0_OUTPUT_ENABLE;    /* enable IIC10 output */
    SS1 |= _0001_SAU_CH0_START_TRG_ON;      /* enable IIC10 */
    
}

void IIC10_sendend_condition(void)
{    
    #if defined (ENABLE_IIC10_WR_NO_STOP_RD)
    if (IIC10_Master_flag.enable_no_stop)
    {   
        // write > no stop > read
        R_Config_IIC10_StopCondition_II();
    }
    else 
    {   
        // regular write > stop > read
        R_Config_IIC10_StopCondition();
    }
    IIC10_Master_flag.enable_no_stop = 0;
    #else
    R_Config_IIC10_StopCondition();
    #endif
}

MD_STATUS IIC10_read(unsigned char device_addr,unsigned char reg_addr,unsigned char* rx_xfer_data,unsigned short rx_num)
{
	MD_STATUS ret = MD_OK;
    unsigned char tmp = 0;	
	unsigned int u32IICA0TimeOutCnt = 0;

    tmp = reg_addr;

    //Make sure bus is ready for xfer
	
	IIC10_Master_flag.tx_end = 1;

    #if defined (ENABLE_IIC10_WR_NO_STOP_RD)
    IIC10_Master_flag.enable_no_stop = 1;
    #endif

	ret = drv_IIC10_write(device_addr, &tmp, 1);
	if (ret != MD_OK)
    {
        printf("[I2C read error1]0x%02X\r\n" , ret);
		return ret;
	} 
	
	u32IICA0TimeOutCnt = IIC10_TIMEOUT_LIMIT;
	while ( IIC10_Master_flag.tx_end )
    {
        u32IICA0TimeOutCnt--;
        if(u32IICA0TimeOutCnt == 0)
        {         
	        IIC10_Master_flag.tx_end = 0;   
            #if defined (ENABLE_IIC10_WR_NO_STOP_RD)
            IIC10_Master_flag.enable_no_stop = 0;
            #endif         
            R_Config_IIC10_StopCondition();
            printf("%s bus busy*(xfer is complete),dev addr:0x%02X,reg addr:0x%02X\r\n",__func__,device_addr,reg_addr);   
            return MD_BUSY2;
        }
    } 	//Wait until the xfer is complete
    
	IIC10_Master_flag.rx_end = 1;
	ret = drv_IIC10_read(device_addr, rx_xfer_data, rx_num);
	
	u32IICA0TimeOutCnt = IIC10_TIMEOUT_LIMIT;
	while ( IIC10_Master_flag.rx_end )
    {
        u32IICA0TimeOutCnt--;
        if(u32IICA0TimeOutCnt == 0)
        {
	        IIC10_Master_flag.rx_end = 0;
            #if defined (ENABLE_IIC10_WR_NO_STOP_RD)
            IIC10_Master_flag.enable_no_stop = 0;
            #endif         
            R_Config_IIC10_StopCondition();
            printf("%s bus busy**(xfer is complete),dev addr:0x%02X,reg addr:0x%02X\r\n",__func__,device_addr,reg_addr);
            return MD_BUSY2;
        }
    } 	//Wait until the xfer is complete
	if (ret != MD_OK)
    {
        printf("[I2C read error2]0x%02X\r\n" , ret);
		return ret;
	} 

    // delay_ms(5);

    return ret;
}

MD_STATUS IIC10_write(unsigned char device_addr,unsigned char reg_addr,unsigned char* tx_xfer_data,unsigned short tx_num)
{
	MD_STATUS ret = MD_OK;
    unsigned char i = 0;
    unsigned char buffer[128] = {0};	
	unsigned int u32IICA0TimeOutCnt = 0;

    if (tx_num > 128)
    {
        printf("[I2C_write]tx_num over range(%2d)\r\n",tx_num);

        return MD_ARGERROR;
    }

    buffer[0] = reg_addr;
    for ( i = 0 ; i < tx_num ; i++ )
    {
        buffer[i+1] = tx_xfer_data[i];
    }

    //Make sure bus is ready for xfer
	
	IIC10_Master_flag.tx_end = 1;
	ret = drv_IIC10_write(device_addr, buffer , tx_num + 1);   // reg byte + data byte , at least 2 bytes
	if (ret != MD_OK)
    {
        printf("[I2C write error1]0x%02X\r\n" , ret);
		return ret;
	}
	
    #if 1
	u32IICA0TimeOutCnt = IIC10_TIMEOUT_LIMIT;
	while ( IIC10_Master_flag.tx_end )
    {
        u32IICA0TimeOutCnt--;
        if(u32IICA0TimeOutCnt == 0)
        {
            IIC10_Master_flag.tx_end = 0;            
            R_Config_IIC10_StopCondition();
            printf("%s bus busy(xfer is complete),dev addr:0x%02X,reg addr:0x%02X\r\n",__func__,device_addr,reg_addr);
            return MD_BUSY2;
        }
    } 	//Wait until the xfer is complete
    #endif

    // delay_ms(5);

    return ret;
}

void IIC10_scan_slave(void)
{
	MD_STATUS ret = MD_OK;
    unsigned char reg_addr = 0x31;
    unsigned char device_addr_8bit = I2C_SLAVE_ADDR_8BIT;
    unsigned char tmp = 0;
    unsigned char i = 0;

    printf_tiny("\r\n\r\n%s START\r\n\r\n",__func__);
    // for (i = 0 ; i < 5 ; i++)    //0xFF
    // {
    //     // device_addr_8bit = i;
    //     ret = IIC10_read(device_addr_8bit,reg_addr, (unsigned char*) &tmp,1);
    //     if (ret != MD_OK)
    //     {
    //         printf_tiny("[IIC10 scan error]0x%02X:0x%02X\r\n\r\n",device_addr_8bit,ret);
    //     }
    //     else
    //     {
    //         printf_tiny("[IIC10 scan address!]0x%02X:0x%02X\r\n\r\n",device_addr_8bit,ret);
    //     }
    // }
    // printf_tiny("\r\n\r\n%s END\r\n",__func__);

}
