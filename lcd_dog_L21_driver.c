//***************************************************************************
//
// File Name : "lcd_dog_L21_driver.c "
// Date : 03/26/2019
// Version : 1.0
// Target MCU : ATSAML21J18B
// Target Hardware : DOG LCD module
// Author : Riwaz Awasthi
// DESCRIPTION : Contains all the necessary functions to initialize
//               and update the LCD DOG module.
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





//#include "saml21j18b.h"
#include "lcd.h"
#define IOCONaddr_b0  0x0A    //address at reset, default 16-bit mode
#define IOCONaddr_b1  0x05
#define IODIRAaddr_b1 0x00
#define IODIRBaddr_b1 0x10
#define GPPUAaddr_b1  0x06
#define GPIOAaddr_b1  0x09
#define OLATBaddr_b1  0x1A
#define WRITE_opcode  0x40
#define READ_opcode   0x41

unsigned char* ARRAY_PORT_PINCFG0 = (unsigned char*)&REG_PORT_PINCFG0;
unsigned char* ARRAY_PORT_PMUX0 = (unsigned char*)&REG_PORT_PMUX0;

char disp_buff_1[16];
char disp_buff_2[16];
char disp_buff_3[16];

void delay_40ms(){
	for(int i=0; i<20000; i++){}
}
void delay_30us(){
	for(int i=0; i<5; i++){}
}


/*****************************************************
* Function : init_spi_lcd
* Author   : Riwaz Awasthi
*
* Description: Initializes  the ATSAML21J18B’s SERCOM1 to communicate
*              with the DOG module LCD display.
*
*              PA16  PAD0  MOSI
*              PA17  PAD1  SCK
*              PA18  PAD2  /SS       hardware controlled
*              PA19  PAD3  MISO
*              PB00        /RS
*              PB01        BLC
*****************************************************/
void init_spi_lcd() {
	
	//REG_MCLK_AHBMASK |= 0x00000004;	/* APBC bus clock enabled by default */
	//REG_MCLK_APBCMASK |= 0x00000002;	/* SERCOM1 APBC bus clock enabled by default */
	// Generic clock generator 0, enabled at reset @ 4MHz, is used for peripheral clock
	REG_GCLK_PCHCTRL19 = 0x00000040;	/* SERCOM1 core clock not enabled by default */

	ARRAY_PORT_PINCFG0[16] |= 1;    /* allow pmux to set PA16 pin configuration */
	ARRAY_PORT_PINCFG0[17] |= 1;    /* allow pmux to set PA17 pin configuration */
	ARRAY_PORT_PINCFG0[18] |= 1;    /* allow pmux to set PA18 pin configuration */
	ARRAY_PORT_PINCFG0[19] |= 1;    /* allow pmux to set PA19 pin configuration */
	ARRAY_PORT_PMUX0[8] = 0x22;     /* PA16 = MOSI, PA17 = SCK */
	ARRAY_PORT_PMUX0[9] = 0x22;     /* PA18 = SS,   PA19 = MISO */

	REG_SERCOM1_SPI_CTRLA = 1;              /* reset SERCOM1 */
	while (REG_SERCOM1_SPI_CTRLA & 1) {}    /* wait for reset to complete */
	REG_SERCOM1_SPI_CTRLA = 0x3030000C;     /* MISO-3, MOSI-0, SCK-1, SS-2, SPI master */
	REG_SERCOM1_SPI_CTRLB = 0x00002000;     /* Master SS, 8-bit */
	REG_SERCOM1_SPI_BAUD = 0;               /* SPI clock is 4MHz/2 = 2MzHz */
	REG_SERCOM1_SPI_CTRLA |= 2;             /* enable SERCOM1 */
	
	REG_PORT_DIRSET1 = 0x03;               //PB00, PB01 are configured as output pins
	REG_PORT_OUTSET1= 0x03;
}
/*****************************************************
* Function : lcd_spi_transmit_cmd
* Author   : Riwaz Awasthi
*
* Description: outputs a command passed in r16 via SPI port. Waits for data
;              to be written by SPI port before continuing.
*
*              PA16  PAD0  MOSI
*              PA17  PAD1  SCK
*              PA18  PAD2  /SS       hardware controlled
*              PA19  PAD3  MISO
*****************************************************/
void lcd_spi_transmit_cmd (uint8_t output_val){
	
	REG_PORT_OUTCLR1 = 0x01;  //RS
	REG_PORT_OUTCLR1 = 0x02;
	while(!(REG_SERCOM1_SPI_INTFLAG & 1)) {}    /* wait until Tx ready */
	REG_SERCOM1_SPI_DATA = output_val;          /* send data byte */
	while(!(REG_SERCOM1_SPI_INTFLAG & 1)) {}    /* wait until Tx ready */
	REG_PORT_OUTSET1 = 0x02;
	
}

