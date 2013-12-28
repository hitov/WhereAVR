#ifndef _BOARD_H

/*Include board specific headers*/
#ifdef __AVR__
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#endif


/*GPIO definitions*/
#ifdef __AVR__
#define LM7001_PLL_ENABLE_DDR         (DDRB)
#define LM7001_PLL_ENABLE_PORT        (PORTB)
#define LM7001_PLL_ENABLE_BIT         (1)

#define LM7001_PLL_CLOCK_DDR          (DDRB)
#define LM7001_PLL_CLOCK_PORT         (PORTB)
#define LM7001_PLL_CLOCK_BIT          (0)

#define LM7001_PLL_DATA_DDR           (DDRB)
#define LM7001_PLL_DATA_PORT          (PORTB)
#define LM7001_PLL_DATA_BIT           (2)

#define LM7001_PLL_ENABLE_OUTPUT()    (LM7001_PLL_ENABLE_DDR  |=  (1 << LM7001_PLL_ENABLE_BIT))
#define LM7001_PLL_ENABLE_UP()        (LM7001_PLL_ENABLE_PORT |=  (1 << LM7001_PLL_ENABLE_BIT))
#define LM7001_PLL_ENABLE_DOWN()      (LM7001_PLL_ENABLE_PORT &= ~(1 << LM7001_PLL_ENABLE_BIT))

#define LM7001_PLL_CLOCK_OUTPUT()     (LM7001_PLL_CLOCK_DDR   |=  (1 << LM7001_PLL_CLOCK_BIT))
#define LM7001_PLL_CLOCK_UP()         (LM7001_PLL_CLOCK_PORT  |=  (1 << LM7001_PLL_CLOCK_BIT))
#define LM7001_PLL_CLOCK_DOWN()       (LM7001_PLL_CLOCK_PORT  &= ~(1 << LM7001_PLL_CLOCK_BIT))

#define LM7001_PLL_DATA_OUTPUT()      (LM7001_PLL_DATA_DDR    |=  (1 << LM7001_PLL_DATA_BIT))
#define LM7001_PLL_DATA_UP()          (LM7001_PLL_DATA_PORT   |=  (1 << LM7001_PLL_DATA_BIT))
#define LM7001_PLL_DATA_DOWN()        (LM7001_PLL_DATA_PORT   &= ~(1 << LM7001_PLL_DATA_BIT))


#define DCD_LED_DDR                   (DDRD)
#define DCD_LED_PORT                  (PORTD)
#define DCD_LED_BIT                   (7)

#define DCD_LED_ENABLE_OUTPUT()       (DCD_LED_DDR  |=  (1 << DCD_LED_BIT))
#define DCD_LED_ON()                  (DCD_LED_PORT &= ~(1 << DCD_LED_BIT))
#define DCD_LED_OFF()                 (DCD_LED_PORT |=  (1 << DCD_LED_BIT))
#define DCD_LED_TOGGLE()              (DCD_LED_PORT ^=  (1 << DCD_LED_BIT))


#define GPS_RFPWR_DDR                 (DDRD)
#define GPS_RFPWR_PORT                (PORTD)
#define GPS_RFPWR_BIT                 (2)

#define GPS_RFPWR_ON()                (GPS_RFPWR_DDR  |= (1 << GPS_RFPWR_BIT));  \
                                       (GPS_RFPWR_PORT &= ~(1 << GPS_RFPWR_BIT))

#define GPS_RFPWR_OFF()               (GPS_RFPWR_DDR  &= ~(1 << GPS_RFPWR_BIT)); \
                                       (GPS_RFPWR_PORT &= ~(1 << GPS_RFPWR_BIT))

#define APRS_PTT_DDR                  (DDRD)
#define APRS_PTT_PORT                 (PORTD)
#define APRS_PTT_BIT                  (3)

#define APRS_PTT_OUTPUT()             (APRS_PTT_DDR  |=  (1 << APRS_PTT_BIT))
#define APRS_PTT_ON()                 (APRS_PTT_PORT |=  (1 << APRS_PTT_BIT))
#define APRS_PTT_OFF()                (APRS_PTT_PORT &= ~(1 << APRS_PTT_BIT))

#define APRS_FSK_DDR                  (DDRC)
#define APRS_FSK_PORT                 (PORTC)
#define APRS_FSK_MASK                 (0x0F)
#define APRS_FSK_OUTPUT()             (APRS_FSK_DDR  |=  APRS_FSK_MASK)
#define APRS_FSK_OUT(value)           (APRS_FSK_PORT = value)

    // PORT C - Sinewave Generation, and
    //  Bit/Pin 0 (out) connected to a 1k ohm resistor
    //  Bit/Pin 1 (out) connected to a 2k ohm resistor
    //  Bit/Pin 2 (out) connected to a 3.9k ohm resistor
    //  Bit/Pin 3 (out) connected to an 8.2k ohm resistor
