
#include "global_includes.h"
#include <string.h>
#include "systemData.h"

int serialScreen = 0;


void init_serial(void)
{
    TXSTA1 = 0;

    TXSTA1bits.TXEN = 1; //enable tx
    TXSTA1bits.BRGH = 1; //high speed ?
    
    RCSTA1 = 0;
    RCSTA1bits.SPEN = 1; //serial port enabled
    RCSTA1bits.CREN = 1; //Continuous rx enable ?
    
    //baud con
    BAUDCON1bits.BRG16 = 1;  //16 bit baud gen
    //baud = fosc/[4(n+1)]
    SPBRGH1 = 0;
    SPBRG1 = 42;
}

void send_string(char * buf)
{
    uint16_t i = 0;
    
    while ( (buf[i] != '\0') && (i<256))
    {
        TXREG = buf[i];
        Nop();
        Nop();
        while (TXSTA1bits.TRMT == 0)
        {
            Nop();
        }
        i++;
    }
}


extern unsigned long int globalTmr;

extern unsigned int acks;
extern unsigned int ackPending;

void send_serial(void)
{
    char serialBuf[256];
    char rowBuf[32];
    
    static int errCtr = 0;
    
    if (TXSTA1 == 0)
    {
        errCtr++;
        return;
    }
    
    serialBuf[0] = 0x1B;
    serialBuf[1] = '[';
    serialBuf[2] = 'H';
    serialBuf[3] = 0;

    static int clearCtr = 0;    
    
    //every 4th time, clear screen
    if (++clearCtr >= 4)
    {
        clearCtr = 0;
        strcat(serialBuf, "\x1B[J");
    }

    
    strcat(serialBuf, "Printer test brd " __DATE__ " " __TIME__);
    
    sprintf(rowBuf, ", up %lu \r\n", globalTmr );
    strcat(serialBuf, rowBuf);
    
    int i;
    
    switch (serialScreen)
    {
        case 0:
            strcat(serialBuf, "0- This screen\r\n1- Genl status\r\n2- buffer\r\n3- print stat\r\n");
            break;
        case 1:
            strcat(serialBuf, "General status:\r\n");
            sprintf(rowBuf, "INT1 ints %u, state %u \r\n", systemData.int1_ints, systemData.prnState );
            strcat(serialBuf, rowBuf);
            sprintf(rowBuf, "portb ints %u, last %u \r\n", systemData.portb_ints , systemData.lastPortb);
            strcat(serialBuf, rowBuf);
            sprintf(rowBuf, "limit sw %u \r\n", systemData.limitSwToggles);
            strcat(serialBuf, rowBuf);
//            sprintf(rowBuf, "Acks %u, pending %u, busy %d \r\n", acks, ackPending, PORTCbits.RC2 );
//            strcat(serialBuf, rowBuf);
            sprintf(rowBuf, "l/r/e %u %u %u \r\n", systemData.leftCounts , systemData.rightCounts, systemData.errCounts);
            strcat(serialBuf, rowBuf);            
            
            sprintf(rowBuf, "pr l/r %u %u \r\n", systemData.left_counts_p , systemData.right_counts_p );
            strcat(serialBuf, rowBuf);            
            
            strcat(serialBuf, "Current dir: ");
            if (systemData.curDirection == DIRECTION_LEFT_TO_STOP)
            {
                strcat(serialBuf, "Left to stop \r\n");
            }
            else if (systemData.curDirection == DIRECTION_RIGHT_FROM_STOP)
            {
                strcat(serialBuf, "right from stop \r\n");
            }
            else
                {
                strcat(serialBuf, "err \r\n");
            }    
            
            
            //
            break;
        case 2:
            strcat(serialBuf, "Buffer:\r\n");
            
            for (i=0;i<32;i++)
            {
                sprintf(rowBuf, "%u,", systemData.rollingInputBuffer[i]);
                strcat(serialBuf, rowBuf);
            }
             strcat(serialBuf, " \r\n");
             break;
             
        case 3:
            strcat(serialBuf, "Print status:\r\n");
            sprintf(rowBuf, "INT1 ints %u, state %u \r\n", systemData.int1_ints, systemData.prnState );
            strcat(serialBuf, rowBuf);
            sprintf(rowBuf, "print counts %u, busy in %u \r\n", systemData.printCycles, BUSY_IN );
            strcat(serialBuf, rowBuf);
            
            break;
        default:
            break;
    }
    
    
    
    
    send_string(serialBuf);
//    
//    //load to txreg
//    TXREG = 'k';
//    ;
}

inline void delay1us(void)
{
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();

    Nop();
    Nop();
    Nop();
    Nop();
    Nop();

    Nop();
    Nop();
    Nop();
    Nop();
    Nop();

    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
}

inline void setCentByte(char c)
{
        //Set data out
    LATA0 = (c>>0) & 1;   //d0
    LATA1 = (c>>1) & 1;   
    LATA2 = (c>>2) & 1;   
    LATA3 = (c>>3) & 1;   
    
    LATA5 = (c>>4) & 1;   
    LATE0 = (c>>5) & 1;   
    LATE1 = (c>>6) & 1;   
    LATE2 = (c>>7) & 1;   //d7
 
}

void sendPrinterByte(char c)
{

    ackPending = 0;
    while(BUSY_IN == 1)
    {
        Nop();
    }
    
    setCentByte(c);
    
   //Assert
    SET_CENT_OUTPUT;
    //Delay at least .5 us
    delay1us();
    
    //pulse strobe
    LATD1=0;
    delay1us();
    delay1us();
    LATD1=1;
    
    while (ackPending == 0)
    {
        Nop();
    }
    
    ackPending = 0;
    
    SET_CENT_INPUT;
    delay1us();
    delay1us();
    delay1us();
    delay1us();
    
}

void testPrintCycle(void)
{
    systemData.prnState=PRN_STATE_PRINTING;
    systemData.left_counts_p = 0;
    systemData.right_counts_p = 0;
    sendPrinterByte(' ');
    sendPrinterByte(' ');
    sendPrinterByte(' ');
    sendPrinterByte('R');
    sendPrinterByte('O');
    sendPrinterByte('G');
    sendPrinterByte('E');
    sendPrinterByte('R');

    sendPrinterByte('\r');
    sendPrinterByte('\n');
//    sendPrinterByte('\0');
    
    systemData.printCycles++;
}
    
void process_read(char byte)
{
    if ((byte >= '0') && (byte <='9'))
    {
        serialScreen = byte - '0';
        return;
    }
    
    switch (byte)
    {
        case 'r':
            resetState();
            break;
        case 'p':
            
            testPrintCycle();
            
            break;

        default:
            break;
    }
}

void read_serial(void)
{
    if (RCIF)
    {
        //Byte was received
        process_read(RCREG);
    }
    
    if (RCSTAbits.OERR)
    {
        RCSTAbits.CREN = 0;
        Nop();
        RCSTAbits.CREN = 1;
        
    }
}

