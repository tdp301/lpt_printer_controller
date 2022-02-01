/*
 * File:   main.c
 * Author: Thomas
 *
 * Created on August 8, 2020, 11:23 AM
 */


#define GLOBAL
#include <xc.h>
#include "global_includes.h"
#include "serial.h"
#include "systemData.h"

unsigned long int globalTmr = 0;

unsigned int acks = 0;
unsigned int ackPending = 0;

//printerStateType prnState;

void main(void) {
    init_pins();
    
    while (1)
    {
        Nop();
        
        if (systemData.runSerialFlag == 1)
        {
            systemData.runSerialFlag = 0;
            send_serial();
            read_serial();
        }
    }
    return;
}

void resetState(void)
{
    systemData.startupCounts = 0;
    systemData.prnState = PRN_STATE_STARTUP;
    
    systemData.leftCounts = 0;
    systemData.rightCounts = 0;
    systemData.errCounts = 0;
    systemData.curDirection = DIRECTION_ERR;
}

inline void _assertLimitSwitch()
{
    LIMIT_SW_DRAIN = 0;
    LIMIT_SW_TRIS = 0;  //output
    systemData.limitSwDelay = 5;   //5 cycles on
    systemData.limitSwToggles++;
}

/**
 * General current logic:
 *  NOTE all interrupts for low priroity go to one vector!
 * 
 * T2 interrupt toggles every 10 ms
 * 
 * 1. If the limit switch delay is on, it will decrement and clear LIMIT_SW_TRIS 
 *   when zero. If for example limitSwDelay is set to 5, then the limit switch will
 *   be asserted then auto cleared at 50ms.
 * 
 * 2. set flag to refresh serial every X cycles (currently 10 = 100ms)
 * 
 * 3. IF int1 is set, set ack pending. currently disabled.
 * 
 * 4. If RBIF is set, then the port b has changed (change notification interrupt)
 *      B4 and B5 pins only 
 *      These come from a couple motor windings that go through an inverter. 
 *      Pins d7,d6 are the outgoing bits (OUT_A = D7 corresponds to b5)
 * 
 * Wiring:
 *  (red stripe) 
 *  <- Pin 14 (InB) --- D6 "OUT_B" --------------------------\  inv logic
 *  -> Signal for 14 -- Inverter --> 470 ohm --> B4 "IN B" --/   
 *  <- Pin 5 (InA) --- D7 "OUT_A" ---------------------------\  inv logic
 *  -> Signal for 5 -- Inverter --> 470 ohm ---> B5 "IN A" --/
 * 
 *  (white stripe)
 *  xx
 *  --> signal for 11 (TdB) -- B7 
 *  xx
 *  --> signal for 2 (TdA) --- B6
 * 
 *   Depending on prnState, the output may be relayed from the input. If in the "PRN_STATE_PRINTING"
 *    state, the output is only relayed if the head is moving right.
 */

