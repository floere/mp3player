/*
 * So, basically we have two timers:
 * 1. A timer that shoves data into the mp3 buffer (32 bytes) of the chip.
 * 2. A timer that checks the USB Interface for new stuff or a removed memstick.
 * 
 * 
 * r. We have a radio.
 *    Why? Think synchronized music in the whole apartment.
 *    Perhaps it can be switched off or on. Perhaps it is always running.
 *    Let's use the accelerometer for that maybe (punching the thing ;) ).
 *    Frequency? 103.6 MHz? some band where nothing is on.
 *    Let's also send intermittent radio info to let the people
 *    around me know where the good/bad music comes from.
 *
 */

//*******************************************************
//					MP3 Development Platform
//*******************************************************
//#include <stdio.h>
#include "LPC214x.h"
#include "setup.h"
#include "MP3Dev.h"
#include "serial.h"
#include "rprintf.h"

//*******************************************************
//				Memory Management Libraries
//*******************************************************
#include "rootdir.h"
#include "sd_raw.h"
#include "fat16.h"

//*******************************************************
//				USB Libraries
//*******************************************************
#include "main_msc.h"

//*******************************************************
//					External Component Libs
//*******************************************************
#include "LCD_driver.h"
#include "MMA7260.h"
#include "vs1002.h"
#include "ns73.h"

//*******************************************************
//					Interrupt Functions
//*******************************************************

static void timer0ISR(void);
static void timer1ISR(void);


//*******************************************************
//					External Variables
//*******************************************************
extern char white;	//Color "White" used in LCD Functions(From LCD_driver.h)
extern char black;	//Color "black" used in LCD Functions(From LCD_driver.h)
extern char red;	//Color	"red" used in LCD Functions(From LCD_driver.h)

//*******************************************************
//				Global Variables for Main
//*******************************************************
static char button_pressed;				//Holds the button value of the Nav. Switch.  Updated in the Timer 1 interrupt and monitored in the main routine
static char update_screen;				//Flag which indicates if screen has been rotated.
static int prev_position;				//Accelerometer Value placeholder
static int cur_position;				//Accelerometer Value placeholder
static char file_is_open=0;				//Global Flag indicating if a file is currently open
static char song_is_over=0;				//Global Flag indicating if the current song has finished playing

static SongStruct current_song;			//Holds information related to the currently selected song
static DisplayStruct file_manager;		//Main menu display which contains list of MP3's on SD card
static DisplayStruct settings_menu;		//Settings Menu...
static DisplayStruct *current_display;	//
static int NUMBEROFFILES=0;

int volume_setting=16, radio_channel=973;
char radio_enable=OFF;

//*******************************************************
//					Display Strings
//*******************************************************
const char welcome[22]="Song List:\n\r";
const char NotFound[15]="File Not Found!";
const char USB[14]="USB Connected!";
const char newline[2] = "\n";
const char volume[7] = "Volume";
const char contrast[9] = "Contrast";
const char radio_power[11]= "Radio Power";
const char radio_channel_menu[13]= "Radio Channel";
const char settings[10]="Settings:";

