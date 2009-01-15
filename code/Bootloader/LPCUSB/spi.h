#include "type.h"

#define SPI_PRESCALE_MIN  8

void	SPIInit(void);
void	SPISetSpeed(U8 speed);

U8		SPISend(U8 outgoing);
void	SPISendN(U8 *pbBuf, int iLen);
void	SPIRecvN(U8 *pbBuf, int iLen);

