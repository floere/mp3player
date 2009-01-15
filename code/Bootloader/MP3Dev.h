void delay_ms(int count);

//*******************************************************
//					GPIO Definitions
//*******************************************************
//#define MP3_XRES	0x00000004		//Define P0.2
#define MP3_XRES	0x01000000		//For Test Only!! (P1.24)
//#define FM_LA		0x00000008		//Define P0.3
#define FM_LA		0x08000000		//For Test Only!! (P1.27)
#define LCD_SCK		0x00000010		//Define P0.4 (Used when not in SPI Mode)
#define LCD_DIO		0x00000040		//Define P0.6 (Used when not in SPI Mode)
#define FM_SCLK		0x00000010		//Define P0.4 (Used when not in SPI Mode)
#define FM_DIO		0x00000040		//Define P0.6 (Used when not in SPI Mode)
#define SD_CS		0x00000080		//Define P0.7
#define GS2			0x00000400		//Define P0.10
#define GS1			0x00000800		//Define P0.11
#define FM_TEB		0x00010000		//Define P0.16
#define LCD_RES 	0x00040000		//Define P0.18
#define LCD_CS		0x00080000		//Define P0.19
#define MP3_XCS		0x00100000		//Define P0.20
#define MP3_DREQ	0x00200000		//Define P0.21
#define	VBUS		0x00800000		//Define P0.23
#define LED_RED		0x10000000		//Define P0.28
#define LED_BLU		0x20000000		//Define P0.29
#define LED_GRN		0x40000000		//Define P0.30
#define MP3_XDCS	0x00010000		//Define P1.16
#define MP3_GPIO0	0x00020000		//Define P1.17
#define MP3_GPIO1	0x00040000		//Define P1.18
#define MP3_GPIO2	0x00080000		//Define P1.19
#define MP3_GPIO3	0x00100000		//Define P1.20
#define SW_UP		0x00200000		//Define P1.21
#define SW_MID		0x00400000		//Define P1.22
#define SW_DWN		0x00800000		//Define P1.23

//*******************************************************
//				Special Function Pin Definitions
//*******************************************************
#define MMA_X_PINSEL    0x03000000	//Select ADC Function for P0.12
#define MMA_Y_PINSEL    0x0C000000	//Select ADC Function for P0.13
#define MMA_Z_PINSEL    0xC0000000	//Select ADC Function for P0.15
#define MP3RX_PINSEL	0x00010000	//Select UART1 Function for P0.8
#define MP3TX_PINSEL	0x00040000	//Select UART1 Function for P0.9
#define SCLK_PINSEL		0x00000100	//Select SPI Function for P0.4
#define MISO_PINSEL		0x00000400	//Select SPI Function for P0.5
#define MOSI_PINSEL		0x00001000	//Select SPI Function for P0.6

//*******************************************************
//					General Definitions
//*******************************************************
#define X_CHANNEL		0x00000008	//Select AD1.3
#define Y_CHANNEL		0x00000010	//Select AD1.4
#define Z_CHANNEL		0x00000020	//Select AD1.5

//Menus
#define	VOLUMEMENU		0
#define	CALMENU			1
#define	CONTMENU		1
#define	RADIOMENU		2


#define	NEXT			0
#define	PREVIOUS		1

#define OFF				0
#define ON				1

#define ORIENTUP			0
#define ORIENTLEFT			1


#define SONG_BYTES_BUFFERED	4		//Send 32 bits to the MP3 player
#define	UP_BUT	2
#define	DWN_BUT	1
#define	MID_BUT	3
#define NO_BUT	0

//*******************************************************
//					Global Macros
//*******************************************************
#define ledBlueOff()	IOSET0 = LED_BLU;
#define ledBlueOn()		IOCLR0 = LED_BLU;

#define ledRedOff()		IOSET0 = LED_RED;
#define ledRedOn()		IOCLR0 = LED_RED;

#define ledGrnOff()		IOSET0 = LED_GRN;
#define ledGrnOn()		IOCLR0 = LED_GRN;
