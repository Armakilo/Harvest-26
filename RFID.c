/*
 * File:   RFID.c
 * Author: s8n93
 *
 * Created on March 11, 2026, 3:42 PM
 */

#include <xc.h>
#define _XTAL_FREQ 32000000

// RB4, RB5, RB3, are used, and we will use pin RC7 as our chip select



void SPIWriteByte(uint8_t data)
{
    SSP1BUF = data;
    while(SSP1STATbits.BF == 0){}; //data transmit in pragress
    
    return;
}

void writeRegister(uint8_t address, uint8_t data)
{
    LATCbits.LATC7 = 0;//1. redundant, but good to include the step just incase
    
    uint8_t controlByte = (address << 1) & 0b01111110; //steps 2 & 3
       
    
    SPIWriteByte(controlByte);
    SPIWriteByte(data);
    
    LATCbits.LATC7 = 0; //cs high, stop transmission
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
    
    SPIWriteByte(0x80 | (address << 1)) & 0xFE;//control byte, edited for MFRC522
        
    return SPIReadByte();
}

// we must scan the tag, and then send it back

void RFID_SPIsetup(void) { //might have to 
    
    
    
    //OSC_Init(){
    
    SSP2CON1bits.SSPEN = 0;
    
    // RB4 = SDI
    
    TRISBbits.TRISB4 = 1;
    ANSELBbits.ANSB4 = 0;
    SSP2DATPPS = 0x17;
    
    // RB5 = SDO
    
    TRISBbits.TRISB5 = 0;
    ANSELBbits.ANSB5 = 0;
    RC2PPS = 0x17;
    
    //RB3 = SCK
    
    TRISBbits.TRISB3 = 0;
    ANSELBbits.ANSB3 = 1;
    RC3PPS = 0x16;
    
    //RC7 = /CS
    
    TRISCbits.TRISC7 = 0;
    ANSELCbits.ANSC7 = 0; //remember to set up PORT bits
    
    SSP2CON1bits.CKP = 0; //clock idles low
    SSP2STATbits.CKE = 0; //data is transmitted on idle -> active
    SSP2STATbits.SMP = 1; // might be wrong, change later?
        
    // set clock to 50 kbits/sec or 50 kHz
    
    SSP2CON1bits.SSPM = 0b1010;
    SSP2ADD = 159;
    SSP2CON1bits.SSPEN = 1; //ready
            
    writeRegister(0x01, 0b010000);
    while((readRegister(0x01) & 0b0010000) == 0);
      
       
    return;
}

void RFID_Rx(){
    
    int data[64] = {0};
    
    writeRegister(0x13, 0b00001000); //RxModeReg, it will recive all data frames. (bit 3 = 0))
    writeRegister(0x0A, 0b10000000); //clear the FIFO using the FIFOLevelReg
    writeRegister(0x01, 0b00110000); //starts the recieve
    // read from the FIFO buffer
    
    // Nothing connected to pin MFIN? 
    // Modulation signal coming from internal part. to change this, look at reg 0x17.
    // ISO ..A uses Manchester coding?
    // UID is 4 bytes.
    // All MIFARE ICs are compliant to ISO 1443
    // Got a length byte, format byte, 
    
    for (int i=0; i<=63; i++){
        data[i] = readRegister(0x09);
    }

    
    
    
    return;
    


}

void RFID_Tx(){
    
    writeRegister();

    return;
}