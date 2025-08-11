/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>

#include "r_smc_entry.h"
#include "platform.h"

#include "misc_config.h"
#include "custom_func.h"

#include "IIC10_simple_i2c_driver.h"
#include "IIC11_simple_i2c_driver.h"
#include "IICA0_slave_driver.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

volatile struct flag_32bit flag_PROJ_CTL;
#define FLAG_PROJ_TIMER_PERIOD_1000MS                 	(flag_PROJ_CTL.bit0)
#define FLAG_PROJ_TRIG_BTN1                       	    (flag_PROJ_CTL.bit1)
#define FLAG_PROJ_TRIG_BTN2                 	        (flag_PROJ_CTL.bit2)
#define FLAG_PROJ_REVERSE3                    		    (flag_PROJ_CTL.bit3)
#define FLAG_PROJ_REVERSE4                              (flag_PROJ_CTL.bit4)
#define FLAG_PROJ_REVERSE5                              (flag_PROJ_CTL.bit5)
#define FLAG_PROJ_REVERSE6                              (flag_PROJ_CTL.bit6)
#define FLAG_PROJ_REVERSE7                              (flag_PROJ_CTL.bit7)


#define FLAG_PROJ_TRIG_1                                (flag_PROJ_CTL.bit8)
#define FLAG_PROJ_TRIG_2                                (flag_PROJ_CTL.bit9)
#define FLAG_PROJ_TRIG_3                                (flag_PROJ_CTL.bit10)
#define FLAG_PROJ_TRIG_4                                (flag_PROJ_CTL.bit11)
#define FLAG_PROJ_TRIG_5                                (flag_PROJ_CTL.bit12)
#define FLAG_PROJ_TRIG_6                                (flag_PROJ_CTL.bit13)
#define FLAG_PROJ_REVERSE14                             (flag_PROJ_CTL.bit14)
#define FLAG_PROJ_REVERSE15                             (flag_PROJ_CTL.bit15)

/*_____ D E F I N I T I O N S ______________________________________________*/

volatile unsigned long counter_tick = 0;
volatile unsigned long btn_counter_tick = 0;

#define BTN_PRESSED_LONG                                (2500)

volatile I2C_FLAG_MANAGER_T IICA0_Slave_flag = 
{
    .rx_end = 0,
    .tx_end = 0,
    // .enable_no_stop = 0,
};
volatile I2C_FLAG_MANAGER_T IIC10_Master_flag = 
{
    .rx_end = 0,
    .tx_end = 0,
    .enable_no_stop = 0,
};
volatile I2C_FLAG_MANAGER_T IIC11_Master_flag = 
{
    .rx_end = 0,
    .tx_end = 0,
    .enable_no_stop = 0,
};

unsigned char g_slave_read_value[256] = {0}; //buffer I2C read 
// unsigned char g_slave_write_value[256] = {0};//buffer I2C write


/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

unsigned long btn_get_tick(void)
{
	return (btn_counter_tick);
}

void btn_set_tick(unsigned long t)
{
	btn_counter_tick = t;
}

void btn_tick_counter(void)
{
	btn_counter_tick++;
    if (btn_get_tick() >= 60000)
    {
        btn_set_tick(0);
    }
}

unsigned long get_tick(void)
{
	return (counter_tick);
}

void set_tick(unsigned long t)
{
	counter_tick = t;
}

void tick_counter(void)
{
	counter_tick++;
    if (get_tick() >= 60000)
    {
        set_tick(0);
    }
}

void delay_ms(unsigned long ms)
{
	#if 1
    unsigned long tickstart = get_tick();
    unsigned long wait = ms;
	unsigned long tmp = 0;
	
    while (1)
    {
		if (get_tick() > tickstart)	// tickstart = 59000 , tick_counter = 60000
		{
			tmp = get_tick() - tickstart;
		}
		else // tickstart = 59000 , tick_counter = 2048
		{
			tmp = 60000 -  tickstart + get_tick();
		}		
		
		if (tmp > wait)
			break;
    }
	
	#else
	TIMER_Delay(TIMER0, 1000*ms);
	#endif
}

void IICA0_slave_process(void)    // IICA0
{
    /*
        WR : BYTE1 BYTE2
        0x71 , 2 bytes
        W)0x71 , 0x04 , 0x03 

        WR : BYTE1 BYTE2 BYTE3
        W)0x71 , 0x02 , 0x08 , 0x00 

        WR : BYTE1 BYTE2 ... BYTESn
        0x71 , multi bytes

        RD : 
        W 0x71 , 1 byte
        R 0x71 , multi bytes
        ex : 
        W)0x71 , 0x00 , R)0x71 , 0x00 , 3 BYTES 0x00 0x04 0x00
        W)0x71 , 0x01 , R)0x71 , 0x01 , 2 BYTES 0x3F 0x0A
        W)0x71 , 0x30 , R)0x71 , 0x30 , 1 BYTES 0x00
        W)0x71 , 0xF7 , R)0x71 , 0xF7 , 4 BYTES 0x04 0x01 0xE2 0xDF
        W)0x71 , 0xF8 , R)0x71 , 0xF8 , 3 BYTES 0x03 0x00 0xFC
        W)0x71 , 0xF9 , R)0x71 , 0xF9 , 3 BYTES 0x03 0x00 0xFD
    */

}