int main (void)
{
//*******************************************************
//					Local Variables
//*******************************************************
unsigned long int numberOfChars=0;

//*******************************************************
//					Main Code
//*******************************************************
	//Initialize ARM I/O
	bootUp();
	
	//Initialize the MP3 player with double clock speed and lower volume
	vs1002Config();			//Configure MP3 I/O
	vs1002Reset();			//Reset MP3 Player
	vs1002Init();			//Double the clock speed and set to "New" mode
	for(char i=0; i<2; i++)vs1002SetVolume(DECREASE);		//Lower the Volume!!
	vs1002Finish();			//Restore the SPI I/O lines
	
	//Show the splash-screen (Sparkfun Logo)
	LCDInit();				//Initialize the LCD
	LCDClear(white);		//Clear the screen with white
	LCDPrintLogo();			//Print the Sparkfun Logo
	
	//Initialize the FM Transmitter to 97.3
	IOCLR1 |= FM_CS;			//Select SPI for FM Transmitter
	delay_ms(900);				//
	ns73Config();				//Configigure the FM Trans. I/O
	ns73Init();					//Setup the Default Register Values
	ns73SetChannel(973);		//Transmit to 97.3 FM	
	IOSET1 |= FM_CS;			//Remove FM Transmitter from SPI bus
	delay_ms(100);				//

	//Find Out how many files are on the SD card
	PINSEL0 |= (SCLK_PINSEL | MISO_PINSEL | MOSI_PINSEL);	//Make sure SPI is selected for reading the card
	do{
		numberOfChars++;
	}while(rootDirectory_files_stream(0)!='\0');			//Get the total number of characters in the filenames on the SD card
	char tempNames[numberOfChars];	//This is one array that contains all of the filenames in the SD card, seperated by '\0'
	NUMBEROFFILES=rootDirectory_files(tempNames, numberOfChars+1);
	FileStruct Files[NUMBEROFFILES];						//Dynamically create an array for all of the filenames
	
	//Get all of the filenames into a Global Array
	int chartracker=0;	
	for(int j=0; j<NUMBEROFFILES; j++){
		for(int i=0; i<=MAXFILENAMELEN+1; i++){
			if(chartracker <= numberOfChars){
				if(tempNames[chartracker]==','){
					Files[j].file_name[i] = '\0';
					i=MAXFILENAMELEN+1;
				}
				else Files[j].file_name[i] = tempNames[chartracker];
				chartracker++;
			}
			else Files[j].file_name[i] ='\0';
		}	
	}
	
	
	//Load the file manager with the songs/files on the SD card and set this as the current display
	file_manager.total_pages = (NUMBEROFFILES/NUMROWS)-1;
	if(NUMBEROFFILES%NUMROWS != 0)file_manager.total_pages+=1;
	
	file_manager.orientation =ORIENTUP;
	file_manager.text_color=white;
	file_manager.back_color=black;
	
	file_manager.current_page=0;
	file_manager.current_row=1;
	file_manager.current_index=0;	
	getNewFiles(&file_manager, &Files[0]);
	for(int i=0; i<MAXDISPLEN; i++)file_manager.title[i]=welcome[i];
	
	//Generate the settings menu and fill in default settings values
	fillSettings(&settings_menu);
	
	//Setup the menu
	current_display = &file_manager;
	PINSEL0 &= 0xFFFFC00F;		//Hand over SPI lines to LCD talk
	LCDClear(current_display->back_color);
	printMenu(current_display);
	
	while(1){
		if(IOPIN0 & (1<<23))
		{
			LCDClear(white);
			LCDPrintString(USB, 0, black, 7,4,current_display->orientation);
			VICIntEnClr = 0x30;		//Stop all interrupts to allow USB communication
			main_msc();
			reset();
		}
	
		//Wait for a button to be pressed or for the screen to be rotated.
		//while(!button_pressed && !update_screen){
		if(!button_pressed && !update_screen){
			//If the current song is done playing, start playing the next song if it's available.
			if(song_is_over){
				VICIntEnClr = 0x20;			//Stop the "Send Song" interrupts
				closeSong(&current_song);	//Close the current song
				file_is_open=0;				//change master flag
				ledBlueOff();
				//Get the next song on the page by changing rows
				if(file_manager.current_row < NUMROWS){
					file_manager.current_row++;
					file_manager.current_index++;
				}
				//Go to the next page of songs if necessary
				if((file_manager.current_row == NUMROWS) && (file_manager.current_page < file_manager.total_pages)){
					file_manager.current_row=1;
					file_manager.current_index=0;
					file_manager.current_page++;
					getNewFiles(&file_manager, &Files[0]);		//Get the next page of songs
				}
				//If there is another song in the list, get it and play it!
				if(file_manager.list[file_manager.current_row+1].file_name[0] != '\0'){	
					LCDClear(file_manager.back_color);
					printMenu(&file_manager);									//Print the menu with the next song!
					file_is_open=loadSongInfo(&current_song, &file_manager);	//Get the current song info.					
					vs1002Config();												//Enable the MP3 Comm. Lines
					vs1002SCIWrite(SCI_MODE, SM_SDINEW);						//Make sure the MP3 player is in the right mode.
					vs1002Finish();												//Disable the MP3 Comm. Lines
					ledBlueOn();
					VICIntEnable |= 0x20;		//Enable Timer 0 Interrupts(This is the "Song Sending" interrupt).
				}	
			}
		}

		if(button_pressed == UP_BUT)handleUpButton(current_display, &Files[0]);
		else if(button_pressed == DWN_BUT)handleDownButton(current_display, &Files[0]);
		else if(button_pressed == MID_BUT)handleMiddleButton();
		
		else if(update_screen){
			VICIntEnClr = 0x10;	//Stop interrupts and allow accelerometer to stabilize
			delay_ms(300);
			if(MMA_get_y()>700 && prev_position>700){	//Get new accelerometer value; make sure position is correct.
				if(file_is_open)quickClear(current_display);
				else LCDClear(current_display->back_color);
				current_display = &settings_menu;
				current_display->orientation=ORIENTLEFT;
				ledBlueOff();
				if(file_is_open)ledGrnOn();
				else ledRedOn();
			}
			else if(MMA_get_y()<700 && prev_position<700){
				if(file_is_open)quickClear(current_display);
				else LCDClear(current_display->back_color);
				current_display = &file_manager;
				current_display->orientation=ORIENTUP;
				ledGrnOff();
				ledRedOff();
				if(file_is_open)ledBlueOn();
			}
			printMenu(current_display);					//If the screen has changed, show the new menu
			VICIntEnable |= 0x10;						//Re-enable interrupts
		}
		button_pressed=NO_BUT;
		VICIntEnable |= 0x10;
	}
    return 0;
}


