//********************************************************************
//
//				MP3 Player Constant Values
//
//********************************************************************
#define MAXFILENAMELEN 		30		//Defines the number of characters allowed in a filename
#define MAXBUFFERSIZE 		32		//The amount of bytes buffered from the MP3 file.  The FIFO buffer of the MP3 player is 32 bytes
#define	NUMFILESBUFFERED	15		//Number of filenames to buffer from the SD card per read
#define MAXDISPLEN			22		//Number of characters that will fit in one row of the LCD
#define	NUMROWS				15		//Number of rows available on the LCD (doesn't include the title)

#define DEBOUNCE_ACCELEROMETER 300
#define DEBOUNCE_SWITCH        100

//********************************************************************
//
//				MP3 Player Structures
//
//********************************************************************
//SongStruct: The song struct is used to store information about the currently selected song.
typedef struct{
	char name[MAXDISPLEN];				//Filename with the .mp3 removed	
	char file_name[MAXFILENAMELEN];		//Filename retrieved from the SD card
	struct fat16_file_struct* handle;	//File Handle read from the SD card
	int size;							//Size of the file in bytes
	char extension;						//Indicates the file type
	unsigned char data[MAXBUFFERSIZE];	//Holds the data to be sent to the MP3 player
} SongStruct;

//Filestruct: 
typedef struct{
	char file_name[MAXFILENAMELEN];
} FileStruct;

//DisplayStruct: Contains the necessary to generate and maintain a menu
typedef struct{
	char current_row,current_column;	//Keeps track of the current selection for the menu
	char orientation;					//Sets the orientation of the menu (ORIENTUP or ORIENTLEFT)
	char text_color, back_color;		//Define the color of the text and background
	char total_pages, current_page;		//Define how many pages the menu has, and which one is currently displayed
	int current_index;					//Defines which file in "list" is currently selected
	FileStruct list[NUMFILESBUFFERED];	//Array of character strings that are to be displayed for the menu
	char title[MAXDISPLEN];				//Name of the menu, displayed at the top of the screen.
} DisplayStruct;

//********************************************************************
//
//				MP3 Player Function Prototypes
//
//********************************************************************
void bootUp(void);
void reset(void);	
char getButton(void);
void getNewFiles(DisplayStruct *files, FileStruct *file_list);
char loadSongInfo(SongStruct *song, DisplayStruct *selected_song);
void closeSong(SongStruct *song);
void highlightRow(DisplayStruct *current_display, char direction);
void printMenu(DisplayStruct *current_display);
void fillSettings(DisplayStruct *menu);
void handleMiddleButton(void);
void handleUpButton(DisplayStruct *display, FileStruct *Files);
void handleDownButton(DisplayStruct *display, FileStruct *Files);
void quickClear(DisplayStruct *display);

// Flöre meddling...
// Radio handling
void initializeRadio(int);
void enableRadio(void);
void disableRadio(void);
void setRadioFrequency(int);
void changeRadioFrequency(int);
// void increaseRadioFrequency(int);
// void decreaseRadioFrequency(int);

// MP3 handling
void initializeMP3Player(void);
void startMP3Player(void);
void sendMP3Data(void);
void stopMP3Player(void);

// Volume handling
void raiseVolume(int n);
void lowerVolume(int n);

// SPI handling
void initializeSPI(void);
void selectSD(void);
void selectLCD(void);
void selectRadio(void);
void deselectRadio(void);

// USB handling
int isUSBConnected(void);

// LCD
void splashScreen(void);
void clearScreen(void);

// Timers
void initializeMP3Timer(void);
void initializeUITimer(void);

// Interrupt Handling
void initializeInterrupts(void);
void stopAllInterrupts(void);
void enableMP3Interrupt(void);
void disableMP3Interrupt(void);
void enableUIInterrupt(void);
void disableUIInterrupt(int t);
void exclusiveUIInterrupt(void);

// LEDs
void initializeLEDs(void);

// Menus
void displayRadioMenu(void);

// Accelerometer
void handleAccelerometer(void);
int wishesSettings(void);
int wishesFileManager(void);