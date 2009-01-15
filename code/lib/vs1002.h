/* *********************************************************
               Function declarations
  ********************************************************* */
void vs1002Init(void);		
unsigned short int vs1002SCIRead(unsigned char);
void vs1002SCIWrite(unsigned char, unsigned short int);
void vs1002SendMusic(unsigned char*, int);
void vs1002SineTest(unsigned char);
void vs1002Config(void);
void vs1002Finish(void);
void vs1002Mute(void);
void vs1002SetVolume(char setting);
void vs1002Reset(void);

/* *********************************************************
              MACRO definitions
  ********************************************************* */
#define SELECT_MP3_SCI() IOCLR0 = MP3_XCS
#define UNSELECT_MP3_SCI() IOSET0 = MP3_XCS
#define SELECT_MP3_SDI() IOCLR1 = MP3_XDCS
#define UNSELECT_MP3_SDI() IOSET1 = MP3_XDCS
#define READ_COMMAND 0x03
#define WRITE_COMMAND 0x02

/* *********************************************************
              General definitions
  ********************************************************* */
#define EXTERNAL_FREQ 13000000

/* *********************************************************
              vs1002 Registers
  ********************************************************* */
#define SCI_MODE	0x00
#define SCI_STATUS	0x01
#define SCI_BASS	0x02
#define SCI_CLOCKF	0x03
#define SCI_DECODE_TIME	0x04
#define SCI_AUDATA	0x05
#define SCI_WRAM	0x06
#define SCI_WRAMADDR	0x07
#define SCI_HDAT0	0x08
#define SCI_HDAT1	0x09
#define SCI_AIADDR	0x0A
#define SCI_VOL		0x0B
#define SCI_AICTRL0	0x0C
#define SCI_AICTRL1	0x0D
#define SCI_AICTRL2	0x0E
#define SCI_AICTRL3	0x0F

/* *********************************************************
              vs1002 Bit Masks
  ********************************************************* */
//SCI_MODE Bits	
 #define SM_DIFF		0x0001
 #define SM_SETTOZERO	0x0002
 #define SM_RESET		0x0004
 #define SM_OUTOFWAV	0x0008
 #define SM_PDOWN		0x0010
 #define SM_TESTS		0x0020
 #define SM_STREAM		0x0040
 #define SM_PLUSV		0x0080
 #define SM_DACT		0x0100
 #define SM_SDIORD		0x0200
 #define SM_SDISHARE	0x0400
 #define SM_SDINEW		0x0800
 #define SM_ADPCM		0x1000
 #define SM_ADPCM_HP	0x2000
 
 //SCI_STATUS Bits
 #define SS_VER			0x0070		//Use this as a mask for the Version when reading the Status Register
 //The rest of the Status bits are to be used by the vs1002 FW only
 
 //SCI_CLOCKF Bits
 #define SC_DOUBLECLK	0x8000
 #define SC_INCLK		EXTERNAL_FREQ/2000
 
 //SCI_VOL Settings
 #define 	INCREASE		'1'
 #define 	DECREASE		'0'
 #define 	SV_MUTE			0xFFFF
 #define	SV_MAX_VOLUME	0x0000