//Usage: delay_ms(1000);
//Inputs: int count: Number of milliseconds to delay
//The function will cause the firmware to delay for "count" milleseconds.
void delay_ms(int count)
{
    int i;
    count *= 10000;
    for (i = 0; i < count; i++)
        asm volatile ("nop");
}

//Usage: bootUp();
//Inputs: None
//This function initializes the serial port, the SD card, the I/O pins and the interrupts
void bootUp(void)
{
    rprintf_devopen(putc_serial0); //Init rprintf
    delay_ms(10); //Delay for power to stablize

    //Bring up SD and FAT
    if(!sd_raw_init())
    {
        rprintf("SD Init Error\n");
    }
    if(openroot())
    {
        rprintf("SD OpenRoot Error\n");
    }
     
	//Initialize I/O Ports and Peripherals
	//Setup the MP3 I/O Lines
	IODIR0 |= MP3_XCS;
	IODIR0 &= ~MP3_DREQ;
	PINSEL1 |= 0x00000C00;	//Set the MP3_DREQ Pin to be a capture pin
	IODIR1 |= MP3_XDCS | MP3_GPIO0 | MP3_XRES;	
	
	//Setupt the FM Trans. Lines
	IODIR1 |= FM_LA; 												//FM Trans Outputs (Leave SPI pins unconfigured for now)
	IODIR1 |= FM_CS;
	
	//Setup the SD Card I/O Lines
	IODIR0 |= SD_CS;												//SD Card Outputs
	
	//Setup the Accelerometer I/O Lines
	IODIR0 |= (GS1 | GS2);											//Accelerometer Outputs
	PINSEL0 |= (MMA_X_PINSEL | MMA_Y_PINSEL | MMA_Z_PINSEL);		//Make sure that ADC pins have ADC Functions selected
	IOCLR0 = (GS1 | GS2);											//Init. Accel. to 1.5G Mode
	
	//Setup the LCD I/O Lines
	IODIR0 |= (LCD_RES | LCD_CS);									//LCD Outputs
	
	//Setup the LED Lines										
	IODIR0 |= (LED_BLU | LED_RED | LED_GRN);						//Led's
	ledBlueOff();
	ledRedOff();
	ledGrnOff();
	
	//Setup the Buttons
	IODIR1 &= (~SW_UP & ~SW_DWN & ~SW_MID);		//Button Inputs

	IODIR0 &= ~(1<<23);							//Set the Vbus line as an input

    //Setupt the Interrupts
	VPBDIV=1;										// Set PCLK equal to the System Clock	
	VICIntSelect = ~0x30; 							// Timer 0 AND TIMER 1 interrupt is an IRQ interrupt
    VICIntEnable = 0x10; 							// Enable Timer 0 Interrupts (Don't start sending song data with Timer 1)
    VICVectCntl0= 0x25; 							// Use slot 0 for timer 1 interrupt
    VICVectAddr0 = (unsigned int)timer1ISR; 		// Set the address of ISR for slot 1		
    VICVectCntl1 = 0x24; 							// Use slot 1 for timer 0 interrupt
    VICVectAddr1 = (unsigned int)timer0ISR; 		// Set the address of ISR for slot 1
	
	//Configure Timer0
	T0PR = 300;										//Divide Clock by 300 for 40kHz PS
	T0TCR |=0X01;									//Enable the clock
	T0CTCR=0;										//Timer Mode
	T0MCR=0x0003;									//Interrupt and Reset Timer on Match
	T0MR0=1000;										//Interrupt on 40Hz
	
	//Configure Timer1
	T1PR = 200;										//Divide Clock by 300 for 40kHz PS
	T1TCR |=0X01;									//Enable the clock
	T1CTCR=0;										//Timer Mode
	T1CCR=0x0A00;									//Capture and interrupt on the rising edge of DREQ
	
	//Setup the SPI Port
    S0SPCCR = 64;              											// SCK = 1 MHz, counter > 8 and even
    S0SPCR  = 0x20;                										// Master, no interrupt enable, 8 bits	
}

