/*
    SPI Communication for LPC2148 SPI0.
	2/1/2008
	Ryan Owens


*/

#include "spi0.h"
#include "LPC214x.h"
#include <stdio.h>

void SPI0_send(char c)
{
    SPI0_send_recv(c);
}

char SPI0_recv(void)
{
    return SPI0_send_recv(0xff);
}

char SPI0_send_recv(char c)
{
    char in;

    S0SPDR = c;						//Place data to be sent into SPI data register
    while(!(S0SPSR & SPIF));		//Wait for transfer to complete
	in=S0SPSR;
    in = S0SPDR&0xff;				//Return the character placed in the SPI data register by the slave
    
	return in;
}

