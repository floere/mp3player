/* *********************************************************
	VS1002 MP3 Library. 
	Must include the vs1002.h header file.  
	Variable SONG_BYTES_BUFFEREDs should be defined in an 
	external header file and included in this library.
	
	Library contains the following functions:
	
	vs1002_initialize(void);		
	vs1002_SCI_read(unsigned char);
	vs1002_SCI_write(unsigned char, unsigned short int);
	vs1002_send_music(unsigned char*);
	vs1002_sineTest(unsigned char);
	vs1002_config(void);
	vs1002_finish(void);
	vs1002_mute(void);
	vs1002_setVolume(char setting);
	
	Written by Ryan Owens - 2/1/2008
  ********************************************************* */

/**********************************************************
                  Header files
 **********************************************************/
#include "LPC214x.h"		//Common LPC2148 definitions
#include "spi0.h"			//SPI port 0 communication
#include "MP3dev.h"			//Defines SONG_BYTES_BUFFERED
#include "vs1002.h"			//Contains Registers/Bit Masks for vs1002
#include "rprintf.h"
#include "serial.h"

/**********************************************************
                       Functions
**********************************************************/
//Usage: vs1002Init();
//Inputs: None
//Outputs: None
//Description: Initializes the vs1002 mp3 player to "New" modes
//			   and double clock frequency.
void vs1002Init(void){
	vs1002SCIWrite(SCI_MODE, SM_SDINEW);
	delay_ms(1);
	vs1002SCIWrite(SCI_CLOCKF, (SC_DOUBLECLK | SC_INCLK));
}

//Usage: vs1002Config()
//Inputs: None
//Outputs: None
//Description: Configures the LPC2148 for communication with the mp3 player
void vs1002Config(void){
	
	//Setup I/O Ports
	PINSEL0	|= (SCLK_PINSEL | MISO_PINSEL | MOSI_PINSEL);	// SPI pin connections
	PINSEL0 &= 0x0FFFFF0F;									
	
}

//Usage: vs1002Finish();
//Inputs: None
//Outputs: None
//Description: Relinquishes control of the SPI lines to the MP3 player
void vs1002Finish(void){
	PINSEL0 &= 0xFFFFC00F;		// Relinquish SPI pin connections
}

//Usage: register_value = vs1002SCIRead(SCI_MODE);
//Inputs: unsigned char address - Address of the register to be read
//Ouputs: None
//Description: Returns the value of the vs1002 register defined by "address"
unsigned short int vs1002SCIRead(unsigned char address)
{
	unsigned short int temp;
	SELECT_MP3_SCI();
	
	//delay_ms(1);
	SPI0_send(READ_COMMAND);
	SPI0_send(address);
   	temp = 	SPI0_recv();					
   	temp <<= 8;							
   	temp |= SPI0_recv();				
	//delay_ms(1);
	UNSELECT_MP3_SCI();
	
	delay_ms(1);
	return temp;
}

//Usage: vs1002SCIWrite(SCI_MODE, SM_SDINEW);
//Inputs: unsigned char address - Adress of the register to be written to
//		  unsigned short int data - Data to write to the register
//Outputs: None
//Description: Writes "data" to the register defined in "address"
void vs1002SCIWrite(unsigned char address, unsigned short int data)
{	
	SELECT_MP3_SCI();

	SPI0_send(WRITE_COMMAND);
	SPI0_send(address);
	SPI0_send(data >> 8);						// Send High Byte of data
	SPI0_send(data & 0x00ff);					// Send Low Byte of data

	UNSELECT_MP3_SCI();
	delay_ms(1);
}

//Usage: vs1002SineTest(126);
//Inputs: unsigned char pitch - pitch of the sine wave to be produced
//Outputs: None
//Description: Runs the Sine Test defined in the vs1002 datasheet
//**NOTE: ** for a sine wave test @ 5168 hz, send sequence: 0x53, 0xEF, 0x6E, 126, 0, 0, 0, 0
void vs1002SineTest(unsigned char pitch)
{
	SELECT_MP3_SDI();
	
   	SPI0_send(0x53);
	SPI0_send(0xEF);
	SPI0_send(0x6E);
   	SPI0_send(pitch);						//Send the Pitch	
	SPI0_send(0);
	SPI0_send(0);
	SPI0_send(0);
	SPI0_send(0);
	
   	UNSELECT_MP3_SDI();
}

//Usage: vs1002Mute();
//Inputs: None
//Outputs: None
//Description: Mutes the output of the vs1002 MP3 player
void vs1002Mute(void){
	vs1002SCIWrite(SCI_VOL, SV_MUTE);
}


//Usage: vs1002SetVolume(INCREASE);
//Inputs: char setting - either INCREASE(1) or DECREASE(0)
//Outputs: None
//Description: Increases or decreases the volume of the MP3 player based on the value of "setting"
void vs1002SetVolume(char setting){
	unsigned short int currentVolume=0;
	
	currentVolume = vs1002SCIRead(SCI_VOL);
	if(setting == INCREASE){
		if(currentVolume == SV_MAX_VOLUME);	//Don't Change the current volume
		else currentVolume -= 0x0808;		//Increment both channels equally
	}
	else{
		if(currentVolume == SV_MUTE);			//Don't change the current volume
		else currentVolume += 0x0808;
	}
	vs1002SCIWrite(SCI_VOL, currentVolume);
}

//Usage: vs1002SendMusic(bufferedSongData, BUFFERSIZE);
//Inputs: unsigned char* songData - pointer to MP3 data that is to be sent to mp3 player
//		  int buffer_size - size of song data in bytes
//Outputs: None
//Description: Sends the data in the songData buffer to the MP3 player
void vs1002SendMusic(unsigned char* songData, int buffer_size){
	SELECT_MP3_SDI();
	for(int i=0; i<buffer_size; i++){
		SPI0_send(*songData++);			//Send the buffered byte of data, then increment the buffer position
	}
	UNSELECT_MP3_SDI();
}

//Usage: vs1002Reset();
//Inputs: None
//Outputs: None
//Description: Resets the mp3 player
void vs1002Reset(void){
	//Reset the MP3 Player and make sure the mp3 player doesn't try to boot from SPI
	IOCLR0 = MP3_GPIO0;		//Pull GPIO0 low
	IOCLR1 = MP3_XRES;		//Reset the vs1002
	delay_ms(10);			//Hold Reset
	IOSET1 = MP3_XRES;		//Bring vs1002 out of reset
}