//Usage: None (Automatically Called by FW)
//Inputs: None
//This function is a global interrupt called by a match on the Timer 0 match.  This interrupt
//	is responsible for sending music to the MP3 player when it is needed. 
//WARNING: Altering the Timer 0 Prescale register or Timer 0 Match value will put proper MP3 playing at risk.
//			Adding superfluous code to this interrupt section may also contribute to improper MP3 playback.
static void timer1ISR(void)
{
	vs1002Config();												//Enable MP3 Comm. Lines
	while(IOPIN0 & MP3_DREQ){
		vs1002SendMusic(current_song.data, MAXBUFFERSIZE);		//Send the buffered song data
		if(fat16_read_file(current_song.handle, current_song.data, MAXBUFFERSIZE) <= 0)song_is_over=1;	//Buffer more data if available
		else song_is_over=0;									//if there's no more data available, set the flag, else leave it alone
	}	
	vs1002Finish();												//Disable MP3 Comm. Lines
	T1IR = 0xFF; 												//Clear the timer 0 interrupt
	VICVectAddr = 0; 											//Update VIC priorities	
}

//Usage: None (Automatically Called by FW)
//Inputs: None
//This function is a global interrupt called by a match on the Timer 1 match.  The interrupt
// is responsible for determining if a button has been pressed or if the screen has been rotated
// and setting the appropriate global flag if either has occured.
static void timer0ISR(void)
{
	button_pressed=getButton();			//Find out if a button has been pressed, and which one
	cur_position=MMA_get_y();			//Get the current position of the screen
	if((prev_position<700 && cur_position>700)||(prev_position>700 && cur_position<700))update_screen=1; 
	else update_screen=0;				//Determine if the accelerometer has been rotated
	prev_position=cur_position;			//Save the position value for later reference
	T0IR = 0xFF;						//Clear the timer interrupt
	VICVectAddr =0;						//Update the VIC priorities
}

//Usage: button_value=getButton();
//Inputs:  None
//Outputs: None
//Function returns the value of the button that is currently being pressed.
// UP_BUT, DWN_BUT, and MID_BUT values can be viewed in MP3Dev.h
char getButton(void){
	if(!(IOPIN1 & SW_UP)) return UP_BUT;
	else if(!(IOPIN1 & SW_DWN)) return DWN_BUT;
	else if(!(IOPIN1 & SW_MID)) return MID_BUT;
	return NO_BUT;
}


void getNewFiles(DisplayStruct *files, FileStruct *file_list){
	if(files->current_page < files->total_pages){
		for(int i =0; i<NUMROWS; i++){
			for(int j=0; j<MAXFILENAMELEN; j++){
				files->list[i].file_name[j] = file_list[files->current_page*NUMROWS+i].file_name[j];
			}
		}
	}
	else{
		for(int i=0; i<(NUMBEROFFILES-files->total_pages*NUMROWS); i++){
			for(int j=0; j<MAXFILENAMELEN; j++){
				files->list[i].file_name[j] = file_list[files->current_page*NUMROWS+i].file_name[j];
			}
		}
		for(int i=(NUMBEROFFILES-files->total_pages*NUMROWS); i<NUMROWS; i++)files->list[i].file_name[0]='\0';
	}
}

