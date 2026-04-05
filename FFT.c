/*
 * File:   FFT.c
 * Author: k7csk
 *
 * Created on March 11, 2026, 9:58 AM
 */


// DSPIC33EP32MC202 Configuration Bit Settings

// 'C' source line config statements

// FICD
#pragma config ICS = PGD2               // ICD Communication Channel Select bits (Communicate on PGEC2 and PGED2)
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
#pragma config POSCMD = NONE            // Primary Oscillator Mode Select bits (Primary Oscillator disabled)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function bit (OSC2 is clock output)
#pragma config IOL1WAY = ON             // Peripheral pin select configuration (Allow only one reconfiguration)
#pragma config FCKSM = CSDCMD           // Clock Switching Mode bits (Both Clock switching and Fail-safe Clock Monitor are disabled)

// FOSCSEL
#pragma config FNOSC = FRCPLL           // Oscillator Source Selection (Fast RC Oscillator with divide-by-N with PLL module (FRCPLL) )
#pragma config PWMLOCK = ON             // PWM Lock Enable bit (Certain PWM registers may only be written after key sequence)
#pragma config IESO = ON                // Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source)

// FGS
#pragma config GWRP = OFF               // General Segment Write-Protect bit (General Segment may be written)
#pragma config GCP = OFF                // General Segment Code-Protect bit (General Segment Code protect is Disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct
{
    double real;
    double imaginary;
} Complex; // stores the real and imaginary components of complex numbers

#define samples 128
Complex signal[samples];
int sampledVal[samples];
int fs = 1750000;
int count = 0;
Complex* FFT(Complex* sampleArr, int size);
int fund = 0;


void __attribute__((interrupt, no_auto_psv)) _AD1Interrupt(void)
{
    if (count < samples)
    {
        volatile unsigned int *adcPtr = &ADC1BUF0;

        for (int i = 0; i < 16; i++)
        {
            
            signal[count].real = (double)(adcPtr[i]);
            signal[count].imaginary = 0;

            count++;
            
        }
    }
    else
    {
        IEC0bits.AD1IE = 0;
    }
    IFS0bits.AD1IF = 0;
    
}

void ADC_setup()
{
    // oscillator 
    CLKDIVbits.DOZEN = 0; // matches peripheral and processor clock
    CLKDIVbits.FRCDIV = 0b000; // no FRC postscaler
    
    // sets Fp to 35 MHz - FIN/FRC = 7.37  MHz
    // minimu requrements on pg-156
    CLKDIVbits.PLLPOST = 0b00; // N2 = 2 (N2 = 2*(value + 1)) value can't = 2
    CLKDIVbits.PLLPRE = 0b00000; // N1 = 2 (N1 = value + 2)
    PLLFBDbits.PLLDIV = 36; // M = 38 (M = value + 2) value up to 511
    
    while(OSCCONbits.LOCK == 0); // waits until PLL timer is ready
    
    // control register 1
    AD1CON1bits.ADSIDL = 0; // continuous operation while idle
    AD1CON1bits.AD12B = 1; // 12 bit ADC
    AD1CON1bits.FORM = 0b00; // unsigned int out
    AD1CON1bits.SSRC = 0b111; // auto-convert, 0b010 for Timer 3 control
    AD1CON1bits.SSRCG = 0; // for SSRC control
    AD1CON1bits.ASAM = 1; // auto sample 
    
    // control register 2
    AD1CON2bits.VCFG = 0b000; // Vref +/- is VDD and VSS
    AD1CON2bits.CSCNA = 0; // does not scan other inputs
    AD1CON2bits.SMPI = 0b1111; // Generates an interupt every conversion
    AD1CON2bits.BUFM = 0; // Always fill buffer from start
    AD1CON2bits.ALTS = 0; // always selects MUXA samples
    
    // control register 3
    // minimum parameters on pg-469
    AD1CON3bits.ADRC = 0; // uses system clock (Fp)
    AD1CON3bits.ADCS = 4; // TAD = 142.83ns (TAD = Tp(value + 1)
    AD1CON3bits.SAMC = 4; // sample time = value*TAD 
    
    // control register 4
    AD1CON4bits.ADDMAEN = 0; // doesn't use DMA
    
    // additional 
    // find on pg-335/336
    AD1CHS0bits.CH0NA = 0; // CH0 negative input = VREFL
    AD1CHS0bits.CH0SA = 0; // ADC input pin = AN0
    
    IFS0bits.AD1IF = 0;
    IPC3bits.AD1IP = 4;
    IEC0bits.AD1IE = 1;    
    
    // turns on ADC
    AD1CON1bits.ADON = 1; 
}


void sendByte(uint8_t data)
{
    SPI1BUF = data;
    while(SPI1STATbits.SPITBF =! 0);
}

int fundFreq(Complex* FFT)
{
    double magnitudes[samples];

    double max = 0;
    int index = 0;

    for (int i = 0; i < samples; i++)
    {
        magnitudes[i] = sqrt(FFT[i].real * FFT[i].real +
                             FFT[i].imaginary * FFT[i].imaginary);

        if (magnitudes[i] > max)
        {
            if (i =! 0)
            {
                max = magnitudes[i];
                index = i;
            }
        }
    }

    return index * fs / samples;

}


void sendFFT()
{

    Complex* result = FFT(signal, samples);
    fund = fundFreq(result);
    
    sendByte((uint8_t)fund);
    
    count = 0;
    free(result);

}

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

Complex* FFT(Complex* sampleArr, int size)
{
    

    if (size == 1)
    {
            Complex* out = malloc(sizeof(Complex));
            out[0] = sampleArr[0];
            return out;
    }

  
    
    int half = size/2;
    
    Complex* even = malloc(half * sizeof(Complex)); // create arrays for even and odd components of the signal
    Complex* odd = malloc(half * sizeof(Complex));

    for(int i = 0; i < half; i++)
    {
        even[i] = sampleArr[2*i];
        odd[i] = sampleArr[2*i+1];
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


int main(void) {
    ADC_setup();
    
    
  
    SPI1STATbits.SPIEN = 0;
    
    SPI1CON1bits.MSTEN = 0;
    SPI1CON1bits.SSEN = 1;
    SPI1CON1bits.CKP = 0;
    SPI1CON1bits.CKE = 1;
    
    SPI1STATbits.SPIEN = 1;
    
    while(1)
    {
        
        if(count == samples)
        {
            
            sendFFT();
            
            IEC0bits.AD1IE = 0;
        }
        
    }
    
    
    return 1;
}
