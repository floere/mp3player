/* *********************************************************
	NS73 FM Transmiter Library. 
	Must include the NS73.h header file.  
	FM_SCLK, FM_DIO, FM_TEB and FM_LA should be defined in
	an external header file and included.
	
	Library contains the following functions:
	
	void ns73Config(void);
	void ns73Init(void);
	void ns73SerialReset(void);
	void ns73SetChannel(int radio_channel);
	
	Written by Ryan Owens - 2/1/2008
  ********************************************************* */

//**********************************************************
//                  Header files
//**********************************************************
#include "LPC214x.h"		//Common LPC2148 definitions
#include "MP3dev.h"			//
#include "NS73.h"			//Contains Registers/Bit Masks for vs1002
#include "rprintf.h"

	/**********************************************************
                       Functions
**********************************************************/
//Usage: ns73_config();
//Inputs: None
//Outputs: None
//Description: Configures the LPC2148 for communication with the ns73 module
void ns73Config(void)
{
    //Setup Ports
	IODIR1 |= (FM_SCLK | FM_DIO);		//Setup FM Trans. I/O Lines
	IODIR1 |= FM_LA;					//**This has been done in the bootup() routine
	IODIR0 &= ~FM_TEB;
	IOSET1 |= FM_SCLK | FM_DIO;
	delay_ms(1);
}

//Usage: ns73_init()
//Inputs: None
//Outputs: None
//Description: Resets the ns73 module and restores the default values to each register
void ns73Init(void){
	ns73Send(R14, R14_RESET); //Software reset
	delay_ms(1);				 //
	
    //Load register values with initial default values
    ns73Send(R1, R1_DEFAULT); 		//Register 1 defaults
    ns73Send(R2, R2_DEFAULT); 		//Register 2 defaults
    ns73Send(R3, 0x8A); 			//Set broadcast freq to 97.3
    ns73Send(R4, 0x2E);
	ns73Send(R8, R8_DEFAULT); 		//Register 8 defaults
    ns73Send(R0, MUTE); 			//Flip the power switch
    ns73Send(R14, R14_RESET); 		//Software reset
    ns73Send(R6, R6_DEFAULT); 		//Set Register 6
}

//Usage: ns73Send(R1, R1_DEFAULT);
//Inputs: unsigned char outgoing_address - address of the ns73 register to be written to
//		  unsigned char outgoing_data - data to be written to the register
//Outputs: None
//Description: Basic SPI send address and data bytes to ns73 module
unsigned char ns73Send(unsigned char outgoing_address, unsigned char outgoing_data)
{
    unsigned char incoming_byte=0, x;

    UNLATCH;
    delay_ms(10);
    for(x = 0 ; x < 4 ; x++)
    {
        CLKDWN; //Toggle the SPI clock
        if((outgoing_address & 0x01)==0x01){
			IOSET1 |= FM_DIO; //Put bit on SPI data bus
		}	
		else IOCLR1 |= FM_DIO;
		outgoing_address >>= 1; //Rotate 1 bit to the right
		delay_ms(1);
        CLKUP;
		delay_ms(1);
    }

    for(x = 0 ; x < 8 ; x++)
    {
        CLKDWN; //Toggle the SPI clock
		ledRedOn();
        if((outgoing_data & 0x01)==0x01){
			IOSET1 |= FM_DIO; //Put bit on SPI data bus
		}
		else IOCLR1 |= FM_DIO;
		outgoing_data >>= 1; //Rotate 1 bit to the right
		delay_ms(1);
        CLKUP;
		delay_ms(1);
    }
    LATCH; //Latch this trasfer
    delay_ms(10); //Minimum tLAH is 250nS
    UNLATCH;
	CLKDWN;
    return(incoming_byte);
}

//Usage: ns73SerialReset()
//Inputs: None
//Outputs: None
//Description:  Resets the ns73 module
void ns73SerialReset(void){
		delay_ms(10);
		IOCLR1 |= FM_DIO;
		delay_ms(10);
		IOCLR1 |= FM_SCLK;
		delay_ms(10);
		IOSET1 |= FM_DIO;
		for(int i=0; i<26; i++){
			IOSET1 |= FM_SCLK;
			delay_ms(10);
			IOCLR1 |= FM_SCLK;
			delay_ms(10);
		}
		IOSET1 |= FM_SCLK;
		delay_ms(10);
		IOCLR1 |= FM_DIO;
		delay_ms(10);
		IOCLR1 |= FM_SCLK;
		delay_ms(10);
		IOSET1 |= FM_SCLK;
		delay_ms(10);
		IOSET1 |= FM_DIO;
		delay_ms(10);
		IOCLR1 |= FM_SCLK;
}

//Usage: ns73SetChannel(973);
//Inputs: int radio_channel - Desired Radio Channel (should be input in MHz*10 so 97.3FM becomes 973)
//Outputs: None
//Description: Tunes the ns73 module to "radio_channel."
void ns73SetChannel(int radio_channel){
	long int channel=radio_channel*100000;
	unsigned char low_byte, high_byte;
	
	channel += 304000;
	channel /= 8192;
	low_byte = channel & 0xFF;
	high_byte = (channel >> 8) & 0xFF;
	
	ns73Send(R3, low_byte);
	ns73Send(R4, high_byte);
}
