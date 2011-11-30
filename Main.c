/*******************************************************************************
File:			Main.c

				Main WhereAVR function library.

Functions:	extern int	main(void)
				extern void mainTransmit(void)
				extern void mainReceive(void)
				extern void ax25rxByte(unsigned char rxbyte)
				extern void mainDelay(unsigned int timeout)
				extern void Delay(unsigned int timeout)
				SIGNAL(SIG_OVERFLOW0)
				SIGNAL(SIG_OVERFLOW1)
				SIGNAL(SIG_OVERFLOW2)
				SIGNAL(SIG_COMPARATOR)

Created:		1.00	10/05/04	GND	Gary Dion

Revisions:	1.01	11/02/04	GND	Major modification on all fronts
				1.02	12/01/04	GND	Continued optimization
				1.03	06/23/05	GND	Converted to C++ comment style and cleaned up

Copyright:	(c)2005, Gary N. Dion (me@garydion.com). All rights reserved.
				This software is available only for non-commercial amateur radio
				or educational applications.  ALL other uses are prohibited.
				This software may be modified only if the resulting code be
				made available publicly and the original author(s) given credit.

*******************************************************************************/

// OS headers
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/signal.h>

// General purpose include files
#include "StdDefines.h"

// App required include files
#include "ADC.h"
#include "ax25.h"
#include "Main.h"
#include "Messaging.h"
#include "Serial.h"

#define	RXSIZE (256)

#define	WatchdogReset() asm("wdr")		// Macro substitution to kick the dog

// Static functions and variables
volatile unsigned char delay;				// State of Delay function
volatile unsigned char maindelay;		// State of mainDelay function
static unsigned char	rxbytes[RXSIZE];	// Incoming character array
static unsigned char msg_start;			// Index of message start after header
static unsigned char	msg_end;				// Index of message ending character
static unsigned char	command;				// Used just for toggling
static unsigned char	transmit;			// Keeps track of TX/RX state
static unsigned char	rxtoggled;			// Signals frequency just toggled
static unsigned short crc;					// Current checksum for incoming message
static unsigned short dcd;					// Carrier detect of sorts
volatile char busy;							// Carrier detect of sorts

