void SPI1_Init(void);
void SPI1_send(char c);
char SPI1_recv(void);
char SPI1_send_recv(char c);
void SPI1_ssel(int blah);

int SPI1_transfer_pend(void);
char SPI1_get_rx(void);
void SPI1_send_noblock(char c);

#define UNRESET 1
#define RESET 0
