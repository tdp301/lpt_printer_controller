/* 
 * File:   hardware_profile.h
 * Author: thomas
 *
 * Created on October 29, 2017, 4:58 PM
 */

#ifndef HARDWARE_PROFILE_H
#define	HARDWARE_PROFILE_H



#define OUTPUT_PIN 0
#define INPUT_PIN 1

    #define INIT_HW_TRIS { \
        TRISD2 = OUTPUT_PIN;      /* #1 led 2 */      \
        TRISD7 = OUTPUT_PIN;      /* OUTA */      \
        TRISD6 = OUTPUT_PIN;      /* OUTB */      \
    }

#define OUT_A LATD7 //relay b5
#define OUT_B LATD6 //relay B4
#define IN_A PORTBbits.RB5
#define IN_B PORTBbits.RB4

    //Pin connections:
    // mainly output:
    //  D0 - D7 : [Ao : A5][E0:E1]
    //  STR C0
    // input
    //  ACK C4 int1
    //  Busy C2

    #define BUSY_IN PORTCbits.RC2

    #define SET_CENT_INPUT {    \
         TRISA0 = INPUT_PIN;    \
         TRISA1 = INPUT_PIN;    \
         TRISA2 = INPUT_PIN;    \
         TRISA3 = INPUT_PIN;    \
         TRISE2 = INPUT_PIN;    \
         TRISA5 = INPUT_PIN;    \
         TRISE0 = INPUT_PIN;    \
         TRISE1 = INPUT_PIN;    \
         TRISD1 = INPUT_PIN;    \
    }

    #define SET_CENT_OUTPUT {    \
         TRISA0 = OUTPUT_PIN;    \
         TRISA1 = OUTPUT_PIN;    \
         TRISA2 = OUTPUT_PIN;    \
         TRISA3 = OUTPUT_PIN;    \
         TRISE2 = OUTPUT_PIN;    \
         TRISA5 = OUTPUT_PIN;    \
         TRISE0 = OUTPUT_PIN;    \
         TRISE1 = OUTPUT_PIN;    \
         TRISD1 = OUTPUT_PIN;    \
    }


    #define SET_CENT_HIGH {    \
         LATA0 = 1;    \
         LATA1 = 1;    \
         LATA2 = 1;    \
         LATA3 = 1;    \
         LATE2 = 1;     \
         LATA5 = 1;    \
         LATE0 = 1;    \
         LATE1 = 1;    \
         LATD1 = 1;    \
    }
//#define SET_CENT_LOW {    \
//         LATA0 = 0;    \
//         LATA1 = 0;    \
//         LATA2 = 0;    \
//         LATA3 = 0;    \
//         LATE2 = 0;     \
//         LATA5 = 0;    \
//         LATE0 = 0;    \
//         LATE1 = 0;    \
//         LATD1 = 0;    \
//    }

    #define LIMIT_SW_DRAIN LATD0
    #define LIMIT_SW_TRIS TRISD0

    //outputs
    #define STATUS_LED1 LATD2


#endif	/* HARDWARE_PROFILE_H */