/******************************************************************************/
extern int	main(void)
/*******************************************************************************
* ABSTRACT:	Main program entry function.
*
* INPUT:		None
* OUTPUT:	None
* RETURN:	None
*/
{
	static unsigned short loop;			// Generic loop variable
	static unsigned short servo_loop;	// Generic loop variable
	static unsigned char	ones_seconds;	// Remembers tens digit of seconds
	static unsigned char seconds;			// Holds seconds calculated from GPS

	//Initialize serial communication functions
	ADCInit();
	SerInit();
	MsgInit();

	// PORT B - Sinewave Generation, and
	//	Bit/Pin 5 (out) connected to a 1k ohm resistor
	//	Bit/Pin 4 (out) connected to a 2k ohm resistor
	//	Bit/Pin 3 (out) connected to a 3.9k ohm resistor
	//	Bit/Pin 2 (out) connected to an 8.2k ohm resistor
	//	Bit/Pin 1 (out) connected to the PTT circuitry
	//	Bit/Pin 0 (out) DCD LED line
	PORTB = 0x00;							// Initial state is everything off
	DDRB  = 0x3F;							// Data direction register for port B

	// PORT D - General use port
	//	Bit/Pin 2 (out) connected the control line on a servo
	PORTD = 0x00;							// Initial state is everything off
	DDRD  = 0x04;							// Data direction register for port D

	// Initialize the Analog Comparator
	SFIOR = 0;								// Select AIN1 as neg. input
	ACSR = (1<<ACBG) | (1<<ACIE);		// Select Bandgap for pos. input

	//	Initialize the 8-bit Timer0 to clock at 1.8432 MHz
	TCCR0 = 0x02; 							// Timer0 clock prescale of 8

	// Use the 16-bit Timer1 to measure frequency; set it to clock at 1.8432 MHz
	TCCR1B = 0x02;							// Timer2 clock prescale of 8

	//	Initialize the 8-bit Timer2 to clock at 1.8432 MHz
	TCCR2 = 0x07; 							// Timer2 clock prescale of 1024

	// Enable Timer interrupts
	TIMSK = 1<<TOIE0 | 1<<TOIE2; // | 1<<TOIE2;

	// Enable the watchdog timer
	WDTCR	= (1<<WDCE) | (1<<WDE);		// Wake-up the watchdog register
	WDTCR	= (1<<WDE) | 7;				// Enable and timeout around 2.1s

	// Enable interrupts
	sei();

	// Reset watchdog
	WatchdogReset();

	// First move the servo to the starting position
	for (servo_loop = 0 ; servo_loop < 200 ; servo_loop++)
	{
		PORTD |= 0x04;	// servo signal on pin 4 PORT D2

//		Delay(5);			// 1.17 ms pulse - fully clockwise
		Delay(23);			// 1.51 ms pulse - center position - latch locked
//		Delay(32);			// 1.87 ms pulse - fully anticlockwise - release!!!!
		PORTD &= ~0x04;
		Delay(50);			// 18.0 ms between servo pulses
		WatchdogReset();
	}

	// Initialization complete - system ready.  Run program loop indefinitely.
	while (TRUE)
	{
//		txtone = SPACE;						// Debug tone for testing (MARK or SPACE)
//		while(1) WatchdogReset();			// Debug with a single one tone
//		while(1) ax25sendByte(0);			// Debug with a toggling tone
		Delay(250);
		Delay(250);
		Delay(250);
		Delay(250);
		Delay(250);
//		while(busy)	Delay(250);			// Wait for break (not on balloons!!!)
		MsgPrepare();							// Prepare variables for APRS position
		mainTransmit();						// Enable transmitter

		if (command == 0)						// Default message to be sent
		{
			MsgSendPos();						// Send Position Report and comment
		}
		else
		{
			if (command == 'S')
			{
				ax25sendEEPROMString(48);	// Send ">See garydion.com"
			}

			if (command == 'T')
			{
				MsgSendTelem();				// Send Telemetry and comment
			}

		}

		mainReceive();							// Disable transmitter
		command = 0;

		for (loop = 0 ; loop < 3900 ; loop++)	// Wait a maximum of 30 seconds
		{
			Delay(112);									// Delay is 10ms with 112
			if (msg_end)								// Meanwhile, if we get a packet
			{
				if (rxbytes[msg_start] == ':')		// Look for message character
				if (rxbytes[msg_start + 1] == 'N')	// Look for my callsign
				if (rxbytes[msg_start + 2] == '4')
				if (rxbytes[msg_start + 3] == 'T')
				if (rxbytes[msg_start + 4] == 'X')
				if (rxbytes[msg_start + 5] == 'I')
				if (rxbytes[msg_start + 6] == '-')
				if (rxbytes[msg_start + 7] == '1')	// And my SSID
				if (rxbytes[msg_start + 8] == '1')
				if (rxbytes[msg_start + 9] == ' ')
				if (rxbytes[msg_start + 10] == ':')
				{
					command = rxbytes[msg_start + 11];	// Grab command character
					if (rxbytes[msg_start + 12] == '{')	// Sender expects acknowledge
					{
						Delay(250);				// Pause
						Delay(250);
						Delay(250);
						Delay(250);
						Delay(250);
//						while(busy) Delay(250);			// Wait for clear channel
						mainTransmit();						// Enable transmitter
						MsgSendAck (rxbytes,msg_start);	// Send Ack text
						mainReceive();							// Disable transmitter
					}

					if (command == 'P')
					{
						loop = 32766;			// Exit timeout loop to transmit
					}

					if (command == 'R')		// "R" means release
					{
						for (servo_loop = 0 ; servo_loop < 200 ; servo_loop++)
						{
							PORTD |= 0x04;		// servo signal on pin 4 PORT D2
							Delay(32);			// latch released
							PORTD &= ~0x04;
							Delay(50);			// 18.0 ms between servo pulses
						}

					}

					if (command == 'L')		// "L" means lock
					{
						for (servo_loop = 0 ; servo_loop < 200 ; servo_loop++)
						{
							PORTD |= 0x04;		// servo signal on pin 4 PORT D2
							Delay(23);			// latch locked
							PORTD &= ~0x04;
							Delay(50);			// 18.0 ms between servo pulses
						}

					}

				}		// end if (rxbytes[msg_start + 10] == ':')

				msg_end = 0;					// Forget there was a message
				ACSR |= (1<<ACIE);			// Re-enable the comparator
			}		// end if (msg_end)

			// The following condition is true once per second.
			if (Time_Temp[5] != ones_seconds)	// Has ones digit changed?
			{
				ones_seconds = Time_Temp[5];		// Remember new ones digit

				// Convert 'seconds' from ascii to use in time slotting functions
				seconds = (Time_Temp[4] - 48) * 10 + Time_Temp[5] - 48;

				if (seconds == 5 || seconds == 35)	// Check for transmit slots
				{
					loop = 32766;						// Exit timeout loop to transmit
				}

				if (seconds == 42)
				{
					// do something else interesting
				}

			}

		}		// end for (loop = 0 ; loop < 3900 ; loop++)

	}

	return(1);

}		// End main


