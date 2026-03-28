/*
 * File:   FFT.c
 * Author: k7csk
 *
 * Created on March 11, 2026, 9:58 AM
 */


// DSPIC33EP32MC202 Configuration Bit Settings

// 'C' source line config statements

// FICD
#pragma config ICS = PGD1               // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
#pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)

// FPOR
#pragma config ALTI2C1 = OFF            // Alternate I2C1 pins (I2C1 mapped to SDA1/SCL1 pins)
#pragma config ALTI2C2 = OFF            // Alternate I2C2 pins (I2C2 mapped to SDA2/SCL2 pins)
#pragma config WDTWIN = WIN25           // Watchdog Window Select bits (WDT Window is 25% of WDT period)

// FWDT
#pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler bits (1:32,768)
#pragma config WDTPRE = PR128           // Watchdog Timer Prescaler bit (1:128)
#pragma config PLLKEN = ON              // PLL Lock Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (Watchdog timer enabled/disabled by user software)

// FOSC
#pragma config POSCMD = HS              // Primary Oscillator Mode Select bits (HS Crystal Oscillator Mode)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function bit (OSC2 is clock output)
#pragma config IOL1WAY = ON             // Peripheral pin select configuration (Allow only one reconfiguration)
#pragma config FCKSM = CSECMD           // Clock Switching Mode bits (Both Clock switching and Fail-safe Clock Monitor are disabled)

// FOSCSEL
#pragma config FNOSC = FRC              // Oscillator Source Selection (Internal Fast RC (FRC))
#pragma config PWMLOCK = ON             // PWM Lock Enable bit (Certain PWM registers may only be written after key sequence)
#pragma config IESO = OFF                // Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source)

// FGS
#pragma config GWRP = OFF               // General Segment Write-Protect bit (General Segment may be written)
#pragma config GCP = OFF                // General Segment Code-Protect bit (General Segment Code protect is Disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#include <xc.h>
#include <stdlib.h>
#include <math.h>

typedef struct
{
    double real;
    double imaginary;
} Complex; // stores the real and imaginary components of complex numbers

void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void)
{
    

    IFS0bits.U1TXIF = 0; 
}

void sendByte(uint8_t data)
{
    
}

int samples = 40000;
Complex* signal[samples];

Complex compAdd(Complex a, Complex b) // complex addition
{
    Complex res = {a.real + b.real, a.imaginary + b.imaginary};
    return res;
}

Complex compSub(Complex a, Complex b) // complex subtraction
{
    Complex res = {a.real - b.real, a.imaginary - b.imaginary};
    return res;
}

Complex compMult(Complex a, Complex b) // complex multiplication
{
    Complex res = {a.real * b.real - a.imaginary * b.imaginary, a.real * b.imaginary + a.imaginary * b.real};
    return res;
}

Complex* FFT(Complex* samples, int size)
{
    pollExpress(); // fills out signal variable with sampled signal  
    
    int half = size/2;
    
    Complex* arr = malloc(sizeof(Complex));
    
    arr[0] = samples[0];
    
    Complex* even = malloc(half * sizeof(Complex)); // create arrays for even and odd components of the signal
    Complex* odd = malloc(half * sizeof(Complex));

    for(int i = 0; i < half; i++)
    {
        even[i] = samples[2*i];
        odd[i] = samples[2*i+1];
    }
    
    Complex* recEven = FFT(even, half); // recursive sample split into odd and even parts
    Complex* recOdd = FFT(odd, half);
    
    Complex* output = malloc(size * sizeof(Complex));
    
    free(even);
    free(odd);
    
    for(int k = 0; k < half; k++)
    {
        double angle = -2.0 * M_PI * k/size;
        Complex twiddleFactor = {cos(angle), sin(angle)}; //determine twiddle factor for FFT and apply it to get output
        Complex twiddledOdd = compMult(twiddleFactor, recOdd[k]);
        output[k] = compAdd(recEven[k], twiddledOdd);
        output[k + half] = compSub(recEven[k], twiddledOdd);
    }
    
    free(recEven);
    free(recOdd);
    return output;
}


void main(void) {
    PLLFBD = 38;                // M = 40 for 40MHz Clock
    CLKDIVbits.PLLPRE = 0;      // N1 = 2
    CLKDIVbits.PLLPOST = 0;     // N2 = 2
    
    U1MODE = 0;
    U1STA = 0;
    
    U1BRG = int(40000000 / (16*9600)) - 1;
    
    U1MODEbits.UEN = 0b00;
    U1MODEbits.UARTEN = 1;
    U1MODEbits.STSEL = 0;
    U1MODEbits.PDSEL = 0b00;
    
    U1STAbits.UTXEN = 1;
    
    IPC3bits.U1TXIP = 4;
    IFS0bits.U1TXIF = 0;
    IEC0bits.U1TXIE = 0;
    
    
    
    while(1)
    {
        
        
    }
    
    
    return;
}
