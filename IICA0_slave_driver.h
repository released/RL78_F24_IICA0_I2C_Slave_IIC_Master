#ifndef __IICA0_SLAVE_DRIVER_H__
#define __IICA0_SLAVE_DRIVER_H__

/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include "r_cg_macrodriver.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/

// #define DATA_LENGTH                                             (255U)  /* Tx/Rx data buffer length.        */

#define TX_BUFFER_LEN                                               (128)
#define RX_BUFFER_LEN                                               (128)


typedef enum
{
	SLAVE_RX = 0 ,
	SLAVE_TX , 	
}IICA_SLAVE_TypeDef;

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/


void IICA0_slave_Init(void);
void IICA0_slave_decode(unsigned char* target_decode_data);

#endif //__IICA0_SLAVE_DRIVER_H__