//Usage: file_is_open = loadSongInfo(&current_song, &file_manager);
//Inputs: SongStruct *song: Pointer to the struct in which the song information will be loaded
//		  DisplayStruct *selected_song: Pointer to the song whose information will be loaded
//The function takes the currently selected file name from the selected_song structure and loads
// the vital file information into the song structure.  The function also opens the song and prepares
// it to be played.  If the function succesfully opens the song, a 1 is returned, else a 0 is returned.
char loadSongInfo(SongStruct *song, DisplayStruct *selected_song){
	//Get the selected songs file name
	for(int i=0; i<MAXFILENAMELEN; i++){
		song->file_name[i] = selected_song->list[selected_song->current_index].file_name[i];
	}
	//Get the selected songs display name
	for(int i=0; i<MAXDISPLEN; i++){
		if((song->file_name[i] == '.') || (i==MAXDISPLEN-1)){
			song->name[i]='\0';
			i=MAXDISPLEN;
		}	
		else song->name[i]=song->file_name[i];
	}
	
	PINSEL0 |= (SCLK_PINSEL | MISO_PINSEL | MOSI_PINSEL);	//Make sure SPI is selected for reading the card
	//Check to see if the selected song is valid
	if(!root_file_exists(song->file_name)){
		PINSEL0 &= 0xFFFFC00F;
		return 0;
	}	
	//Open the selected song
	else{
		song->handle=root_open(song->file_name);
		song->size=fat16_file_size(song->handle);
		fat16_read_file(song->handle, song->data, MAXBUFFERSIZE);
	}
	
	
	PINSEL0 &= 0xFFFFC00F;
	return 1;
}

//Usage: CloseSong(&current_song);
//Inputs: SongStruct *song: Pointer to a song structure to be closed
//Function closes a song that is currently open.
void closeSong(SongStruct *song){
	PINSEL0 |= (SCLK_PINSEL | MISO_PINSEL | MOSI_PINSEL);
	fat16_close_file(song->handle);
	PINSEL0 &= 0xFFFFC00F;
}

//Usage: highlightRow(&current_display, PREVIOUS);
//Inputs: DisplayStruct *current_display: Pointer to the display structure whose current row value should be changed.
//		  char direction: direction value can be PREVIOUS or NEXT.  Value indicates which row should be highlighted
//Depending on "direction" this value highlights a row, and manages the current_row and current_index values.
void highlightRow(DisplayStruct *display, char direction){
	PINSEL0 &= 0xFFFFC00F;		//Hand over SPI lines to LCD talk
	LCDSetRowColor(display->current_row, 0, display->back_color, display->orientation);
	LCDPrintString(display->list[display->current_index].file_name, 0,display->text_color, display->current_row,0,display->orientation);
	if(direction == PREVIOUS){
		display->current_row--;
		display->current_index--;
	}
	else{
		display->current_row++;
		display->current_index++;		
	}
	LCDSetRowColor(display->current_row, 0, display->text_color, display->orientation);
	LCDPrintString(display->list[display->current_index].file_name, 0,display->back_color, display->current_row,0,display->orientation);	
}

//Usage: printMenu(&settings_menu);
//Inputs: DisplayStruct *display: Pointer to the display struct to be displayed on the LCD.
//This function writes the title, along with all of the file names in the display struct to the screen.
void printMenu(DisplayStruct *display){
	PINSEL0 &= 0xFFFFC00F;		//Hand over SPI lines to LCD talk
	if(display->current_page==0)LCDPrintString(display->title,0, display->text_color, 0,0,display->orientation);
	for(int j=0; j<NUMROWS; j++){
		LCDPrintString(display->list[j].file_name,0, display->text_color, j+1,0,display->orientation);
	}
	LCDSetRowColor(display->current_row, 0, display->text_color, display->orientation);
	LCDPrintString(display->list[display->current_index].file_name, 0,display->back_color, display->current_row,0,display->orientation);
}