/******************************************************************************/
extern void mainTransmit(void)
/*******************************************************************************
* ABSTRACT:	Do all the setup to transmit.
*
* INPUT:		None
* OUTPUT:	None
* RETURN:	None
*/
{
	UCSRB &= ~((1<<RXCIE)|(1<<TXCIE));	// Disable the serial interrupts
	ACSR &= ~(1<<ACIE);						// Disable the comparator
	TCCR0 = 0x03; 								// Timer0 clock prescale of 64
	TCCR1B = 0x02;								// Timer1 clock prescale of 8
	TCCR2 = 0x02; 								// Timer2 clock prescale of 8
	transmit = TRUE;							// Enable the transmitter
	ax25sendHeader();							// Send APRS header
	return;

}		// End mainTransmit(void)


/******************************************************************************/
extern void mainReceive(void)
/*******************************************************************************
* ABSTRACT:	Do all the setup to receive or wait.
*
* INPUT:		None
* OUTPUT:	None
* RETURN:	None
*/
{
	ax25sendFooter();							// Send APRS footer
	transmit = FALSE;							// Disable transmitter
	PORTB &= 0x3D;								// Make sure the transmitter is disabled
	TCCR0 = 0x02; 								// Timer0 clock prescale of 8
	TCCR1B = 0x02;								// Timer1 clock prescale of 8
	TCCR2 = 0x07;								// Timer2 clock prescale of 1024
	ACSR |= (1<<ACIE);						// Re-enable the comparator
	UCSRB |= (1<<RXCIE)|(1<<TXCIE);		// Re-enable the serial interrupts
	MsgHandler(0);								// Reset the GPS decoding engine
	return;

}		// End mainReceive(void)


/******************************************************************************/
extern void ax25rxByte(unsigned char rxbyte)
/*******************************************************************************
* ABSTRACT:	This function calculates the crc of the incomming message.
*
* INPUT:		txbyte	The byte to transmit
* OUTPUT:	None
* RETURN:	None
*/
{
	static unsigned char		loop;			// Generic loop variable
	static unsigned short	lsb_int;		// LSBit of incoming byte
	static unsigned short	xor_int;		// Used for the IF statement

	for (loop = 0 ; loop < 8 ; loop++)	// Loop through all eight bits
	{
		lsb_int = rxbyte & 0x01;			// Set aside the least significant bit
		xor_int = crc ^ lsb_int;			// XOR lsb of CRC with the latest bit
		crc >>= 1;								// Shift 16-bit CRC one bit to the right
		if (xor_int & 0x0001)				// If XOR result from above has lsb set
		{
			crc ^= 0x8408;						// XOR the crc with magic number
		}

		rxbyte >>= 1;							// Shift the reference byte one bit right
	}

	return;

}		// End ax25rxByte(unsigned char rxbyte)


/******************************************************************************/
extern void mainDelay(unsigned char timeout)
/*******************************************************************************
* ABSTRACT:	This function sets "maindelay", programs the desired delay,
*				and takes care of incoming serial characters until it's cleared.
*
* INPUT:		None
* OUTPUT:	None
* RETURN:	None
*/
{
	maindelay = TRUE;							// Set the condition variable
	WatchdogReset();							// Kick the dog before we start
	TCNT0 = 255 - timeout;					// Set desired delay
	while(maindelay)
	{
		Serial_Processes();					// Do this until cleared by interrupt
	}

	return;

}		// End mainDelay(unsigned int timeout)


