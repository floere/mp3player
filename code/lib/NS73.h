//********************************************************************
//
//				General Function Definitions
//
//********************************************************************
void ns73Config(void);
void ns73Init(void);
void ns73SerialReset(void);
void ns73SetChannel(int radio_channel);
unsigned char ns73Send(unsigned char outgoing_address, unsigned char outgoing_data);

//********************************************************************
//
//					Macro Definitions
//
//********************************************************************
#define UNLATCH	IOCLR1 = FM_LA
#define LATCH	IOSET1 = FM_LA

#define CLKDWN	IOCLR1 = FM_SCLK //Toggle the SPI clock
#define CLKUP	IOSET1 = FM_SCLK

//*******************************************************
//					Register Definitions
//*******************************************************
#define R0	0x0
#define R1	0x1
#define R2	0x2
#define R3	0x3
#define R4	0x4
#define R5	0x5
#define R6	0x6
#define R7	0x7
#define R8	0x8
#define R9	0x9
#define R10	0xA
#define R11	0xB
#define R14	0xE

//*******************************************************
//					Bit Definitions
//*******************************************************
//R14 Bits
#define	R14_RESET	0x05

//R1 Bits
#define R1_DEFAULT	0xB4
#define R1_PILOT	0x08
#define R1_SUBC		0x40

//R2 Bits
#define R2_DEFAULT	0x06
#define R2_ULD		0x04

//R8 Bits
#define R8_DEFAULT	0x1A

//R0 Bits
#define PE			0x01
#define PDX			0x02
#define MUTE		0x04
#define EM			0x10
#define EMS			0x20
#define AG			0x80

//R6 Bits
#define	R6_DEFAULT	0x1E