//Usage: fillSettings(&settings_menu, &settings_values);
//Inputs: DisplayStruct *menu: Struct whose file names should be loaded with the settings menu
//		  SettingsStruct *values: settings struct whose initial values should be set
//This function fills the input display struct with a list of settings, along with a "settings" title.  It
//also loads the default display values along with the default settings values. 
void fillSettings(DisplayStruct *menu){
	for(int i=0; i<MAXDISPLEN; i++){
		menu->list[0].file_name[i] = volume[i];
		menu->list[1].file_name[i] = radio_power[i];
		menu->list[2].file_name[i] = radio_channel_menu[i];
		menu->title[i] = settings[i];
	}
	for(int i=3; i<NUMROWS; i++){
		menu->list[i].file_name[0]='\0';
	}
	menu->text_color=white;
	menu->back_color=black;
	menu->current_row=1;
	menu->current_column=0;
	menu->current_index=0;
	menu->orientation = ORIENTLEFT;
	menu->current_page=0;
	menu->total_pages = 0;
}

//Usage: handleUpButton(current_display, &Files[0]);
//Inputs: 	DisplayStruct *display: Display currently being shown to the user.
//			FileStruct *Files: List of files on the SD card
//Outputs: None
//Description: This function will highlight the previous row on the screen.  If the file menu is being shown,
// and the first file is currently selected, the function will get the previous group of files and display them.
void handleUpButton(DisplayStruct *display, FileStruct *Files){
	if(display->current_row >1){	//See if the currently highlighted row is at the top of the page
		highlightRow(display, PREVIOUS); //If it isn't, then highlight the previous row
	}
	//If the current row is at the top of the page, and there are previous songs, then get them.
	else if((display->current_row == 1) && (display->current_page > 0)){	//If a prev. page exists, load it
		//Load previous 15 songs and reset row, index and offset values
		display->current_page-=1;					//Set the "new" current page to be loaded
		//if(file_is_open)quickClear(display);
		//else LCDClear(display->back_color);
		quickClear(display);
		display->current_row=15;					//Start current row at bottom of next screen
		display->current_index=14;					//
		getNewFiles(&file_manager, &Files[0]);		//Load the previous page
		printMenu(display);							//	display previous page
	}
}

//Usage: handleDownButton(current_display, &Files[0]);
//Inputs: 	DisplayStruct *display: Display currently being shown to the user.
//			FileStruct *Files: List of files on the SD card
//Outputs: None
//Description: This function will highlight the next row on the screen.  If the file menu is being shown,
// and the last file is currently selected, the function will get the next group of files and display them.
void handleDownButton(DisplayStruct *display, FileStruct *Files){
	if(display->current_row <15){	//See if the currently highlighted row is at the bottom of the page
		highlightRow(display, NEXT);	//If it's not, then highlight the next row
	}
	//If the current highlighted row is at the bottom, and there're more songs, then get them
	else if((display->current_row == 15) && (display->current_page < display->total_pages)){
		//Load next 15 songs and reset row, index and offset values
		display->current_page+=1;		
		//if(file_is_open)quickClear(display);
		//else LCDClear(display->back_color);
		quickClear(display);
		display->current_row=1;
		display->current_index=0;
		getNewFiles(&file_manager, &Files[0]);
		printMenu(display);				
	}
}

