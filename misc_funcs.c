/*
 * File:   misc_funcs.c
 * Author: w7yf6
 *
 * Created on March 13, 2026, 2:09 PM
 */


#include <xc.h>

void sendit(char it[], int it_size) // *** could we make a master file with these sorts of functions?
{
    for(int i = 0; i < it_size; i++){
        while(TX1STAbits.TRMT == 0){} //waits until register can send data
        TX1REG = it[i];
        
        
        
    }
    return;
}


void flyskyask()
{
    char ask[6] = {0xFE,0x19,0x01,0x05,0x00,0x00};
    sendit(ask,6);
    return;    
}