/******************************************************************************/
extern void Delay(unsigned char timeout)
/*******************************************************************************
* ABSTRACT:	This function sets "delay", programs the desired delay,
*				and takes care of incoming serial characters until it's cleared.
*
* INPUT:		None
* OUTPUT:	None
* RETURN:	None
*/
{
	delay = TRUE;							// Set the condition variable
	WatchdogReset();						// Kick the dog before we start
	TCNT2 = 255 - timeout;				// Set desired delay
	while(delay)
	{
		Serial_Processes();				// Do this until cleared by interrupt
	}

	return;

}		// End Delay(unsigned char timeout)


/******************************************************************************/
SIGNAL(SIG_OVERFLOW0)
/*******************************************************************************
* ABSTRACT:	This routine now decodes packets by sampling the state of
*				rxtoggled, and assembling a packet if toggles occur.  A checksum is
*				performed for validation, then msg_end is set to message size.
*
* INPUT:		None
* OUTPUT:	None
* RETURN:	None
*/
{
	static unsigned char sample_clock;	// Sample count since last sync
	static unsigned char next_sample;	// Sample on which to grab next bit
	static unsigned char last8bits;		// Last 8 bits received
	static unsigned char bit_count;		// Bits of the next incoming byte
	static unsigned char ones_count;		// Sequential ones (detect a stuff)
	static unsigned char start_temp;		// Msg starts at end of header
	static unsigned char	bytes_recd;		// Incoming byte index
	static unsigned char crcstate;		// State of message checksum check
	static unsigned char crchi;			// High byte of expected crc

	if (transmit)
	{
		maindelay = FALSE;					// Clear condition holding up mainDelay
		TCNT0 = 0;								// Make long as possible delay
	}
	else
	{
		TCNT0 = 69;								// Set sample rate to 9600 Hz.
 		if (dcd)									// If we are actively monitoring a signal
		{
			dcd--;								// Decrement the dcd timer
			busy = TRUE;

			if (rxtoggled)						// See if a tone toggle was recognized
			{
				if(ones_count != 5)			// Only process if NOT a bit stuff toggle
				{
					bit_count++;				// Increment bit counter
					last8bits >>= 1;			// Shift in a zero from the left
				}

				rxtoggled = FALSE;			// Clear toggle flag
				ones_count = 0;				// Clear number of sequential ones
				sample_clock = 0;				// Sync clock for bit sampling
				next_sample = 12;				// Grab next bit after 12 clicks
			}
			else
			{
				if (++sample_clock == next_sample)	// Time to grab next bit?
				{
					ones_count++;				// Increment ones counter since no toggle
					bit_count++;				// Increment bit counter
					last8bits >>= 1;			// Shift the bits to the right
					last8bits |= 0x80;		//  shift in a one from the left
					sample_clock = 0;			// Clear the clock for bit sampling
					next_sample = 8;			// Grab next bit 8 clicks from now
				}

			}	// end else for 'if (rxtoggled)'

			if (last8bits == 0x7E)			// If the last 8 bits match the ax25 flag
			{
				crc = 0xFFFF;					// Initialize the crc register
				bit_count = 0;					// Sync bit_count for an 8-bit boundary
				bytes_recd = 0;				// Point to start of message buffer
				start_temp = 0;				// Init message pointer as well
				crcstate = 0;					// Start out expecting crc after header
			}
			else
			{
				if (bit_count == 8)			// Just grabbed 8'th bit for a full byte
				{
					if ((start_temp) && (crcstate == 0)) // Ready for LSB of crc?
					{
						if (last8bits == ((crc ^ 0xFF) & 0xFF)) // And see it?
						{
							PORTB |= 1;						// Turn on the DCD LED
							crchi = (crc >> 8)^0xFF;	// Preserve MSB of crc
							crcstate = 1;					// Expect the MSB of crc next
						}

					}
					else if (crcstate)		 // Or looking for MSB of crc?
					{
						if (last8bits == crchi) 		// And it so happens to match
						{
							msg_start = start_temp; 	// Save message start
							msg_end = bytes_recd - 2;	// Save message end
							rxbytes[msg_end + 1] = 0;	// Null terminate string
							ACSR &= ~(1<<ACIE);			// Disable the comparator

						}

						crcstate = 0;			// Revert to looking for LSB
					}

					bit_count = 0;				// Reset bit counter
					ax25rxByte(last8bits);	// Update checksum
					if (!(msg_end))
						rxbytes[bytes_recd++] = last8bits;	// And stuff the byte

					if ((rxbytes[bytes_recd-1] == 0xF0) &&	// End of header bytes
						 (rxbytes[bytes_recd-2] == 0x03))	//  means end of header
							start_temp = bytes_recd;			// Remember location

				}		// end 'if (bit_count == 8)'

			}		// end else for 'if (last8bits == 0x7E)'

		}		// end 'if (dcd)'
		else
		{
			busy = FALSE;
			PORTB &= 0x3E;						// Turn off the DCD LED
		}		// end else for 'if (dcd)'

	}		// end else for 'if (!(transmit))'

}		// End SIGNAL(SIG_OVERFLOW0)



