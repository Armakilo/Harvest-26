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
#include "harvest.h"
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
volatile int VRA = 0;
volatile uint16_t fund_freq = 0;



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

// A weird quirk of the code is that, sometimes, if you program the PIC with everything attached, the UART gets stuck and cant transmit data.
// I just unplug everything and then press play again
// Things usually get stuck at GetControllerInfo(), where the TRMT bit of TX1STA register never gets set

// we could try resetting various bits(SPEN, TXEN, etc if it becomes a problem in the future)

void getfund()
{
    LATCbits.LATC3 = 0;
    
    fund_freq = 0;
    
    SSP1BUF = 0xff;
    while(SSP1STATbits.BF == 0){};
    fund_freq = fund_freq | ((uint16_t)SSP1BUF << 8);
    
    LATCbits.LATC3 = 1;
    __delay_ms(5);
    LATCbits.LATC3 = 0;
    
    SSP1BUF = 0xff;
    while(SSP1STATbits.BF == 0){};
    fund_freq = fund_freq | ((uint16_t)SSP1BUF);
    
    LATCbits.LATC3 = 1;
}

void sendfund()
{
    uint8_t msg[] = {0xFE, 0x19, 0x01, 0x0A, 0x04, 0x00, 2, 0, 0, 0};
    msg[8] = fund_freq & 0xFF;
    msg[9] = ((fund_freq & 0xFF00) >> 8);
    sendit(msg, 10); 
}

void main(void) 
{
    uint8_t vra_flag = 0;
    //setup
    TRISA = 0b00100000;
    ANSELA = 0;
    
    
    TRISCbits.TRISC6 = 0; // Set TX pin as output
    ANSELCbits.ANSC6 = 0; // Set as digital
    
    //setup UART
    TX1STAbits.TXEN = 1;
    TX1STAbits.SYNC = 0;
    
    RC1STAbits.SPEN = 0; // reset the bit, as per my suggestions
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
    
    SPISetup();
    
    int msize = 10;
    char message[10] = {0xFE,0x19,0x01,0x06,0x04,0x00, 0x0, 0x40, 0x0, 0x40};
    int x = 1;
    
    
    while(1)
    {              
        int prev_SWB = SWB; // might go here?
        // check to make sure things are working
        if(PORTAbits.RA5 == 0)
        {
            LATA = 0xF;
			SetPCUInfo();
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
                
                 if (shield_code_flag != 0)
                {
                    LATAbits.LATA0 = 1;  // Turn on LED to show shield code received
                }
                else
                {
                    LATAbits.LATA0 = 0;
                }
                
                GetInfoController();
            }
            else if (receive_flag == 3)
            {
                receive_ready = 0;
                motor(control_data);
                SWD = ((control_data[21] << 8) + control_data[20]);
                SWC = ((control_data[19] << 8) + control_data[18]);
                // prev SWA = (control_data[15] << 8) + (control_data[16]); //is this wrong?, check this out later. Yeah this is wrong but I want to wait till we have things assembled to test it
                SWA = (control_data[15] << 8) + (control_data[14]);
                SWB = (control_data[17] << 8) + control_data[16];
                VRA = (control_data[23] << 8) + control_data[22];
                ShootLaser();
                GetInfoPCU();
            }
        }
        
        
        
        
        if(SWA > 1900){
            LATAbits.LATA2 = 1; //LED on
            __delay_ms(500);
            follow();
            LATAbits.LATA2 = 0; //LED off
            
        }
        
        if((SWB > 1900) && (VRA > 1500) && (vra_flag ==0)){ //VRA Left -> RFID
            LATAbits.LATA0 = 1;
            __delay_ms(500);
            volatile uint8_t checksum = GetUID();
            LATAbits.LATA0 = 0;
            vra_flag = 1;
                    
        }
        
        if((SWB > 1900) && (VRA < 1500) && (vra_flag == 0) ){ //check on the situation with what VRA reads, this is just a guess
            LATAbits.LATA1 = 1;
            __delay_ms(500);
            getfund();
            
            do{
                getfund();
                __delay_ms(50);
            }while(fund_freq == 0x8F37);
            sendfund();
            LATAbits.LATA1 = 0;
            vra_flag = 1;
        }
     
        if(SWB < 1100){
            vra_flag = 0;
        }
    
    }
    
    
    return;
}
