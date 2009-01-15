//Include definitions of LCD_DIO, LCD_SCK, LCD_RES and LCD_CS


void disp_init(void);
void disp_cmd(unsigned char data);
void disp_data(unsigned char data);
void pset(unsigned char color, unsigned char x, unsigned char y);
unsigned char print_char(char txt, char text_color, char *x_pos, char *y_pos, char inverted);
void clear_screen(char color);
void print_logo(void);
void print_string(const char *txt, int variable, char text_color, char row, char column, char inverted);
//void print_message(void);
void fit_msg(void);
void setRowColor(unsigned char row, unsigned char column, char color, char inverted);
void lcdContrast(char setting);

//LCD Dimensions
#define ROW_LENGTH	132
#define COL_HEIGHT	132

//LCD Commands
#define	NOP			0x00
#define	BSTRON		0x03
#define SLEEPIN     0x10
#define	SLEEPOUT	0x11
#define	NORON		0x13
#define	INVOFF		0x20
#define INVON      	0x21
#define	SETCON		0x25
#define DISPOFF     0x28
#define DISPON      0x29
#define CASET       0x2A
#define PASET       0x2B
#define RAMWR       0x2C
#define RGBSET	    0x2D
#define	MADCTL		0x36
#define	COLMOD		0x3A
#define DISCTR      0xB9
#define	EC			0xC0


#define ENDPAGE     132
#define ENDCOL      130