void IIC11_master_process(void)
{
    unsigned char buffer[16] = {0};
    unsigned short i = 0;
    unsigned short len = 0;
    static unsigned char count = 0;

    if (FLAG_PROJ_TRIG_6)
    {
        FLAG_PROJ_TRIG_6 = 0;
        len = 5;
        reset_buffer(g_slave_read_value,0x00,sizeof(g_slave_read_value));
        IIC11_read(I2C_SLAVE_ADDR_8BIT,I2C_CMD_REG_0xF7,g_slave_read_value,len);
        dump_buffer8(g_slave_read_value,len);        
    }

    if (FLAG_PROJ_TRIG_5)
    {
        FLAG_PROJ_TRIG_5 = 0;        
        len = 4;
        reset_buffer(g_slave_read_value,0x00,sizeof(g_slave_read_value));
        IIC11_read(I2C_SLAVE_ADDR_8BIT,I2C_CMD_REG_0xF8,g_slave_read_value,len);
        dump_buffer8(g_slave_read_value,len);
    }

    if (FLAG_PROJ_TRIG_4)
    {
        FLAG_PROJ_TRIG_4 = 0;        
        len = 4;
        reset_buffer(g_slave_read_value,0x00,sizeof(g_slave_read_value));
        IIC11_read(I2C_SLAVE_ADDR_8BIT,I2C_CMD_REG_0xF9,g_slave_read_value,len);
        dump_buffer8(g_slave_read_value,len);
    }

    if (FLAG_PROJ_TRIG_3)
    {
        FLAG_PROJ_TRIG_3 = 0;
        len = 16;
        for ( i = 0 ; i < len ; i++)
        {
            buffer[i] = i + 0x10;
        }
        buffer[15] = count;
        IIC11_write(I2C_SLAVE_ADDR_8BIT,I2C_CMD_REG_0x03,buffer,len);
        count++;
    }
    
    if (FLAG_PROJ_TRIG_2)
    {
        FLAG_PROJ_TRIG_2 = 0;
        len = 2;
        buffer[0] = 0x08;
        buffer[1] = count;
        IIC11_write(I2C_SLAVE_ADDR_8BIT,I2C_CMD_REG_0x02,buffer,len);
        count++;
    }

    if (FLAG_PROJ_TRIG_1)
    {
        FLAG_PROJ_TRIG_1 = 0;
        len = 1;
        buffer[0] = 0x03;
        IIC11_write(I2C_SLAVE_ADDR_8BIT,I2C_CMD_REG_0x04,buffer,len);
    }

}

void Timer_1ms_IRQ(void)
{
    tick_counter();

    if ((get_tick() % 1000) == 0)
    {
        FLAG_PROJ_TIMER_PERIOD_1000MS = 1;
    }

    if ((get_tick() % 50) == 0)
    {

    }	

    Button_Process_long_counter();
}


/*
    F24 target board
    LED1 connected to P66, LED2 connected to P67
*/
void LED_Toggle(void)
{
    // PIN_WRITE(6,6) = ~PIN_READ(6,6);
    // PIN_WRITE(6,7) = ~PIN_READ(6,7);
    P6_bit.no6 = ~P6_bit.no6;
    P6_bit.no7 = ~P6_bit.no7;
}

void loop(void)
{
	// static unsigned long LOG1 = 0;

    if (FLAG_PROJ_TIMER_PERIOD_1000MS)
    {
        FLAG_PROJ_TIMER_PERIOD_1000MS = 0;

        // printf_tiny("log(timer):%4d\r\n",LOG1++);
        LED_Toggle();             
    }

    Button_Process_in_polling();
    
    IIC11_master_process();

    IICA0_slave_process();
}


// F24 EVB , P137/INTP0 , set both edge 
void Button_Process_long_counter(void)
{
    if (FLAG_PROJ_TRIG_BTN2)
    {
        btn_tick_counter();
    }
    else
    {
        btn_set_tick(0);
    }
}

void Button_Process_in_polling(void)
{
    static unsigned char cnt = 0;

    if (FLAG_PROJ_TRIG_BTN1)
    {
        FLAG_PROJ_TRIG_BTN1 = 0;
        printf_tiny("BTN pressed(%d)\r\n",cnt);

        if (cnt == 0)   //set both edge  , BTN pressed
        {
            FLAG_PROJ_TRIG_BTN2 = 1;
        }
        else if (cnt == 1)  //set both edge  , BTN released
        {
            FLAG_PROJ_TRIG_BTN2 = 0;
        }

        cnt = (cnt >= 1) ? (0) : (cnt+1) ;
    }

    if ((FLAG_PROJ_TRIG_BTN2 == 1) && 
        (btn_get_tick() > BTN_PRESSED_LONG))
    {         
        printf_tiny("BTN pressed LONG\r\n");
        btn_set_tick(0);
        FLAG_PROJ_TRIG_BTN2 = 0;
    }
}

