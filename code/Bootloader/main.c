/*
	Nate's ARM Bootloader
	10/25/07
	
	Bootload binary file over USB
	
	11-2-2007: All bugs from the MSP side seem to be elimiated. There was the wastetime glitch not 
	servicing interrupts. And there was the line switch from v12 to mark2b. Removing MOSFETs.
	
	12-11-07: Trying to figure out how to get a bootloader version number in here. Bootloader
	spans from sector 0 to sector 8. 8 is where all the user settings are at (0x8000-0xFFFF).
	We will put the user settings at 0x8000 and the BL serial number at 0xFFF3
*/	

#include "LPC214x.h"

//UART0 Debugging
#include "serial.h"
#include "rprintf.h"

//Memory manipulation and basic system stuff
#include "firmware.h"
#include "system.h"

//SD Logging
#include "rootdir.h"
#include "sd_raw.h"

//USB
#include "main_msc.h"

//This is the file name that the bootloader will scan for
#define FW_FILE "FW.SFE"

const char bl_version[] __attribute__ ((section(".bl_version_number"))) = "v2.1";

#define ON	0
#define OFF	1


int main (void)
{

	boot_up();
	rprintf("Boot up complete\n");

	if(IOPIN0 & (1<<23))
	{
		main_msc();
	}
	else{
		rprintf("No USB Detected\n");
	}
	
	//Init SD
	if(sd_raw_init())
	{
		openroot();
		rprintf("Root open\n");
		if(root_file_exists(FW_FILE))
		{
			rprintf("New firmware found\n");
			load_fw(FW_FILE);
			rprintf("New firmware loaded\n");
		}
	}
	else{
		//Didn't find a card to initialize
		rprintf("No SD Card Detected\n");
		delay_ms(250);
	}
	rprintf("Boot Done. Calling firmware...\n");
	call_firmware();

	while(1);
}
