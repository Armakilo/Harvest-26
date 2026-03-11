/*
 * File:   FFT.c
 * Author: k7csk
 *
 * Created on March 11, 2026, 9:58 AM
 */


#include <xc.h>
#include <stdlib.h>
#include <math.h>

typedef struct
{
    double real;
    double imaginary;
} Complex; // stores the real and imaginary components of complex numbers

int samples = 200;
float signal[samples];

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

int FFT()
{
    pollExpress(); // fills out signal variable with sampled signal  
    
    int half = samples/2;
    
    Complex* arr = malloc(sizeof(Complex));
    
    arr[0] = samples[0];
    
    


}

void pollExpress()
{
    
}

void main(void) {
    return;
}
