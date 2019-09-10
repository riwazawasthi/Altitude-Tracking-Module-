//***************************************************************************
//
// File Name : "alt3 "
// Date : 03/26/2019
// Version : 1.0
// Target MCU : ATSAML21J18B
// Target Hardware : DOG LCD module
// Author : Riwaz Awasthi
// DESCRIPTION : Implements an altitude tracking module
//               Altitude is tracked relative to assigned altitude.
//               If the altitude is 100ft higher than the assigned altitude
//               yellow and green LED turn on. If it is 200ft higher only
//               the yellow LED is turned ON. Similarly for altitude lower
//               than the base altitude, instead of the yellow LED, red LED turns ON
//               or OFF. If the altitude difference is lower than 100ft only the
//               green LED turns ON. The program also calculates the altitude and
//               displays it on the LCD module.
// Warnings :
// Restrictions : none
// Algorithms : none
// References :
//
// Revision History : Initial version
//
//
//**************************************************************************
#include "lcd.h"
#define alt_setting 2992     //29.92 "Hg pressure converted to 100" of Hg
static float PressureConst =  ((62500.0)/(6938624.0))*(100.0);

//***************************************************************************
//
// Function Name : "delay_ms "
// Date : 03/26/2019
// Version : 1.0
// Target MCU : ATSAML21J18B
// Target Hardware :
// Author : Riwaz Awasthi
// DESCRIPTION : Delays by n miliseconds
//
//
// Warnings : none
// Restrictions : none
// Algorithms : none
// References : none
//
// Revision History : Initial version
//
//**************************************************************************


void delayMs(int n){          // n ms delay 
	int i;
	for(int j = 0; j<4; j++)
	for(; n>0; n--)
	for (i=0; i<199; i++)
	__asm("nop");
}

//***************************************************************************
//
// Function Name : "int_char "
// Date : 03/26/2019
// Version : 1.0
// Target MCU : ATSAML21J18B
// Target Hardware :
// Author : Riwaz Awasthi
// DESCRIPTION : Converts an integer to a char array
//
//
// Warnings : none
// Restrictions : none
// Algorithms : none
// References : none
//
// Revision History : Initial version
//
//**************************************************************************
char* int_char(int n){
	static char DeciNum[10] = "";
	 
	// counter for decimal number array
	int i = 0;
	while(n!=0)
	{
		// temporary variable to store remainder
		int temp  = 0;
		
		// storing remainder in temp variable.
		temp = n % 10;
		
		// temp is always < 10
		DeciNum[i] = temp + 48;
		i++;
		
		n = n/10;
	}
		

	return DeciNum;
}
//***************************************************************************
//
// Function Name : "init_SAML21_ADC "
// Date : 04/08/2019
// Version : 1.0
// Target MCU : ATSAML21J18B
// Target Hardware :
// Author : Riwaz Awasthi
// DESCRIPTION : Initializes ATSAML21J18B's 12-bit ADC
//               PA10 = SET  
//               PB12 = green LED
//               PB13 = yellow LED
//               PB14 = red LED
// Warnings : none
// Restrictions : none
// Algorithms : none
// References : none
//
// Revision History : Initial version
//
//**************************************************************************
void init_SAML21_ADC(){
	REG_GCLK_PCHCTRL30 = 0x00000040;   //clock core for ADC
	REG_ADC_CTRLB = 3  ;    //clock divided by 16
	
	REG_PORT_DIRCLR0 = 0x00000400;  // PA10 configured as input for SET button
	REG_PORT_DIRSET1 = 0x00007000;  // PB12, PB13 and PB14 configures as output
	ARRAY_PORT_PINCFG0[10] = 6;      //PA10 pull enabled
	REG_PORT_OUTSET0 = 0x00000400;   //PA10 configured with pull up
	
	ARRAY_PORT_PINCFG0[3] |= 1;   //use PMUX for PA03
	ARRAY_PORT_PINCFG0[2] |= 1;   //use PMUX for PA02
	ARRAY_PORT_PMUX0[1] =  0x11; //PA03 = VREFA, PA02 = AIN0
	
	REG_ADC_REFCTRL = 3;      //use VREFA if 3, VDDANA if 5
	REG_ADC_INPUTCTRL = 0x1800; // V-= GND, V+ = AIN0(PA02)
	REG_ADC_CTRLA = 2;        //enable ADC
}

