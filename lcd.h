//***************************************************************************
//
// File Name : "lcd.h "
// Date : 03/26/2019
// Version : 1.0
// Target MCU : ATSAML21J18B
// Target Hardware : DOG LCD module
// Author : Riwaz Awasthi
// DESCRIPTION : Header file containing necessary declarations
//
//
// Warnings :
// Restrictions : none
// Algorithms : none
// References :
//
// Revision History : Initial version
//
//
//**************************************************************************

#include "saml21j18b.h"


#ifndef LCD_H_
#define LCD_H_

extern unsigned char* ARRAY_PORT_PINCFG0;
extern unsigned char* ARRAY_PORT_PMUX0;

extern char disp_buff_1[16];
extern char disp_buff_2[16];
extern char disp_buff_3[16];

extern void delay_30us();
extern void delay_40ms();
extern void init_lcd_dog();
extern void init_spi_lcd();
extern void lcd_spi_transmit_cmd(uint8_t output_val);
extern void lcd_spi_transmit_data(uint8_t output_val);
extern void update_lcd_dog();



#endif /* LCD_H_ */