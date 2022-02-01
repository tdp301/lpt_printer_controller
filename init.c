
#include "global_includes.h"

#include "systemData.h"
#include "serial.h"


// PIC18F45K80 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1L
#pragma config RETEN = OFF      // VREG Sleep Enable bit (Ultra low-power regulator is Disabled (Controlled by REGSLP bit))
#pragma config INTOSCSEL = HIGH // LF-INTOSC Low-power Enable bit (LF-INTOSC in High-power mode during Sleep)
#pragma config SOSCSEL = DIG    // SOSC Power Selection and mode Configuration bits (Digital (SCLKI) mode)
#pragma config XINST = OFF       // Extended Instruction Set (Enabled)

// CONFIG1H
#pragma config FOSC = HS2       // Oscillator (HS oscillator (High power, 16 MHz - 25 MHz))
#pragma config PLLCFG = OFF     // PLL x4 Enable bit (Disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Disabled)
#pragma config IESO = OFF       // Internal External Oscillator Switch Over Mode (Disabled)

// CONFIG2L
#pragma config PWRTEN = OFF     // Power Up Timer (Disabled)
#pragma config BOREN = ON       // Brown Out Detect (Controlled with SBOREN bit)
#pragma config BORV = 3         // Brown-out Reset Voltage bits (1.8V)
#pragma config BORPWR = ZPBORMV // BORMV Power level (ZPBORMV instead of BORMV is selected)

// CONFIG2H
#pragma config WDTEN = ON       // Watchdog Timer (WDT controlled by SWDTEN bit setting)
#pragma config WDTPS = 1048576  // Watchdog Postscaler (1:1048576)

// CONFIG3H
#pragma config CANMX = PORTB    // ECAN Mux bit (ECAN TX and RX pins are located on RB2 and RB3, respectively)
#pragma config MSSPMSK = MSK7   // MSSP address masking (7 Bit address masking mode)
#pragma config MCLRE = ON       // Master Clear Enable (MCLR Enabled, RE3 Disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Overflow Reset (Enabled)
#pragma config BBSIZ = BB2K     // Boot Block Size (2K word Boot Block size)

// CONFIG5L
#pragma config CP0 = OFF        // Code Protect 00800-01FFF (Disabled)
#pragma config CP1 = OFF        // Code Protect 02000-03FFF (Disabled)
#pragma config CP2 = OFF        // Code Protect 04000-05FFF (Disabled)
#pragma config CP3 = OFF        // Code Protect 06000-07FFF (Disabled)

// CONFIG5H
#pragma config CPB = OFF        // Code Protect Boot (Disabled)
#pragma config CPD = OFF        // Data EE Read Protect (Disabled)

// CONFIG6L
#pragma config WRT0 = OFF       // Table Write Protect 00800-01FFF (Disabled)
#pragma config WRT1 = OFF       // Table Write Protect 02000-03FFF (Disabled)
#pragma config WRT2 = OFF       // Table Write Protect 04000-05FFF (Disabled)
#pragma config WRT3 = OFF       // Table Write Protect 06000-07FFF (Disabled)

// CONFIG6H
#pragma config WRTC = OFF       // Config. Write Protect (Disabled)
#pragma config WRTB = OFF       // Table Write Protect Boot (Disabled)
#pragma config WRTD = OFF       // Data EE Write Protect (Disabled)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protect 00800-01FFF (Disabled)
#pragma config EBTR1 = OFF      // Table Read Protect 02000-03FFF (Disabled)
#pragma config EBTR2 = OFF      // Table Read Protect 04000-05FFF (Disabled)
#pragma config EBTR3 = OFF      // Table Read Protect 06000-07FFF (Disabled)

// CONFIG7H
#pragma config EBTRB = OFF      // Table Read Protect Boot (Disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>






void init_pins()
{
    TRISA = 0xFF;
    TRISB = 0xFF;
    TRISC = 0xFF;
    TRISD = 0xFF;
    TRISE = 0xFF;
    
    INIT_HW_TRIS;
    
    OUT_A=1;
    OUT_B=1;
    
    //Analogs to digital
    ANCON1 = 0;
    ANCON0 = 0;
    
    CM1CON = 0;
    
    //clock config
    
    
    //Init timer 2. Used for timer 2 interrupt that runs program.
    T2CON = 0;
    T2CONbits.T2OUTPS = 0xF;       //1:16 postscale
    T2CONbits.T2CKPS = 2;       //1:16 prescale
    
    //Normal t2 tick is every 3.2 uS with 16 prescale
    // PR2 at 196 makes int time 10.0352 mS with 16 post
    PR2 = 196;
    TMR2 = 0;
    TMR2IF = 0;
    TMR2IE = 1;
    T2CONbits.TMR2ON = 1;   //timer 2 on
           
    //external int, ack pin
    INT1IE = 1;
    
    
    //Port B interrupt on change for up to b7:4
    IOCBbits.IOCB4 = 1; //ioc enabled
    IOCBbits.IOCB5 = 1; //ioc enabled
    IOCBbits.IOCB6 = 1;
    IOCBbits.IOCB7 = 1;
    INTCONbits.RBIF = 0;
    INTCONbits.RBIE = 1;
    
    
    //global int enable
    INTCONbits.GIE = 1;
    INTCONbits.GIEL = 1;
    
    init_serial();
    
    //pin enables
    LIMIT_SW_DRAIN = 1; //high. Note that this is left input for now.
    SET_CENT_INPUT;
    SET_CENT_HIGH;
    
    //Variable startups
    systemData.prnState = PRN_STATE_STARTUP;
    systemData.startupCounts = 0;
    
    //------------------------------------------------
//    // Init output levels (before switching tris)
//    LATA = 0;
//    LATB = 0;
//    
//    
//    init_system_data(); //Includes reading hwid, which may be needed in other perif setups
//    
//    //--------------- init tris
//    INIT_HW_TRIS;
//    
//    //---------------- init analog
//    ANSELA = 0;     //All digital
//    ANSELB = 0;     //All digital
//    
//
//    //Note: setting an analog to 1 only allows the route to get to the ADC. It doesn't actually configure the ADC to read the pin.
//   // INIT_HW_ANALOG_PINS;
//    
//    //Remap pins
//    _U2RXR = 0b0100001;  //  rpi33
//    _RP20R = 0b00011;  // uart2 transmit  rp20
//    
//    _INT1R = 0b0100000;     //rpi32
//    //_INT2R = 0b0100001;    //rpi33      
//    
//    LIMIT_SW_RELAY = 1;
//    LIMIT_SW_DRAIN = 1;
//    
//    _ODCB6 = 1;
//    
//    _ODCB12 = 1;
//    _ODCB13 = 1;
//    _ODCB14 = 1;
//    _ODCB15 = 1;
//    _ODCB11 = 1;
//    _ODCB9 = 1;
//    _ODCB5 = 1;
//    
//    _LATB12 =1;
//    _LATB13 =1;
//    _LATB14 =1;
//    _LATB15 =1;
//    _LATB11 =1;
//    _LATB9 = 1;
//    _LATB5 =1;
    
#ifdef USING_REV0_HW
    //Remap inputs
    RPINR26bits.C1RXR = CAN_RX_RPI;   //Can 1 receive
#warning "Was u1?"
    _U2RXR = UART_RX_RPI;   //UART 1 receive
    CAN_TX_RPO_PIN = 0b001110;  //Can 1 tx
//    UART_TX_RPO_PIN = 0b00001;    //Uart 1 tx
     UART_TX_RPO_PIN = 0b00011;    //Uart 2 tx
#endif
    
}