#define APRS_FSK_SINE_TABLE           {7, 10, 13, 14, 15, 14, 13, 10, 8, 5, 2, 1, 0, 1, 2, 5}


/*      * * * THIS IS WHERE THE CALLSIGNS ARE DETERMINED * * *
    Each callsign character is shifted to use the high seven bits of the byte.
    Use the table below to determine the hex values for these characters.
    For callsigns less than six digits, pad the end of the callsign with spaces.
    For all bytes in header, except for the very last byte, bit0 must be clear.
    Yes, this means only the very last byte has bit0 set to 1 in the Station ID!

        Callsign byte lookup table
         -----------------------------------------------------------
        | Letters:                    |   Numbers and SSID's:       |
        | A = 0x82       N = 0x9C     |   0 = 0x60       8 = 0x70   |
        | B = 0x84       O = 0x9E     |   1 = 0x62       9 = 0x72   |
        | C = 0x86       P = 0xA0     |   2 = 0x64       10 = 0x74  |
        | D = 0x88       Q = 0xA2     |   3 = 0x66       11 = 0x76  |
        | E = 0x8A       R = 0xA4     |   4 = 0x68       12 = 0x78  |
        | F = 0x8C       S = 0xA6     |   5 = 0x6A       13 = 0x7A  |
        | G = 0x8E       T = 0xA8     |   6 = 0x6C       14 = 0x7C  |
        | H = 0x90       U = 0xAA     |   7 = 0x6E       15 = 0x7E  |
        | I = 0x92       V = 0xAC     |   Space = 0x40              |
        | J = 0x94       W = 0xAE     |                             |
        | K = 0x96       X = 0xB0     |   REMEMBER!  Set bit0 in    |
        | L = 0x98       Y = 0xB2     |   the last SSID to one!     |
        | M = 0x9A       Z = 0xB4     |                             |
         -----------------------------------------------------------
        End of lookup table */

#define APRS_HEADER \
{ \
    /* Begin transmission of packet destination address (APxxxx0)*/ \
    0x82, /* A */ \
    0xA0, /* P */ \
    0x82, /* A */ \
    0xAC, /* V */ \
    0xA4, /* R */ \
    0x60, /* 0 */ \
    0x60, /* SSID=0 */ \
    /* Begin transmission of packet source address */ \
    0x98, /* Byte 1 (L) */ \
    0xB4, /* Byte 2 (Z) */ \
    0x62, /* Byte 3 (1) */ \
    0x82, /* Byte 4 (A) */ \
    0xA4, /* Byte 5 (R) */ \
    0x9A, /* Byte 6 (M) */ \
    0x76, /* Station ID (11) */ \
    \
    0xA4, /* Byte 1 (R) */ \
    0x8A, /* Byte 2 (E) */ \
    0x98, /* Byte 3 (L) */ \
    0x82, /* Byte 4 (A) */ \
    0xB2, /* Byte 5 (Y) */ \
    0x40, /* Byte 6 (Space) */ \
    0x60, /* Station ID (0) */ \
                               \
    0xAE, /* Byte 1 (W) */ \
    0x92, /* Byte 2 (I) */ \
    0x88, /* Byte 3 (D) */ \
    0x8A, /* Byte 4 (E) */ \
    0x64, /* Byte 5 (2) */ \
    0x40, /* Byte 6 (Space) */ \
    0x65, /* Station ID (2) */ \
    /* Finish out the header with two more bytes */ \
    0x03, /* Control field - 0x03 is APRS UI-frame */ \
    0xF0,  /* Protocol ID - 0xF0 is no layer 3 */ \
    0x00  \
}

#else
#define LM7001_PLL_ENABLE_OUTPUT()
#define LM7001_PLL_ENABLE_UP()
#define LM7001_PLL_ENABLE_DOWN()

#define LM7001_PLL_CLOCK_OUTPUT()
#define LM7001_PLL_CLOCK_UP()
#define LM7001_PLL_CLOCK_DOWN()

#define LM7001_PLL_DATA_OUTPUT()
#define LM7001_PLL_DATA_UP()
#define LM7001_PLL_DATA_DOWN()
#endif

/*Delays definitions*/
#ifdef __AVR__
#define delay_us(TIME)  _delay_us(TIME);
#define delay_ms(TIME) _delay_ms(TIME)
#define dbg_printf
#else
#define delay_us(TIME)
#define delay_ms(TIME)
#define dbg_printf printf
#endif

#endif

