/*
 * File:   main.c
 * Author: w7yf6
 *
 * Created on February 24, 2026, 2:35 PM
 */


// PIC16F18855 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FEXTOSC = ECH    // External Oscillator mode selection bits (EC above 8MHz; PFM set to high power)
#pragma config RSTOSC = HFINT32 // Power-up default value for COSC bits (HFINTOSC with OSCFRQ= 32 MHz and CDIV = 1:1)
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

void SPIWriteByte(uint8_t data)
{
    SSP1BUF = data;
    while(SSP1STATbits.BF == 0){}; //data transmit in pragress
    
    return;
}

void writeRegister(uint8_t address, uint8_t data)
{
    LATBbits.LATB3 = 0;//1. redundant, but good to include the step just incase
    
    uint8_t controlByte = address & 0b01111111; //steps 2 & 3
    
    SPIWriteByte(controlByte);
    SPIWriteByte(data);
    
    LATBbits.LATB3 = 0; //cs high, stop transmission
    //can put /CS high to end, or start a new ctrl byte.
      
    return;
}

uint8_t SPIReadByte()
{
    //dummy byte = 0xFF
    SPIWriteByte(0xFF);
    return SSP1BUF;
}

uint8_t readRegister(uint8_t address)
{
    
    SPIWriteByte(0x80 | address);//control byte
        
    return SPIReadByte();
}

void main(void) {
    
    
    //SDI - RB3 - Input
    ANSELBbits.ANSB0 = 0;
    TRISBbits.TRISB0 = 1;
    SSP1DATPPS = 0x08; //RB0
    
    //SDO - RB4
    ANSELBbits.ANSB1 = 0;
    TRISBbits.TRISB1 = 0;
    RB1PPS = 0x15; //SDO
    
    
    //SCK - RB5
    ANSELBbits.ANSB2 = 0;
    TRISBbits.TRISB2 = 0;
    RB2PPS = 0x14; //SCK1
    
    //add CS Stuff later
    
    SSP1CON1bits.CKP = 1; //sensor clock idles high
    SSP1STATbits.CKE = 0; //data is changed at the falling edge
    SSP1STATbits.SMP = 1; //as specified
    
    
    SSP1CON1bits.SSPM=0b1010;
    SSP1ADD = 7; //from calculation, Fclock = 1Meg
    SSP1CON1bits.SSPEN = 1; //SPI on
    
    //setup S2 (RA5)
    ANSELAbits.ANSA5 = 0;
    TRISAbits.TRISA5 = 1; //input
    
    //writeRegister(0xF4, 0x27); //put device in normal power mode
    
    
    writeRegister(0xF4, 0x27);
    
    
    while(1)
    {
        if (PORTAbits.RA5 == 0) //switch pressed
        {
            while(PORTAbits.RA5 == 0){}//wait until switch released
            uint8_t msb = readRegister(0xFA);
            uint8_t lsb = readRegister (0xFB);
            uint8_t xlsb = readRegister (0xFC);
            uint32_t raw_temp = ((uint32_t)msb << 12) | ((uint32_t)lsb << 4) | ((uint32_t)xlsb >> 4);
            uint16_t temp = (uint16_t)raw_temp/16;
        }
        else
        {
            
        
        }
    }
    
    
    
    
    return;
}
