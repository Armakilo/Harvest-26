/* 
 * File:   ADC_code.c
 * Author: corey
 *
 * Created on March 28, 2026, 2:29 PM
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
    float real;
    float imaginary;
} Complex; // stores the real and imaginary components of complex numbers

#define samples 256
Complex signal[samples];
uint32_t fs = 10000;
volatile int count = 0;
void FFT(Complex *x, int N);
volatile uint16_t fund = 0;
volatile uint16_t prev_fund = 0;
volatile uint16_t final_fund = 0;

void __attribute__((__interrupt__, no_auto_psv)) _SPI1Interrupt(void)
{
    IFS0bits.SPI1IF = 0;
}

void __attribute__((interrupt, no_auto_psv)) _AD1Interrupt(void)
{
    if (count <= samples - 16)
    {
        volatile unsigned int *adcPtr = &ADC1BUF0;

        for (int i = 0; i < 16; i++)
        {
            signal[count++] = (Complex){adcPtr[i], 0};
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
    AD1CON1bits.SSRC = 0b010; // 0b111 for auto-convert, 0b010 for Timer 3 control
    AD1CON1bits.SSRCG = 0; // for SSRC control
    AD1CON1bits.ASAM = 1; // auto sample 
    
    // Timer 3 setup
    T3CON = 0;
    TMR3 = 0;
    PR3 = 3499; 
    T3CONbits.TON = 1;
    
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

int fundFreq(Complex* FFT)
{   
    float magnitudes[samples];

    float max = 0;
    int index = 0;

    for (int i = 2; i < samples/2; i++)
    {
        magnitudes[i] = sqrtf(FFT[i].real * FFT[i].real +
                              FFT[i].imaginary * FFT[i].imaginary);

        if (magnitudes[i] > max)
        {
            max = magnitudes[i];
            index = i;
        }
    }

    return (int)((index * fs) / samples);
}

void sendFFT()
{
    // Remove DC offset
    float mean = 0;
    for (int i = 0; i < samples; i++)
        mean += signal[i].real;

    mean /= samples;

    for (int i = 0; i < samples; i++)
        signal[i].real -= mean;

    // Hann window
    for (int i = 0; i < samples; i++)
    {
        float w = 0.5 * (1 - cos(2 * M_PI * i / (samples - 1)));
        signal[i].real *= w;
    }
    
    FFT(signal, samples);
    fund = fundFreq(signal);
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

void FFT(Complex *x, int N)
{
    int i, j, k;
    int n1, n2;
    double c, s, e, t1, t2;

    // Bit-reversal
    j = 0;
    for (i = 1; i < N; i++)
    {
        int bit = N >> 1;
        while (j & bit)
        {
            j ^= bit;
            bit >>= 1;
        }
        j |= bit;

        if (i < j)
        {
            Complex temp = x[i];
            x[i] = x[j];
            x[j] = temp;
        }
    }

    // FFT
    for (i = 1; i <= log2(N); i++)
    {
        n1 = 1 << (i - 1);
        n2 = n1 << 1;
        e = -2 * M_PI / n2;

        for (j = 0; j < n1; j++)
        {
            c = cos(j * e);
            s = sin(j * e);

            for (k = j; k < N; k += n2)
            {
                t1 = c * x[k + n1].real - s * x[k + n1].imaginary;
                t2 = s * x[k + n1].real + c * x[k + n1].imaginary;

                x[k + n1].real = x[k].real - t1;
                x[k + n1].imaginary = x[k].imaginary - t2;

                x[k].real += t1;
                x[k].imaginary += t2;
            }
        }
    }
}

int main(void) {
    ADC_setup();
    
    SPI1BUF = 0;
    IFS0bits.SPI1IF = 0;
    
    IEC0bits.SPI1IE = 0; 
    SPI1CON1bits.DISSCK = 0;
    SPI1CON1bits.DISSDO = 0; 
    SPI1CON1bits.MODE16 = 1;
    SPI1CON1bits.SMP = 0;
    
    
    SPI1CON1bits.CKE = 0; 
    SPI1CON1bits.CKP = 0; 
    SPI1CON1bits.MSTEN = 0; 
    SPI1STATbits.SPIROV=0; 
    SPI1STATbits.SPIEN = 1; 
    
    
    IFS0bits.SPI1IF = 0; 
    IEC0bits.SPI1IE = 1;

    ANSELBbits.ANSB0 = 0;
    TRISBbits.TRISB0 = 1;
    
    while(1)
    {

        while(PORTBbits.RB0 == 0){}
        
        if(count == samples)
        {     
            IEC0bits.AD1IE = 0;
            
            prev_fund = fund;
            sendFFT();
            
            if (prev_fund == fund)
            {
                final_fund = fund;
            }
            else 
            {
                final_fund = 0xFFFF;
            }
       
            count = 0;
            IEC0bits.AD1IE = 1;
        }
        
    }
    
    
    return 1;
}
