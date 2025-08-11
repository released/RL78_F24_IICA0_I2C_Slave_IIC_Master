/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>

#include "r_cg_iica.h"

#include "retarget.h"
#include "misc_config.h"
#include "custom_func.h"

#include "IICA0_slave_driver.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

struct flag_8bit flag_IICA0_SLAVE_CTL;
#define FLAG_IICA0_SLAVE_I2C_REVERSE0                 		   	(flag_IICA0_SLAVE_CTL.bit0)
#define FLAG_IICA0_SLAVE_I2C_REVERSE1                       	(flag_IICA0_SLAVE_CTL.bit1)
#define FLAG_IICA0_SLAVE_I2C_REG_WR     				        (flag_IICA0_SLAVE_CTL.bit2)
#define FLAG_IICA0_SLAVE_I2C_REG_RD                		        (flag_IICA0_SLAVE_CTL.bit3)
#define FLAG_IICA0_SLAVE_I2C_START_RCV                       	(flag_IICA0_SLAVE_CTL.bit4)
#define FLAG_IICA0_SLAVE_I2C_MASTER_RDY_TO_RCV                  (flag_IICA0_SLAVE_CTL.bit5)
#define FLAG_IICA0_SLAVE_I2C_REVERSE6                           (flag_IICA0_SLAVE_CTL.bit6)
#define FLAG_IICA0_SLAVE_I2C_REVERSE7                           (flag_IICA0_SLAVE_CTL.bit7)

/*_____ D E F I N I T I O N S ______________________________________________*/

unsigned char g_tx_data[TX_BUFFER_LEN] = {0};
unsigned char g_rx_data[RX_BUFFER_LEN] = {0};
unsigned short g_iica0_cal_rx_len = 0;

unsigned char g_decode_reg = 0;
/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
extern volatile uint8_t   g_iica0_slave_status_flag;            /* iica0 slave flag */
extern volatile uint8_t * gp_iica0_rx_address;                  /* iica0 receive buffer address */
extern volatile unsigned short  g_iica0_rx_cnt;                 /* iica0 receive data length */
extern volatile unsigned short  g_iica0_rx_len;                 /* iica0 receive data count */
extern volatile uint8_t * gp_iica0_tx_address;                  /* iica0 send buffer address */
extern volatile unsigned short  g_iica0_tx_cnt;                 /* iica0 send data count */

// for driver level
// MD_STATUS drv_IICA0_slave_read(unsigned char * const rx_buf, unsigned short rx_num)
// {
// 	MD_STATUS ret = MD_OK;

// 	R_IICA0_Slave_Receive(rx_buf, rx_num);

// 	if (ret != MD_OK)
//     {
//         printf("[%s error]0x%02X\r\n" , __func__ , ret);
// 		return ret;
// 	} 
//     return ret;
// }

// MD_STATUS drv_IICA0_slave_write(unsigned char * const tx_buf, unsigned short tx_num)
// {
// 	MD_STATUS ret = MD_OK;

//     R_IICA0_Slave_Send(tx_buf , tx_num );

// 	if (ret != MD_OK)
//     {
//         printf("[%s error]0x%02X\r\n" , __func__ , ret);
// 		return ret;
// 	} 

//     return ret;
// }

bool drv_Is_IICA0_slave_bus_busy(void)
{
    return IICBSY0;
}

void drv_IICA0_slave_callback_error(unsigned char err)
{
    printf("IICA0 MD_STATUS:0x%02X\r\n",err);
}


// for application
void IICA0_slave_Init(void)
{    
	g_iica0_rx_len = 0xFF;                  /* iica0 receive data length */
	g_iica0_rx_cnt = 0;                     /* iica0 receive data count */
    g_iica0_tx_cnt = 0xFF;                  /* iica0 send data count */

    FLAG_IICA0_SLAVE_I2C_REG_RD = 0;
    FLAG_IICA0_SLAVE_I2C_REG_WR = 0;
    // FLAG_IICA0_SLAVE_I2C_MASTER_RDY_TO_RCV = 0;
}

