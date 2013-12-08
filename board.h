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
#define APRS_FSK_SINE_TABLE           {15, 20, 25, 28, 30, 28, 25, 20, 15, 9, 4, 1, 0, 1, 4, 9}



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

