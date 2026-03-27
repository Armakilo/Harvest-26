/*
 * File:   RFID.c
 * Author: s8n93
 *
 * Created on March 11, 2026, 3:42 PM
 */

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


#include <xc.h>
#define _XTAL_FREQ 32000000

// RB4, RB5, RB3, are used, and we will use pin RC7 as our chip select



void SPIWriteByte(uint8_t data)
{
    LATCbits.LATC7 = 0;
    SSP1BUF = data;
    while(SSP1STATbits.BF == 0){}; //data transmit in pragress
    LATCbits.LATC7 = 1;
    __delay_ms(1);
    return;
}

void writeRegister(uint8_t address, uint8_t data)
{
    
    
    uint8_t controlByte = (address << 1) & 0b01111110; //steps 2 & 3
       
    
    SPIWriteByte(controlByte);
    SPIWriteByte(data);
    
    
    //can put /CS high to end, or start a new ctrl byte.
      
    return;
}

uint8_t SPIReadByte()
{
    
    
    //dummy byte = 0xFE
    SPIWriteByte(0xFE);
    
    return SSP1BUF;
}

uint8_t readRegister(uint8_t address)
{
    
    SPIWriteByte((0x80 | (address << 1)& 0xFE)) ;//control byte, edited for MFRC522
    
    
    return SPIReadByte();
}

void tranceive(){
    


}

uint8_t rdFIFO(){

    uint8_t info = readRegister(0x09);
    return info;

}


// we must scan the tag, and then send it back

void main(){//RFID_SPIsetup() { //might have to 
    
    
    
    //OSC_Init(){
    
    SSP1CON1bits.SSPEN = 0;
    
    // RB4 = SDI
    
    TRISBbits.TRISB4 = 1;
    ANSELBbits.ANSB4 = 0;
    SSP1DATPPS = 0x0C;
    
    // RB5 = SDO
    
    TRISBbits.TRISB5 = 0;
    ANSELBbits.ANSB5 = 0;
    RB5PPS = 0x15;
    
    //RB3 = SCK
    
    TRISBbits.TRISB3 = 0;
    ANSELBbits.ANSB3 = 0;
    RB3PPS = 0x14;
    
    //RC7 = CS
    
    TRISCbits.TRISC7 = 0;
    ANSELCbits.ANSC7 = 0; //remember to set up PORT bits
    LATCbits.LATC7 = 1;
    
    SSP1CON1bits.CKP = 0; //clock idles low
    SSP1STATbits.CKE = 1; //data is transmitted on active -> idle
    SSP1STATbits.SMP = 1; // might be wrong, change later?
        
    // set clock to 50 kbits/sec or 50 kHz
    
    SSP1CON1bits.SSPM = 0b1010;
    SSP1ADD = 159;
    SSP1CON1bits.SSPEN = 1; //ready
    
    
     //setup S2 (RA5)
    ANSELAbits.ANSA5 = 0;
    TRISAbits.TRISA5 = 1; //input
    
//    //hard reset code - we are using a soft reset
//    TRISCbits.TRISC2 = 0; //output
//    ANSELCbits.ANSC2 = 0; //digital
//    
//    LATCbits.LATC2 = 0;
//    LATCbits.LATC2 = 1;
    
    
    uint8_t response[10];
            
    writeRegister(0x01, 0b0001111); //resets could also use a Hard Reset by writing a 0 then a 1 to the RST pin
    
    
    readRegister(0x3E);
    
    writeRegister(0x2A,0x84);//Tmode, TpreHi = 0x4, Tauto = 1
    writeRegister(0x2B,0x00);//Tprescaler
    writeRegister(0x2C,0x01);//TreloadH
    writeRegister(0x2D,0x49);//TreloadL
    writeRegister(0x11,0b00101001); //ModeReg
    writeRegister(0x15,0b01000000);//TxASKReg, force 100 ASK on
    
    uint8_t TxControl = readRegister(0x14);
    writeRegister(0x14, TxControl | 0b01000); //bitmask the Tx2CW bit not super sure about this one
    
    
    
    
    __delay_ms(50);
    writeRegister(0x09, 0x26); // write the REQA command to the FIFO
    
    rdFIFO();
     
    writeRegister(0x01, 0x0C); //Command register, tranceive
    writeRegister(0x0D, 0x87); //bitframing reg will now tranceive. bitmasked
    
    int buffbytes = readRegister(0x0A);
    
    for (int i = 0; i <= buffbytes; i++){
      response[i] = rdFIFO();
    }
    
   
    uint8_t errorRegValue = readRegister(0x06); //0x06 = error reg
    uint8_t controlRegValue = readRegister(0x0C); //0x0C = controlReg
    
    writeRegister(0x09, 0x93); //anticollision part 1
    writeRegister(0x09, 0x20); //anticollision part 2
    
    
    //need to do 2 reps to send all data
    
    writeRegister(0x01, 0x0C); //Command register, tranceive
    writeRegister(0x0D, 0x80); //bitframing reg will now tranceive. command is not 7 bits
    
    writeRegister(0x01, 0x0C); //Command register, tranceive
    writeRegister(0x0D, 0x80); //bitframing reg will now tranceive. command is not 7 bits
    
    
    buffbytes = readRegister(0x0A);
       
    for (int i = 0; i <= buffbytes; i++){
      response[i] = rdFIFO();
    }
    
    while(1){    
    }          
   
       
    return;
}