void IICA0_slave_return(void)
{
    unsigned char counter = 0x00;  
    
    g_tx_data[counter++] = g_decode_reg;    //return first byte as register
                   
    switch(g_decode_reg)
    {
        case I2C_CMD_REG_0x01:
            g_iica0_tx_cnt = 1+2;
            g_tx_data[counter++] = 0x3F;
            g_tx_data[counter++] = 0x0A;
            break;

        case I2C_CMD_REG_0xF7:
            g_iica0_tx_cnt = 1+4;
            g_tx_data[counter++] = 0x04;
            g_tx_data[counter++] = 0x01;
            g_tx_data[counter++] = 0xE2;
            g_tx_data[counter++] = 0xDF;
            break;

        case I2C_CMD_REG_0xF8:
            g_iica0_tx_cnt = 1+3;
            g_tx_data[counter++] = 0x03;
            g_tx_data[counter++] = 0x00;
            g_tx_data[counter++] = 0xFC;
            break;
    }

    gp_iica0_tx_address = (uint8_t*) g_tx_data; // assign addr to drvier
}

void IICA0_slave_decode(unsigned char* target_decode_data)
{
    unsigned short len = g_iica0_rx_cnt;

    printf("\r\n----START(%d)----\r\n",g_iica0_rx_cnt);
    
    dump_buffer8(target_decode_data, len);

    printf("----EOL----\r\n");
}

void IICA0_slave_data_process(IICA_SLAVE_TypeDef s)
{
    // slvae RX , keep receive data
    if ((s == SLAVE_RX) && FLAG_IICA0_SLAVE_I2C_REG_WR)
    {
        #if 1
        IICA0_slave_decode(g_rx_data);
        #else
        if (IICA0_Slave_flag.rx_end)
        {
            IICA0_slave_decode(g_rx_data);
            
            IICA0_Slave_flag.rx_end = 0U;
        }
        #endif
    }

    // slave TX , receive reg and prepare TX data
    if ((s == SLAVE_TX) && FLAG_IICA0_SLAVE_I2C_REG_RD)
    {       
        IICA0_slave_return();
    }
}

void IICA0_slave_check_reg(unsigned char d)
{                    
    switch(d)
    {
        case I2C_CMD_REG_0x01:
        case I2C_CMD_REG_0xF7:
        case I2C_CMD_REG_0xF8:
            FLAG_IICA0_SLAVE_I2C_REG_RD = 1;
            g_decode_reg = d;
            break;
        case I2C_CMD_REG_0x50:
        case I2C_CMD_REG_0x51:
            FLAG_IICA0_SLAVE_I2C_REG_WR = 1;
            break;
    }
}

