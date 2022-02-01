/* 
 * File:   systemData.h
 * Author: thomas
 *
 * Created on October 30, 2017, 7:39 PM
 */

#ifndef SYSTEMDATA_H
#define	SYSTEMDATA_H

typedef enum
{
    SYS_LED1,
   
            
            NUMBER_OF_LEDS, //KEEP LAST
} boardLed;

typedef enum
{
    LED_STATE_OFF,
    LED_STATE_ON,
    LED_STATE_TOGGLING,         //Determined by toggleRateHz
    LED_STATE_CONSTANT_PWM,     //Determined by pwmPct
    LED_STATE_PULSING,          //Determined by toggleRateHz
}ledStateType;

typedef struct
{
    ledStateType state;
    float toggleRateHz; //E.g. if 1, then 1s off 1s on. Only applicable in toggling mode.
    float pwmPct;       //0 to 100
} ledType;

typedef struct
{
    int lastState;
    int ticksSinceToggle;
    int increasing;
} ledInternalData;

#define uint32_t unsigned long
#define uint16_t unsigned short
#define U8 unsigned char


typedef enum
{
    PRN_STATE_STARTUP = 0,
    PRN_STATE_IDLE,
            PRN_STATE_PRINTING,
} printerStateType;

typedef enum
{
    DIRECTION_LEFT_TO_STOP,
    DIRECTION_RIGHT_FROM_STOP,
    DIRECTION_ERR,
} directionType;

typedef struct
{
//    struct {
//        unsigned int s;
//        unsigned int m;
//        unsigned int ms;
//        unsigned int h;
//    } tmr;
//    
//    ledType led[NUMBER_OF_LEDS];    //Leds controlled by OS
//    ledInternalData ledData[NUMBER_OF_LEDS];
//
//    uint32_t extInts;
//    uint32_t extInts2;
//    
//    uint32_t pin2counts_atSw;
//    
//    volatile uint16_t limitSwCountdown;
     
        uint16_t int1_ints;
        uint16_t portb_ints;
        volatile uint16_t lastPortb;
        uint16_t leftCounts;        //toward stop
        uint16_t rightCounts;       //toward ctr
        uint16_t left_counts_p;
        uint16_t right_counts_p;
        uint16_t errCounts;
        uint16_t bounceCounts;
        
        U8 runSerialFlag;
        U8 limitSwDelay;
        U8 limitSwToggles;
        
        U8 rollingInputBuffer[32];
        
        volatile printerStateType prnState;
        volatile uint16_t startupCounts;
        
        U8 printCycles;
        volatile directionType curDirection;
} SYSTEM_DATA_TYPE;

#ifdef GLOBAL
    SYSTEM_DATA_TYPE systemData;
#else
    extern SYSTEM_DATA_TYPE systemData;
#endif

#endif	/* SYSTEMDATA_H */