void __interrupt(low_priority) t2Int(void)
{
    static unsigned long int counter = 0;
    
    if (TMR2IF == 1)
    {
        if (systemData.limitSwDelay > 0)
        {
            systemData.limitSwDelay--;
        }
        else if (systemData.limitSwDelay <= 0)
        {
            LIMIT_SW_TRIS = 1;  //in case was driving
        }

        TMR2IF = 0;

        if (counter ++ > 100)
        {
            counter = 0;

            STATUS_LED1 = !STATUS_LED1;
            globalTmr++;
        }
        
        if (counter % 10 == 0)
         {
            systemData.runSerialFlag = 1;
        }
    }
    
    
    //ack pin
    if (INT1IF == 1)
    {
        systemData.int1_ints++;
        INT1IF = 0;
        acks++;
        ackPending = 1;
    }
    
    //Note: expected in left direction toward stop (after inv)
    //  B5 (5)  0 1 1 0 0             
    //  B4 (14) 0 0 1 1 0              
    //          0 2 3 1 0
    // In right direction after inv
    //  B5  0 0 1 1 0       
    //  B4  0 1 1 0 0 
    //      0 1 3 2 0
    
    
    //Port B change
    if (INTCONbits.RBIF == 1)
    {
        INTCONbits.RBIF = 0;
        systemData.portb_ints ++;
        unsigned char currentPortB = (PORTB >> 4);// & 0xF;
        
        static unsigned char bufferIndex = 0;
        
        systemData.rollingInputBuffer[bufferIndex] = currentPortB;
        bufferIndex++;
        if(bufferIndex >= 32)
            bufferIndex = 0;
        
//        int errorFlag = 0;
        //directionType curDirection = DIRECTION_ERR;
        
        if (currentPortB != systemData.lastPortb)
        {
            switch (systemData.lastPortb)
            {
                case 0:
                    if (currentPortB == 2)
                        systemData.curDirection = DIRECTION_LEFT_TO_STOP;
                    else if (currentPortB == 1)
                        systemData.curDirection = DIRECTION_RIGHT_FROM_STOP;
                    break;
                case 1:
                    if (currentPortB == 0)
                        systemData.curDirection = DIRECTION_LEFT_TO_STOP;
                    else if (currentPortB == 3)
                        systemData.curDirection = DIRECTION_RIGHT_FROM_STOP;
                    break;
                case 2:
                    if (currentPortB == 3)
                        systemData.curDirection = DIRECTION_LEFT_TO_STOP;
                    else if (currentPortB == 0)
                        systemData.curDirection = DIRECTION_RIGHT_FROM_STOP;
                    break;                
                case 3:
                    if (currentPortB == 1)
                        systemData.curDirection = DIRECTION_LEFT_TO_STOP;
                    else if (currentPortB == 2)
                        systemData.curDirection = DIRECTION_RIGHT_FROM_STOP;
                    break;                
                default:
                    break;
            }   //end switch (last pin state)
            
            if (systemData.curDirection == DIRECTION_ERR)
            {
                systemData.errCounts++;
            }
            else
            {
                if (systemData.curDirection == DIRECTION_RIGHT_FROM_STOP)
                {
                    systemData.rightCounts++;
                    systemData.right_counts_p++;
                }
                else if (systemData.curDirection == DIRECTION_LEFT_TO_STOP)
                {
                    systemData.leftCounts++;
                    systemData.left_counts_p++;
                }
                

                //----------------------------------------------------
                // Limit sw control
                if (systemData.prnState == PRN_STATE_STARTUP)
                {
                    systemData.startupCounts ++;
                    if (systemData.startupCounts >= 50)
                    {
                        systemData.prnState = PRN_STATE_IDLE;

                        _assertLimitSwitch();
                    }
                }
            }   //end if NO ERROR
        }   //end if PINS CHANGED
        else
            systemData.bounceCounts++;

        //----------------------------------------------------
        // Output control
#define FWD_ONLY
        
#ifdef ALWAYS_RELAY
        //relay
        OUT_A = !IN_A;
        OUT_B = !IN_B;
#elif defined(NORMAL_DIR)
       if (systemData.prnState == PRN_STATE_IDLE)
        {
            //relay
            OUT_A = IN_A;
            OUT_B = IN_B;
        }
        else if (systemData.prnState == PRN_STATE_PRINTING)
        {

            if (systemData.curDirection != DIRECTION_LEFT_TO_STOP)
            {
                OUT_A = IN_A;
                OUT_B = IN_B;
            }
            else if (systemData.leftCounts > 260)
            {
                OUT_A = IN_A;
                OUT_B = IN_B;
            }

        } 
#elif defined(FWD_ONLY)
        // 1 3 2 0
        static int lastOutput = -1;
        int currentOutput = -1;
        
        //279 counts is normal span
        int commutate = (systemData.prnState == PRN_STATE_PRINTING);// &&(
//        ((systemData.curDirection == DIRECTION_LEFT_TO_STOP) && (systemData.left_counts_p > 100)) ||
//        ((systemData.curDirection == DIRECTION_RIGHT_FROM_STOP) && (systemData.right_counts_p <170)) );
        //(systemData.left_counts_p > 50) && (systemData.right_counts_p <100));
//        (systemData.left_counts_p > 200));
        
        
        if ((currentPortB != systemData.lastPortb) && commutate)
        {
            static int ping = 1;
//            ping = !ping;
            if (ping)
            {
                if (lastOutput == -1)
                {
                    currentOutput = currentPortB;
                }
                else
                {
                    switch(lastOutput)
                    {
                        case 0:
                            currentOutput = 1;
                            break;
                        case 1:
                            currentOutput = 3;
                            break;
                        case 2:
                            currentOutput = 0;
                            break;
                        case 3:
                            currentOutput = 2;
                            break;
                        default:
                            break;
                    }

                }
                //bit zero is b4, and bit one is b5
                // b5=d7 OUT_A bit 1, b4 d6 out_b bit 0
                OUT_A = (currentOutput & 0x2) == 0;
                OUT_B = (currentOutput & 0x1) == 0;
                
                lastOutput = currentOutput;
        
            }
            
            
        }   //end if (port b status changed)
#else
      ;  
#endif
        
        systemData.lastPortb = currentPortB; //inputs are on b4,5
    }
}