//***************************************************************************
//
// Function Name : "clr_dsp_buffs "
// Date : 03/26/2019
// Version : 1.0
// Target MCU : ATSAML21J18B
// Target Hardware : 
// Author : Riwaz Awasthi
// DESCRIPTION  : Clears the contents of display buffers
//
// Warnings : none
// Restrictions : none
// Algorithms : none
// References : none
//
// Revision History : Initial version
//
//**************************************************************************
void clr_dsp_buffs(){
	for (int i=0; i<16; i++){
		disp_buff_1[i] = ' ';
		disp_buff_2[i] = ' ';
		disp_buff_3[i] = ' ';
	}
	update_lcd_dog();
}
//***************************************************************************
//
// Function Name : "load_msg "
// Date : 03/26/2019
// Version : 1.0
// Target MCU : ATSAML21J18B
// Target Hardware : 
// Author : Riwaz Awasthi
// DESCRIPTION : Loads message to either disp_buff_1,
//               disp_buff_2 or dsp_buff_3
// 
//
// Warnings : none
// Restrictions : none
// Algorithms : none
// References : none
//
// Revision History : Initial version
//
//**************************************************************************
void load_msg( char msg[]){
	if(msg[16]=='1') {
		for(int i=0; i<16; i++){
			disp_buff_1[i]= msg[i];
		}
	}
	else if (msg[16] =='2') {
		for(int i=0; i<16; i++){
			disp_buff_2[i]= msg[i];
		}
	}
	else{
		for(int i=0; i<16; i++){
			disp_buff_3[i]= msg[i];
		}
	}
	
}

//***************************************************************************
//
// Function Name : main
// Date : 03/26/2019
// Version : 1.0
// Target MCU :
// Target Hardware ;
// Author : Riwaz Awasthi
// DESCRIPTION: main function of the program
//
//
// Warnings : none
// Restrictions : none
// Algorithms : none
// References : none
//
// Revision History : Initial version
//
//**************************************************************************
int main(void){
	init_SAML21_ADC();
	init_lcd_dog();
	clr_dsp_buffs();
	int initial ;
	int result, altitude;
	int diff, PressRead; 
	int pos_neg;
	int green, yellow, red;
	char* str;
	char disp_buff[17] = "alt = ";
	int index_buffer = 7;
	
	REG_ADC_SWTRIG = 2;   /* start a conversion */
    while(!(REG_ADC_INTFLAG &1)); /*wait for conversion complete*/
	result = REG_ADC_RESULT;
	
	initial = result;
	
	//startup display
	REG_PORT_OUTCLR1 =((1<<12)|(1<<13)|(1<<14));
	delayMs(1000);     // ~1s delay
	REG_PORT_OUT1 = ~(0x00000000);
	
	
	
	while (1){
		index_buffer = 7;  //reset the index to LCD display
		int temp = REG_PORT_IN0;
		REG_ADC_SWTRIG = 2;   /* start a conversion */
		while(!(REG_ADC_INTFLAG &1)); /*wait for conversion complete*/
		result = REG_ADC_RESULT;
		
		if (!(temp & 0x00000400)) {
			initial = result;
		}
		
		green = 0x00001000;
		yellow = 0x00000000;
		red = 0x00000000;
		if(result > initial){
			diff = result - initial;   // as you go down pressure/voltage increases
			if((diff >= 33) && (diff< 66)) {red = 1<<14;}			
			if (diff >= 66){
				green = 0x00000000;
				red = 1<<14;
			}
		}
		if (result < initial){
			diff = initial -result;
			if((diff >= 33) && (diff< 66)) {yellow = 1<<13;}
			if(diff>=66) {
				green = 0x00000000;
				yellow = 1<<13;
			}
		}
		
		
		
		if (yellow){    //blink yellow
			REG_PORT_OUT1 = ~(green | yellow) ;
			delayMs(250);
			REG_PORT_OUT1 = ~(green) ;
			delayMs(250);
		}
		else if (red) {    //blink red
			REG_PORT_OUT1 = ~(green | red) ;
			delayMs(250);
			REG_PORT_OUT1 = ~(green) ;
			delayMs(250);
		}
		else{
			REG_PORT_OUT1 = ~green;
		}
		//Calculate the altitude
		PressRead = result * PressureConst;
		if(alt_setting > PressRead) 
		{ 
			pos_neg = 1;
			disp_buff[index_buffer++] = '+';
		}
		else { 
			pos_neg = -1;
			disp_buff [index_buffer++] = '-';
		}
		altitude = pos_neg * 10 * (alt_setting - PressRead);
		str = int_char(altitude);
		
		for(int j=10; j>=0; j--){      //create the message to be written to LCD
			if(str[j] != 0){
				disp_buff[index_buffer] = str[j];
				index_buffer++;
			}
		}

		disp_buff[16] = '2';   //message to be displayed on 2nd LCD line
		load_msg(disp_buff);
		update_lcd_dog();
			
		
	}
	
	
}