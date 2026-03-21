/*
 * File:   misc_funcs.c
 * Author: w7yf6
 *
 * Created on March 13, 2026, 2:09 PM
 */


#include <xc.h>

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

void SetPCUInfo()
{
    char data[9] = {0xFE,0x19,0x03,0x04,0x03,0x00,4,17,1};
    sendit(data,9);
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