// F24 EVB , P137/INTP0
void Button_Process_in_IRQ(void)    
{
    FLAG_PROJ_TRIG_BTN1 = 1;
}

void UARTx_Process(unsigned char rxbuf)
{    
    if (rxbuf > 0x7F)
    {
        printf_tiny("invalid command\r\n");
    }
    else
    {
        printf_tiny("press:%c(0x%02X)\r\n" , rxbuf,rxbuf);   // %c :  C99 libraries.
        switch(rxbuf)
        {
            case '1':
                FLAG_PROJ_TRIG_1 = 1;
                break;
            case '2':
                FLAG_PROJ_TRIG_2 = 1;
                break;
            case '3':
                FLAG_PROJ_TRIG_3 = 1;
                break;
            case '4':
                FLAG_PROJ_TRIG_4 = 1;
                break;
            case '5':
                FLAG_PROJ_TRIG_5 = 1;
                break;
            case '6':
                FLAG_PROJ_TRIG_6 = 1;
                break;

            case 'X':
            case 'x':
                RL78_soft_reset(7);
                break;
            case 'Z':
            case 'z':
                RL78_soft_reset(1);
                break;
        }
    }
}

/*
    Reset Control Flag Register (RESF) 
    BIT7 : TRAP
    BIT6 : 0
    BIT5 : 0
    BIT4 : WDCLRF
    BIT3 : 0
    BIT2 : 0
    BIT1 : IAWRF
    BIT0 : LVIRF
*/
void check_reset_source(void)
{
    /*
        Internal reset request by execution of illegal instruction
        0  Internal reset request is not generated, or the RESF register is cleared. 
        1  Internal reset request is generated. 
    */
    uint8_t src = RESF;
    printf_tiny("Reset Source <0x%08X>\r\n", src);

    #if 1   //DEBUG , list reset source
    if (src & BIT0)
    {
        printf_tiny("0)voltage detector (LVD)\r\n");       
    }
    if (src & BIT1)
    {
        printf_tiny("1)illegal-memory access\r\n");       
    }
    if (src & BIT2)
    {
        printf_tiny("2)EMPTY\r\n");       
    }
    if (src & BIT3)
    {
        printf_tiny("3)EMPTY\r\n");       
    }
    if (src & BIT4)
    {
        printf_tiny("4)watchdog timer (WDT) or clock monitor\r\n");       
    }
    if (src & BIT5)
    {
        printf_tiny("5)EMPTY\r\n");       
    }
    if (src & BIT6)
    {
        printf_tiny("6)EMPTY\r\n");       
    }
    if (src & BIT7)
    {
        printf_tiny("7)execution of illegal instruction\r\n");       
    }
    #endif

}

/*
    7:Internal reset by execution of illegal instruction
    1:Internal reset by illegal-memory access
*/
//perform sofware reset
void _reset_by_illegal_instruction(void)
{
    static const unsigned char illegal_Instruction = 0xFF;
    void (*dummy) (void) = (void (*)(void))&illegal_Instruction;
    dummy();
}
void _reset_by_illegal_memory_access(void)
{
    #if 1
    const unsigned char ILLEGAL_ACCESS_ON = 0x80;
    IAWCTL |= ILLEGAL_ACCESS_ON;            // switch IAWEN on (default off)
    *(__far volatile char *)0x00000 = 0x00; //write illegal address 0x00000(RESET VECTOR)
    #else
    signed char __far* a;                   // Create a far-Pointer
    IAWCTL |= _80_CGC_ILLEGAL_ACCESS_ON;    // switch IAWEN on (default off)
    a = (signed char __far*) 0x0000;        // Point to 0x000000 (FLASH-ROM area)
    *a = 0;
    #endif
}

void RL78_soft_reset(unsigned char flag)
{
    switch(flag)
    {
        case 7: // do not use under debug mode
            _reset_by_illegal_instruction();        
            break;
        case 1:
            _reset_by_illegal_memory_access();
            break;
    }
}

// retarget printf
int __far putchar(int c)
{
    // F24 , UART0
    STMK0 = 1U;    /* disable INTST0 interrupt */
    SDR00L = (unsigned char)c;
    while(STIF0 == 0)
    {

    }
    STIF0 = 0U;    /* clear INTST0 interrupt flag */
    return c;
}

void hardware_init(void)
{
    // const unsigned char indicator[] = "hardware_init";
    BSP_EI();
    R_Config_UART0_Start();         // UART , P15 , P16
    R_Config_TAU0_1_Start();        // TIMER
    R_Config_INTC_INTP0_Start();    // BUTTON , P137 
    
    //I2C slave - IICA0 : P62/SCL , P63/SDA
    IICA0_slave_Init();  

    //I2C master - SCL10 : P10 , SDA10 : P11 , conflict with F24 EVB CAN-FD 
    //I2C master - SCL11 : P71 , SDA11 : P70
    IIC10_scan_slave();
    IIC11_scan_slave();


    // check_reset_source();
    printf_tiny("\r\n\r\n%s finish\r\n\r\n",__func__);
}
