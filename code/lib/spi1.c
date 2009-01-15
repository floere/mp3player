/*
    10/22/07 : NES

    Looks like low level SPI bus routines.

*/

#include "spi1.h"
#include "LPC214x.h"
#include <stdio.h>

void SPI1_Init(void)
{
    PINSEL1 |= (0x02 << 6) | (0x02 << 4) | (0x02 << 2); /* Configure IO for SPI1 Mode */
    IODIR0 |= (1 << 20); /* P0.20 set to output */
    IOCLR0 = (1<<20); /* Disable SSEL (LIPO Module select is high) */

    SSPCR0 =  (0x07 << 0)/* 8 bit transfers */
    | (0 << 4)   /* SPI Mode */
    | (0 << 6)   /* CPOL = 0 */
    | (0 << 7)   /* Cphase = 0 */
    | (59 << 8); /* Divide clock by 60 */
    SSPCPSR = 50; /* Div clock by 5 (200khz total) */
    SSPCR1 = (1 << 1); /* Enable the module */

    int blah;
    blah = SSPDR; //GCC warning clear

    while( (SSPSR & (1<<1)) == 0 );
}

void SPI1_send(char c)
{
    SPI1_send_recv(c);
}

char SPI1_recv(void)
{
    return SPI1_send_recv(0xff);
}

char SPI1_send_recv(char c)
{
    int in;

    SSPDR = c;

    while( SPI1_transfer_pend());

    in = SSPDR&0xff;
    return in;
}

void SPI1_ssel(int blah)
{
    if(blah)
        IOSET0 = (1<<20);
    else
        IOCLR0 = (1<<20);
}

/* Returns 0 if no transfer pending
 * Bit 0 indicates tx not empty
 * Bit 1 indicates rx empty
 */
int SPI1_transfer_pend(void)
{
    int result = 0;
    if( ((SSPSR & (1<<0)) == 0) )
    {
        result += 1;
    }   /* if the Transmitter is full */
    if( ((SSPSR & (1<<2)) == 0 ) )   /* or the receiver is empty */
    {
        result += 2;
    }
    return result;
}

char SPI1_get_rx(void)
{
    while( (SSPSR & (1 << 2)) == 0 );
    return (SSPDR&0xff);
}

void SPI1_send_noblock(char c)
{
    SSPDR = c;
}
