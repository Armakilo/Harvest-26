/*
 * File:   milestone_2_code.c
 * Author: j86n6
 *
 * Created on February 9, 2026, 9:45 AM
 */


// PIC16F18855 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FEXTOSC = ECH    // External Oscillator mode selection bits (EC above 8MHz; PFM set to high power)
#pragma config RSTOSC = HFINT32   // Power-up default value for COSC bits (EXTOSC operating per FEXTOSC bits)
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (FSCM timer enabled)

// CONFIG2
#pragma config MCLRE = ON       // Master Clear Enable bit (MCLR pin is Master Clear function)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config LPBOREN = OFF    // Low-Power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) set to 1.9V on LF, and 2.45V on F Devices)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = ON     // Peripheral Pin Select one-way control (The PPSLOCK bit can be cleared and set only once in software)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled, SWDTEN is ignored)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4
#pragma config WRT = OFF        // UserNVM self-write protection bits (Write protection off)
#pragma config SCANE = available// Scanner Enable bit (Scanner module is available for use)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low Voltage programming enabled. MCLR/Vpp pin function is MCLR.)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (Program Memory code protection disabled)
#pragma config CPD = OFF        // DataNVM code protection bit (Data EEPROM code protection disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#define _XTAL_FREQ 32000000

volatile uint8_t control_data[26] = {};
volatile int data_type;
volatile int data_size = 10;
volatile int data_index = 0;
volatile int receive_ready = 0;
volatile int receive_flag = 0;
volatile int SWA = 0;
volatile int SWB = 0;
volatile int SWC = 0;
volatile int SWD = 0;
volatile uint8_t shield_code_flag = 0;
volatile uint8_t repair_code_flag = 0;


void sendit(char it[], int it_size) //function to send the data
{
    for(int i = 0; i < it_size; i++){
        while(TX1STAbits.TRMT == 0){} //waits until register can send data
        TX1REG = it[i];
        
    }
}

void GetInfoController()
{
    char ask[6] = {0xFE,0x19,0x01,0x05,0x00,0x00};
    sendit(ask,6);
        
}

void GetInfoPCU()
{
    char ask[6] = {0xFE,0x19,0x01,0x04,0x00,0x00};
    sendit(ask,6);
}

void motor(char data[26]){
    //volatile int* command = output;
    int mry = 0;
    int mly = 0;
    int rdirec = 0;
    int ldirec = 0;
    char msg[10] = {0xFE,0x19,0x01,0x06,0x04,0x00, 0x0, 0x40, 0x0, 0x40};
        mry = (data[8]+(data[9]<<8) - 1000)/5; //mry goes from 0 to 200
        mly = (data[10]+(data[11]<<8) - 1000)/5; //mly goes from 0 to 200
        if (mry < 110 && mry > 90){rdirec = 0;}
        else if (mry > 110)
        {
            rdirec = 1;
            mry = mry - 100;
        }        
        else if (mry < 90)
        {
            rdirec = 2;
            mry = 100 - mry;
        }
        
        if (mly < 110 && mly > 90){ldirec = 0;}
        else if (mly > 110)
        {
            ldirec = 1;
            mly = mly - 100;
        }        
        else if (mly < 90)
        {
            ldirec = 2;
            mly = 100 - mly;
        }
        
        msg[6] = ldirec;
        msg[7] = mly;
        msg[8] = rdirec;
        msg[9] = mry;
        
        sendit(msg, 10);
}

void ShootShield()
{
    char tosend[6] = {0xFE, 0x19, 0x02, 0x09, 0x00, 0x00};
    sendit(tosend, 6);
}

void ShootAttack()
{
    char tosend[7] = {0xFE, 0x19, 0x01, 0x09, 0x01, 0x00, 1};
    sendit(tosend, 7); 
}

void ShootRepair()
{
    char tosend[6] = {0xFE, 0x19, 0x04, 0x09, 0x00, 0x00};
    sendit(tosend, 6);
}

void ShootLaser()
{
    if (SWD > 1600)
        {
            if (SWC > 1800)
            {
                if (!shield_code_flag)
                {
                    ShootShield();
                }
            }
            else if (SWC > 1300 && SWC < 1700)
            {
                ShootAttack();
            }
            else if (SWC < 200)
            {
                if (!repair_code_flag);
                {
                    ShootRepair();
                }
            }
        }
}

void __interrupt() ISR(void)
{
    if (PIR3bits.RCIF) {
        control_data[data_index] = RC1REG; 
        data_index++;
        
        if (RC1STAbits.OERR) // clear overrun if needed
        {
            RC1STAbits.CREN = 0;
            RC1STAbits.CREN = 1;
        }
        
        if (data_index == 4)
        {
            data_type = (control_data[3]<<8) + control_data[2];
            if (data_type == 0x0300)
            {
                data_size = 6;
            }
            else if (data_type == 0x0402)
            {
                data_size = 12;
            }
            else if (data_type == 0x0502)
            {
                data_size = 26;
            }
        }
        if (data_index >= data_size && data_size == 6)
        {
            data_index = 0;
            receive_ready = 1;
            receive_flag = 1;
        }
        else if (data_index >= data_size && data_size == 12)
        {
            data_index = 0;
            receive_ready = 1;
            receive_flag = 2;
        }
        else if (data_index >= data_size && data_size == 26)
        {
            data_index = 0;
            receive_ready = 1;
            receive_flag = 3;
        }
    }
}

void main(void) 
{
    
    //setup
    TRISA = 0b00100000;
    ANSELA = 0;
    
    //setup UART
    TX1STAbits.TXEN = 1;
    TX1STAbits.SYNC = 0;
    RC1STAbits.SPEN = 1;
    
    RC1STAbits.CREN = 1; //enables 
    TRISCbits.TRISC5 = 1; //set Rx pin as input
    ANSELCbits.ANSC5 = 0;
    
    
    //setup baud rate.
    BAUDCONbits.BRG16 = 1;
    TX1STAbits.BRGH = 1;
    
    //n = 68 = ~0x44 baud rate == 115200
    SPBRGH = 0x0;
    SPBRGL = 0x44;
    
    //enable interrupts
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    PIE3bits.RCIE = 1;
    
    //PPS setup
    RC6PPS = 0x10; //tx
    RXPPS = 0x15; //rx
    //loop for tx data
    
    
    int msize = 10;
    char message[10] = {0xFE,0x19,0x01,0x06,0x04,0x00, 0x0, 0x40, 0x0, 0x40};
    int x = 1;
    
    
    while(1)
    {              
        // check to make sure things are working
        if(PORTAbits.RA5 == 0)
        {
            LATA = 0xF;
            GetInfoController();
            while(PORTAbits.RA5 == 0){}
            
        }
        else
        {
            LATA = 0;
        }
        
        if (receive_ready)
        {
            if (receive_flag == 1)
            {
                receive_ready = 0;
            }
            else if (receive_flag == 2)
            {
                receive_ready = 0;
                shield_code_flag = control_data[10];
                repair_code_flag = control_data[11];
                GetInfoController();
            }
            else if (receive_flag == 3)
            {
                receive_ready = 0;
                motor(control_data);
                SWD = ((control_data[21] << 8) + control_data[20]);
                SWC = ((control_data[19] << 8) + control_data[18]);
                ShootLaser();
                GetInfoPCU();
            }
        }
              
        
    
    }
    
    
    return;
}