//Usage: handleMiddleButton();
//Inputs: None
//Outputs: None
//Description: This function is called when the middle button is pressed.
//  The function performs an action based on the current state of the MP3 player.
//	(1.-If the file menu is displayed, and a song is not being played, the function starts the currently highlighted song.)
//	(2.-If the file menu is displayed and a song IS being played, the fucntion stops the currently playing song)
//	(3.-If the settings menu is displayed, the function opens the selected setting and allows the user to edit the setting.)
void handleMiddleButton(void){
	VICIntEnClr = 0x10;
	delay_ms(250);
	
	//If the File Menu is being displayed, middle button acts like play/stop
	if(current_display == &file_manager){
		if(!file_is_open){				//If a file isn't already playing then this acts like a play button
			file_is_open=loadSongInfo(&current_song, &file_manager);	//Get the current song info.
			if(!file_is_open){			//Make sure this is a valid file
				LCDClear(white);
				LCDPrintString(NotFound, 0, black, 1,0,current_display->orientation);
				delay_ms(1000);
				LCDClear(black);
				printMenu(&file_manager);
			}	
			else{
				vs1002Config();							//Enable the MP3 Comm. Lines
				vs1002SCIWrite(SCI_MODE, SM_SDINEW);	//Make sure the MP3 player is in the right mode.
				vs1002Finish();							//Disable the MP3 Comm. Lines
				ledBlueOn();
				//Send first song data
				vs1002Config();										//Enable MP3 Comm. Lines
				while((IOPIN0 & MP3_DREQ) != 0){
					vs1002SendMusic(current_song.data, MAXBUFFERSIZE);	//Send the buffered song data
					if(fat16_read_file(current_song.handle, current_song.data, MAXBUFFERSIZE) <= 0)song_is_over=1;	//Buffer more data if available
					else song_is_over=0;									//if there's no more data available, set the flag, else leave it alone
				}
				vs1002Finish();
				PINSEL1 |= 0x00000C00;						
				VICIntEnable |= 0x20;		//Enable Timer 1 Interrupts(This is the "Song Sending" interrupt).
				IODIR0 |= (LCD_DIO | LCD_SCK | LCD_CS | LCD_RES);		//Assign LCD pins as Outputs
			}
		}	
		else{							//If a file is already open then this acts like a stop button
			VICIntEnClr = 0x20;											//Disable Time 0 Interrupts(Stop the "Song Sending" interrupt)
			ledBlueOff();
			vs1002Config();												//Enable the MP3 Comm Lines
			vs1002SCIWrite(SCI_MODE, SM_OUTOFWAV);						//Tell the MP3 Player to jump out of WAV decoding
			for(int i=0; i<150; i++)vs1002SCIWrite(SCI_MODE, 0x00);	//Send 150 zeroes to the player to clear it's FIFO.
			vs1002Finish();												//Disable the MP3 Comm. Lines
			IODIR0 |= (LCD_DIO | LCD_SCK | LCD_CS | LCD_RES);			//Assign LCD pins as Outputs
			closeSong(&current_song);									//Close the current song
			file_is_open=0;												//Clear the global flag
			VICIntEnable = 0x10;
		}
	}
	
	//Else we are on the Settings menu, and we need to handle the settins options
	else{
		if(file_is_open)quickClear(current_display);
		else LCDClear(settings_menu.back_color);
		LCDPrintString(current_display->list[current_display->current_index].file_name,0,current_display->text_color,1,0,current_display->orientation);
		VICIntEnable|=0x10;
		if(current_display->current_row==VOLUMEMENU){
			VICIntEnable |= 0x10;
			LCDSetRowColor(2, 0, current_display->back_color, current_display->orientation);
			LCDPrintString("%d", volume_setting, white, 2, 0, current_display->orientation);
			while(button_pressed < MID_BUT){
				VICIntEnClr = 0x10;				//Stop Interrupts to
				delay_ms(150);					//	debounce the switch					
				if(button_pressed==UP_BUT){
					if(volume_setting < 32){
						volume_setting+=1;
						vs1002Config();				//Enable the MP3 Comm. Lines
						vs1002SetVolume(INCREASE);	//Lower the volume
						vs1002Finish();				//Disable MP3 Comm. Lines
					}	
					LCDSetRowColor(2, 0, current_display->back_color, current_display->orientation);
					LCDPrintString("%d", volume_setting, current_display->text_color, 2, 0, current_display->orientation);
				}
				else if(button_pressed==DWN_BUT){
					if(volume_setting >= 0){
						volume_setting--;
						vs1002Config();			//Enable the MP3 Comm. Lines
						vs1002SetVolume(DECREASE);	//Lower the volume
						vs1002Finish();			//Disable MP3 Comm. Lines
					}
					LCDSetRowColor(2, 0, current_display->back_color, current_display->orientation);
					LCDPrintString("%d", volume_setting, white, 2, 0, current_display->orientation);
				}
				VICIntEnable |= 0x10;
			}
		}	
		else if(current_display->current_row==RADIOCMENU){
			LCDSetRowColor(2, 0, current_display->back_color, current_display->orientation);
			LCDPrintString("%d", radio_channel, white, 2, 0, current_display->orientation);
			button_pressed=NO_BUT;
			VICIntEnable |= 0x10;
			while(button_pressed < MID_BUT){
				VICIntEnClr = 0x10;	//Stop Interrupts to
				delay_ms(100);		//	debounce the switch					
				if(button_pressed==UP_BUT){
					//Increase Radio Channel
					if(radio_channel < 1075)radio_channel+=2;
					LCDSetRowColor(2, 0, current_display->back_color, current_display->orientation);
					LCDPrintString("%d", radio_channel, white, 2, 0, current_display->orientation);
				}
				else if(button_pressed==DWN_BUT){
					//Decrease Radio Channel
					if(radio_channel > 885)radio_channel-=2;
					LCDSetRowColor(2, 0, current_display->back_color, current_display->orientation);
					LCDPrintString("%d", radio_channel, white, 2, 0, current_display->orientation);
				}
				ns73SetChannel(radio_channel);
				VICIntEnable |= 0x10;
			}
			IOCLR1 |= FM_CS;				//Select the FM transmitter
			delay_ms(100);
			ns73Config();					//Configigure the FM Trans. I/O
			ns73SetChannel(radio_channel);	//Set the channel	
			IOSET1 |= FM_CS;				//Unselect the FM transmitter
		}
		else if(current_display->current_row==RADIOPMENU){
			LCDSetRowColor(2, 0, current_display->back_color, current_display->orientation);
			if(radio_enable)LCDPrintString("On", 0, current_display->text_color, 2,0,current_display->orientation);
			else LCDPrintString("Off", 0, current_display->text_color, 2,0,current_display->orientation);
			button_pressed=NO_BUT;
			VICIntEnable |= 0x10;
			while(button_pressed < MID_BUT){
				VICIntEnClr = 0x10;	//Stop Interrupts to
				delay_ms(100);		//	debounce the switch					
				if(button_pressed==UP_BUT){
					//Enable Radio
					radio_enable=ON;
					LCDSetRowColor(2, 0, current_display->back_color, current_display->orientation);
					LCDPrintString("On", 0, current_display->text_color, 2,0,current_display->orientation);
					
					IOCLR1 |= FM_CS;			//Select the FM transmitter
					delay_ms(100);
					ns73Config();				//Configigure the FM Trans. I/O
					ns73Send(R0, PE | AG);		//Power up the radio	
					IOSET1 |= FM_CS;			//Unselect the FM transmitter

				}
				else if(button_pressed==DWN_BUT){
					//Disable Radio
					radio_enable=OFF;
					LCDSetRowColor(2, 0, current_display->back_color, current_display->orientation);
					LCDPrintString("Off", 0, current_display->text_color, 2,0,current_display->orientation);

					IOCLR1 |= FM_CS;			//Select the FM transmitter
					delay_ms(100);
					ns73Config();				//Configigure the FM Trans. I/O
					ns73Send(R0, MUTE);		//Power up the radio	
					IOSET1 |= FM_CS;			//Unselect the FM transmitter	

				}
				VICIntEnable |= 0x10;
			}
		}
		if(file_is_open)quickClear(current_display);
		else LCDClear(current_display->back_color);
		printMenu(current_display);
	}
	VICIntEnable |= 0x10;
}