/*****************************************************
* Function : lcd_spi_transmit_data
* Author   : Riwaz Awasthi
*
* Description: outputs a data passed in r16 via SPI port. Waits for data
;              to be written by SPI port before continuing.
*
*              PA16  PAD0  MOSI
*              PA17  PAD1  SCK
*              PA18  PAD2  /SS       hardware controlled
*              PA19  PAD3  MISO
*****************************************************/
void lcd_spi_transmit_data (uint8_t output_val){
	
	
	REG_PORT_OUTSET1 = 0x01;
	REG_PORT_OUTCLR1 = 0x02;
	while(!(REG_SERCOM1_SPI_INTFLAG & 1)) {}    /* wait until Tx ready */
	REG_SERCOM1_SPI_DATA = output_val;          /* send data byte */
	while(!(REG_SERCOM1_SPI_INTFLAG & 1)) {}    /* wait until Tx ready */
	REG_PORT_OUTSET1 = 0x02;
}

/*****************************************************
* Function : init_lcd_dog
* Author   : Riwaz Awasthi
*
* Description: inits DOG module LCD display for SPI (serial) operation.
*              Can be used as is with MCU clock speeds of 4MHz or less.
*
*              PA16  PAD0  MOSI
*              PA17  PAD1  SCK
*              PA18  PAD2  /SS       hardware controlled
*              PA19  PAD3  MISO
*****************************************************/
void init_lcd_dog(){
	init_spi_lcd();
	REG_PORT_OUTCLR1 = 0x02;
	delay_40ms ();     //startup delay
	REG_PORT_OUTSET1 = 0x02;
	delay_40ms();
	REG_PORT_OUTCLR1 = 0x02;
	
	//func_set1
	lcd_spi_transmit_cmd(0x39);
	delay_30us();
	//for(int i=0; i<200000; i++){}
	
	//func_set2
	lcd_spi_transmit_cmd(0x39);
	delay_30us();
	
	//bias_set
	lcd_spi_transmit_cmd(0x1E);
	delay_30us();
	
	//contrast set
	lcd_spi_transmit_cmd(0x7E);
	delay_30us();
	
	//power control
	lcd_spi_transmit_cmd(0x55);
	delay_30us();
	
	//follower control
	lcd_spi_transmit_cmd(0x6C);
	delay_30us();
	
	//display ON
	lcd_spi_transmit_cmd(0x0c);
	delay_30us();
	
	//clear display
	lcd_spi_transmit_cmd(0x01);
	delay_30us();
	
	//entry_mode
	lcd_spi_transmit_cmd(0x06);
	delay_30us();
	
}

/*****************************************************
* Function : update_lcd_dog
* Author   : Riwaz Awasthi
*
* Description: Updates the LCD display lines 1, 2, and 3, using the
*              contents of dsp_buff_1, dsp_buff_2, and dsp_buff_3, respectively.
*
*              
*****************************************************/
void update_lcd_dog(){
	init_spi_lcd();
	
	//init DDRAM addr-ctr
	lcd_spi_transmit_cmd(0x80);
	delay_30us();
	
	//send buffer 1
	for(int i=0; i<16; i++){
		lcd_spi_transmit_data(disp_buff_1[i]);
	}
	
	//init DDRAM addr-ctr
	lcd_spi_transmit_cmd(0x90);
	delay_30us();
	//send buffer 2
	for(int i=0; i<16; i++){
		lcd_spi_transmit_data(disp_buff_2[i]);
	}
	
	//init DDRAM addr-ctr
	lcd_spi_transmit_cmd(0xA0);
	delay_30us();
	//send buffer 3
	for(int i=0; i<16; i++){
		lcd_spi_transmit_data(disp_buff_3[i]);
	}
}
	
