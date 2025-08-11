# RL78_F24_IICA0_I2C_Slave_IIC_Master
RL78_F24_IICA0_I2C_Slave_IIC_Master

update @ 2025/08/11

1. base on EVM : RL78 F24 EVB , initial 

- UART : P15 , P16

- I2C slave - IICA0 : P62/SCL , P63/SDA

- I2C master - SCL11 : P71 , SDA11 : P70

2. Use external I2C master and slave to test scenario

3. Scenario : I2C slave (IICA0)

- when ext. I2C master send RD command : 0x01 , I2C slave (IICA0) will return reg(0x01) , 0x3F , 0x0A to I2C master 

![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/IICA0_Slave_return_0x01.jpg)


- when ext. I2C master send RD command : 0xF7 , I2C slave (IICA0) will return reg(0xF7) , 0x04 , 0x01 , 0xE2 , 0xDF to I2C master 

![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/IICA0_Slave_return_0xF7.jpg)


- when ext. I2C master send RD command : 0xF8 , I2C slave (IICA0) will return reg(0xF8) , 0x03, 0x00 , 0xFC to I2C master 

![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/IICA0_Slave_return_0xF8.jpg)


check below function 
```c

void IICA0_slave_return(void)
{
    unsigned char counter = 0x00;  
    
    g_tx_data[counter++] = g_decode_reg;    //return first byte as register
                   
    switch(g_decode_reg)
	...
```

- when ext. I2C master send WR command : 0x50 , I2C slave (IICA0) will print log message when receive finish

![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/IICA0_Slave_receive_5BYTES.jpg)


![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/IICA0_Slave_receive_17BYTES.jpg)


check below function 
```c
void IICA0_slave_decode(unsigned char* target_decode_data)
{
    unsigned short len = g_iica0_rx_cnt;

    printf("\r\n----START(%d)----\r\n",g_iica0_rx_cnt);
    
    dump_buffer8(target_decode_data, len);

    printf("----EOL----\r\n");
}
```

4. Scenario : I2C master (SCL11) , check : IIC11_master_process 

- press digit 1 , send WR command , reg 0x04 , data 0x03 to ext. I2C slave

![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/LA_IIC11_digit_1.jpg)


- press digit 2 , send WR command , reg 0x02 , data 0x08 , counter , to ext. I2C slave

![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/LA_IIC11_digit_2.jpg)


- press digit 3 , send WR command , reg 0x03 , data0~data15 , to ext. I2C slave

![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/LA_IIC11_digit_3.jpg)


- use define : ENABLE_IIC11_WR_NO_STOP_RD , to control , when read RD command , to ext. I2C slave 

when WRITE > NO STOP (repeat start) > READ

![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/LA_IIC11_digit_4_no_stop.jpg)


![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/LA_IIC11_digit_5_no_stop.jpg)


![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/LA_IIC11_digit_6_no_stop.jpg)


when WRITE > STOP > START > READ

![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/LA_IIC11_digit_4_with_stop.jpg)


![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/LA_IIC11_digit_5_with_stop.jpg)


![image](https://github.com/released/RL78_F24_IICA0_I2C_Slave_IIC_Master/blob/main/LA_IIC11_digit_6_with_stop.jpg)



check below function 
```c

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
	...
	
```