//Usage: quickClear(currentDisplay);
//Inputs: DisplayStruct *display - pointer to the display that contains the current display
//Outputs: None
//Description: Rather than painting every pixel a specific color (like LCDClear does) this routine
//			   will simply color all of the text the same color as the background, giving the illusion
//			   of a clear screen.  Only works with the background color.  Much faster than LCDClear, and is
//			   good to use while an MP3 is playing.
void quickClear(DisplayStruct *display){
	PINSEL0 &= 0xFFFFC00F;		//Hand over SPI lines to LCD talk
	//If we're on the first page of the menu, we also need to clear the title.
	if(display->current_page==0){
		LCDPrintString(display->title,0, display->back_color, 0,0,display->orientation);
		
	}
	LCDSetRowColor(display->current_row, 0, display->back_color, display->orientation);
	for(int j=0; j<NUMROWS; j++){
		LCDPrintString(display->list[j].file_name,0, display->back_color, j+1,0,display->orientation);
		LCDPrintString(newline,0, black, j+1,0,0);
	}
}

void reset(void)
{
    // Intentionally fault Watchdog to trigger a reset condition
    WDMOD |= 3;
    WDFEED = 0xAA;
    WDFEED = 0x55;
    WDFEED = 0xAA;
    WDFEED = 0x00;
}