/******************************************************************************/
//SIGNAL(SIG_OVERFLOW1)
/*******************************************************************************
* ABSTRACT:	This function would handle the counter1 overflow interrupt.	Since
*				TCNT1 is used for for another process, we can't use this function.
*/
//{
//}		// End SIGNAL(SIG_OVERFLOW1)


/******************************************************************************/
SIGNAL(SIG_OVERFLOW2)
/*******************************************************************************
* ABSTRACT:	This function handles the counter2 overflow interrupt.
*				Counter2 is used to generate a sine wave using resistors on
*				Pins B5-B1. Following are the sixteen 4-bit sinewave values:
*							7, 10, 13, 14, 15, 14, 13, 10, 8, 5, 2, 1, 0, 1, 2, 5.
*				If in receive mode, the counter is pre-loaded with a long delay
*				and the delay variable is cleared.
*
*		 !!!Important!!! This code is -optimized- for the least # of clock cycles.
*				If you modify it, PLEASE be sure you know what you're doing!
*
* INPUT:		None
* OUTPUT:	None
* RETURN:	None
*/
{
	// This line is for if you followed the schematic:
	static char	sine[16] = {58,22,46,30,62,30,46,22,6,42,18,34,2,34,18,42};
	// This line is for if you installed the resistors in backwards order :-) :
//	static char	sine[16] = {30,42,54,58,62,58,54,42,34,22,10,6,2,6,10,22};
	static unsigned char sine_index;		// Index for the D-to-A sequence

	if (transmit)
	{
		++sine_index;							// Increment index
		sine_index &= 15;						// And wrap to a max of 15
		PORTB = sine[sine_index];			// Load next D-to-A sinewave value
		TCNT2 = txtone;						// Preload counter based on freq.
	}
	else
	{
		delay = FALSE;							// Clear condition holding up Delay
		TCNT2 = 0;								// Make long as possible delay
	}

}		// End SIGNAL(SIG_OVERFLOW2)


/******************************************************************************/
SIGNAL(SIG_COMPARATOR)
/*******************************************************************************
* ABSTRACT:	This function handles the comparator interrupt.  Interrupts are
*				disabled during TCNT1 access to prevent 16-bit corruption.
*				A 1200 tone is 768 counts and 2200 tone is 419 at 14.7456 MHz.
*
* INPUT:		None
* OUTPUT:	None
* RETURN:	None
*/
{
	static unsigned short	count;
	static unsigned char 	last;

	// Ideally, interrupts should be disabled when TCNT1 is read and written to.
	// This is because it's a 2-byte "atomic" operation. In this case, the extra
	// step was omitted to save cycles.  This -may- cause occasional packet loss.
	if (!(msg_end))							// If not waiting to progress a message
	{
		count = TCNT1;							// Read counts since last interrupt

		if (count > 542)						// Below 1700 Hz?	(542 @ 14.7456 MHZ)
		{
			TCNT1 = 0;							// Clear the counter
			if (last == SPACE)				// If the last tone detected was a SPACE
			{
				rxtoggled = TRUE;				// Toggle detected
				dcd = count;					// Set "random" timeout
			}

			last = MARK;						// MARK is detected
		}
		else if (count > 100)				// Ignore a frequency above 4.6 kHz
		{
			TCNT1 = 0;							// Clear the counter
			if (last == MARK)					// If the last tone detected was a SPACE
			{
				rxtoggled = TRUE;				// Toggle detected
				dcd = count;					// Set "random" timeout
			}

			last = SPACE;						// SPACE is detected
		}

	}

}		// End SIGNAL(SIG_COMPARATOR)