void r_Config_IICA0_custom_slave_handler(void)
{
    /* Control for stop condition */
    if (1U == SPD0)
    {
        /* Get stop condition */
        SPIE0 = 0U;                                     //Enable/disable generation of interrupt request when stop condition is detected 
        // WTIM0 = 1U;                                  //After input of nine clocks, the clock is set to low level and wait is set for master device.
        // WREL0 = 1U;                                  //release I2C bus wait
              
        IICA0_slave_data_process(SLAVE_RX); 
        IICA0_slave_Init();
        g_iica0_slave_status_flag = 1U;        
        printf("STO\r\n");
    }
    else
    {
        //for I2C repeated start (restart)
        if (1U == STD0)                                 // start or restart condition
        {
            g_iica0_slave_status_flag = 0U;             //Reset slave status flag           
            // printf("REPEAT\r\n");            
        }

        if (0U == (g_iica0_slave_status_flag & _80_IICA_ADDRESS_COMPLETE))
        {

            //Address match , I2C slave transmit requested
            if (1U == COI0)                             //check selected as slave or not
            {
                //selected by slave address
                SPIE0 = 1U;                             //Enable/disable generation of interrupt request when stop condition is detected 
                g_iica0_slave_status_flag |= _80_IICA_ADDRESS_COMPLETE;

                // printf("ADDR(0x%02X)\r\n",g_iica0_slave_status_flag);

                //I2C slave transmit requested                                
                if (1U == TRC0)                             //check if transmit or not
                {
                    //data transmit timing
                    WTIM0 = 1U;         //After input of nine clocks, the clock is set to low level and wait is set for master device.

                    // printf("TX\r\n");

                    if (g_iica0_tx_cnt > 0U)           //check remaining transmit data
                    {                        
                        // printf("TX-1\r\n");
                        //start to transmit next data in buffer
                        IICA0 = *gp_iica0_tx_address;
                        gp_iica0_tx_address++;
                        g_iica0_tx_cnt--;
                    }
                    else
                    {
                        // printf("TX-2\r\n");     
                        IICA0_Slave_flag.tx_end = 1U;

                        //Send dummy data?
                        IICA0 = 0;

                        //release clock stretching
                        WREL0 = 1U;                 //release I2C bus wait
                    }
                }
                else    //I2C slave receive requested
                {
                    // receive data portion of Master Write Slave Read is started from here

                    FLAG_IICA0_SLAVE_I2C_START_RCV = 1;

                    g_iica0_rx_cnt = 0;
                    g_iica0_rx_len = 0xFF;

                    // receive data portion of Master Write Slave Read is started from here
                    // printf("RX(0x%02X)\r\n",g_iica0_slave_status_flag);

                    //data receive start
                    ACKE0 = 1U;//enable ACK response for receive data
                    WTIM0 = 0U;//After input of eight clocks, the clock is set to low level and wait is set for master device.
                    WREL0 = 1U;//start to receive
                }
            }
            else
            {
                // printf("!COI0\r\n");
                drv_IICA0_slave_callback_error(MD_ERROR);
            }
        }
        else
        {
            
            //data transfer timing , I2C slave transmit requested
            if (1U == TRC0)
            {
                // printf("RX111\r\n");

                //data transmit timing , ACK was not detected
                if ((0U == ACKD0) && (g_iica0_tx_cnt != 0U))    //(0U == ACKD0)
                {
                    //release clock stretching
                    WREL0 = 1U;         //release I2C bus wait

                    // printf("TX-3\r\n");     
                }
                else
                {
                    if (g_iica0_tx_cnt > 0U)
                    {
                        // printf("TX-4\r\n");   
                        //continue to transmit  
                        #if 1   // master ready to receive
                        // if (FLAG_IICA0_SLAVE_I2C_MASTER_RDY_TO_RCV)
                        {                  
                            IICA0 = *gp_iica0_tx_address;
                            gp_iica0_tx_address++;
                            g_iica0_tx_cnt--;
                        }

                        #else    
                        IICA0 = *gp_iica0_tx_address;
                        gp_iica0_tx_address++;
                        g_iica0_tx_cnt--;
                        #endif
                    }
                    else
                    {             
                        // printf("TX-5\r\n");  
                        IICA0_Slave_flag.tx_end = 1U;

                        // WREL0 = 1U;

                        //exit from communication mode
                        LREL0 = 1U;
                    }
                }
            }
            else    //I2C slave receive requested
            {
                // printf("RX222\r\n");
                if (g_iica0_rx_cnt < g_iica0_rx_len)
                {                    
                    g_rx_data[g_iica0_rx_cnt] = IICA0;

                    // first byte will consider as REG byte , prepare raw data
                    // printf("%2d,%3d,%02Xh\r\n" ,g_iica0_rx_cnt,g_iica0_rx_len,g_rx_data[g_iica0_rx_cnt]);
                    
                    IICA0_slave_check_reg(g_rx_data[0]);
                    IICA0_slave_data_process(SLAVE_TX); 

                    if (g_iica0_rx_cnt == (g_iica0_cal_rx_len-1) &&
                        FLAG_IICA0_SLAVE_I2C_START_RCV )
                    {
                        WTIM0 = 1U;                     //After input of nine clocks, the clock is set to low level and wait is set for master device.
                        WREL0 = 1U;                     //release I2C bus wait
                        IICA0_Slave_flag.rx_end = 1U;

                        FLAG_IICA0_SLAVE_I2C_START_RCV = 0;

                        // for (i = 0 ; i < g_iica0_cal_rx_len ; i++)
                        // {
                        //     // printf("meet len(%d):0x%02X\r\n" ,g_iica0_rx_cnt  ,g_rx_data[g_iica0_rx_cnt]);
                        //     printf("%2d,%02Xh," ,g_iica0_rx_cnt,g_rx_data[g_iica0_rx_cnt]);
                        // }
                        // printf("RDY\r\n");

                    }
                    else
                    {
                        //continue to receive next data
                        WREL0 = 1U;                     //release I2C bus wait

                        // printf("RDY1\r\n");
                    }    

                    g_iica0_rx_cnt++;

                }
                else
                {
                    WREL0 = 1U;
                    // printf("RDY2\r\n");
                }
            }
        }
    }
}

