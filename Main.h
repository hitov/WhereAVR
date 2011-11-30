/*******************************************************************************
File:			Main.h

				Main ATmega8 function library.

Version:		1.03

*******************************************************************************/

// external function prototypes
extern int	main(void);
extern void mainTransmit(void);
extern void mainReceive(void);
extern void ax25rxByte(unsigned char rxbyte);
extern void	mainDelay(unsigned char timeout);
extern void	Delay(unsigned char timeout);

