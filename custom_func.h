#ifndef __CUSTOM_FUNC_H__
#define __CUSTOM_FUNC_H__

/*_____ I N C L U D E S ____________________________________________________*/

/*_____ D E C L A R A T I O N S ____________________________________________*/
#define bool  _Bool

#ifndef TRUE
#define TRUE  (1)
#endif

#ifndef true
#define true  (1)
#endif

#ifndef FALSE
#define FALSE  (0)
#endif

#ifndef false
#define false  (0)
#endif

#ifndef BIT0
#define BIT0     (0x00000001UL)       ///< Bit 0 mask of an 32 bit integer
#endif

#ifndef BIT1
#define BIT1     (0x00000002UL)       ///< Bit 1 mask of an 32 bit integer
#endif

#ifndef BIT2
#define BIT2     (0x00000004UL)       ///< Bit 2 mask of an 32 bit integer
#endif

#ifndef BIT3
#define BIT3     (0x00000008UL)       ///< Bit 3 mask of an 32 bit integer
#endif

#ifndef BIT4
#define BIT4     (0x00000010UL)       ///< Bit 4 mask of an 32 bit integer
#endif

#ifndef BIT5
#define BIT5     (0x00000020UL)       ///< Bit 5 mask of an 32 bit integer
#endif

#ifndef BIT6
#define BIT6     (0x00000040UL)       ///< Bit 6 mask of an 32 bit integer
#endif

#ifndef BIT7
#define BIT7     (0x00000080UL)       ///< Bit 7 mask of an 32 bit integer
#endif

/*_____ D E F I N I T I O N S ______________________________________________*/

/*  
	template
	typedef struct _peripheral_manager_t
	{
		uint8_t u8Cmd;
		uint8_t au8Buf[33];
		uint8_t u8RecCnt;
		uint8_t bByPass;
		uint16_t* pu16Far;
	}PERIPHERAL_MANAGER_T;

	volatile PERIPHERAL_MANAGER_T g_PeripheralManager = 
	{
		.u8Cmd = 0,
		.au8Buf = {0},		//.au8Buf = {100U, 200U},
		.u8RecCnt = 0,
		.bByPass = FALSE,
		.pu16Far = NULL,	//.pu16Far = 0	
	};
	extern volatile PERIPHERAL_MANAGER_T g_PeripheralManager;
*/

typedef struct _i2c_flag_manager_t
{
	unsigned char rx_end;
	unsigned char tx_end;
	unsigned char enable_no_stop;
}I2C_FLAG_MANAGER_T;

#define I2C_SLAVE_ADDR_7BIT                                  (0x71)
#define I2C_SLAVE_ADDR_8BIT                                  (I2C_SLAVE_ADDR_7BIT<<1)

#define I2C_CMD_REG_0x00 (0x00)
#define I2C_CMD_REG_0x01 (0x01)
#define I2C_CMD_REG_0x02 (0x02)
#define I2C_CMD_REG_0x03 (0x03)
#define I2C_CMD_REG_0x04 (0x04)
#define I2C_CMD_REG_0x30 (0x30)
#define I2C_CMD_REG_0x50 (0x50)
#define I2C_CMD_REG_0x51 (0x51)
#define I2C_CMD_REG_0xF7 (0xF7)
#define I2C_CMD_REG_0xF8 (0xF8)
#define I2C_CMD_REG_0xF9 (0xF9)

extern volatile I2C_FLAG_MANAGER_T IICA0_Slave_flag;
extern volatile I2C_FLAG_MANAGER_T IIC10_Master_flag;
extern volatile I2C_FLAG_MANAGER_T IIC11_Master_flag;

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/


MD_STATUS IIC11_read(unsigned char device_addr,unsigned char reg_addr,unsigned char* rx_xfer_data,unsigned short rx_num);
MD_STATUS IIC11_write(unsigned char device_addr,unsigned char reg_addr,unsigned char* tx_xfer_data,unsigned short tx_num);
void IIC11_scan_slave(void);

MD_STATUS IIC10_read(unsigned char device_addr,unsigned char reg_addr,unsigned char* rx_xfer_data,unsigned short rx_num);
MD_STATUS IIC10_write(unsigned char device_addr,unsigned char reg_addr,unsigned char* tx_xfer_data,unsigned short tx_num);
void IIC10_scan_slave(void);

void IIC11_sendend_condition(void);
void IIC10_sendend_condition(void);
void r_Config_IICA0_custom_slave_handler(void);

void drv_IICA0_slave_callback_error(unsigned char err);
void drv_IIC10_callback_error(unsigned char err);

void Timer_1ms_IRQ(void);

void Button_Process_long_counter(void);
void Button_Process_in_polling(void);
void Button_Process_in_IRQ(void);
void UARTx_Process(unsigned char rxbuf);

void LED_Toggle(void);
void loop(void);

void check_reset_source(void);
void RL78_soft_reset(unsigned char flag);
void hardware_init(void);

#endif //__CUSTOM_FUNC_H__
