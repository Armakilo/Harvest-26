/*
 * File:   linefollower.c
 * Author: Adam Bateman
 *
 * Created on March 5, 2026, 11:31 AM
 */


#include <xc.h>
//I use pins RB0, RB1, and RB2 for my ADC inputs
//Still need to debug this


uint16_t rd_adc(uint8_t select){
    
    //select takes in 1, 2, or 3. This corresponds to reading the left, right, and center sensor, respectivley
    
    if (select == 1){
        ADPCH = 0b001000; //RB0 is OUT1
    }
    
    else if (select == 2){
        ADPCH = 0b001001; //RB1 is OUT2
    }
    
    else if (select == 3){
        ADPCH = 0b001010; //RB2 is OUT3
    }
    
    ADCON0bits.ADGO = 1; //ADC ready, can now start a conversion
    
    uint16_t output = (uint16_t)ADRESL + ((uint16_t)ADRESH << 8);
    return output;
}

void sendit(char it[], int it_size) // *** could we make a master file with these sorts of functions?
{
    for(int i = 0; i < it_size; i++){
        while(TX1STAbits.TRMT == 0){} //waits until register can send data
        TX1REG = it[i];
        
    }
    return;
}

void runmotor(uint8_t select){ //Motor 1 -> Left motor
    //select takes 1, 2, or 3 which corresponds to turn left, right, or straight
    // if a different value is given, the motor will automatically stop
    char msg[] = {0xFE,0x19,0x01,0x06,0x04,0x00, 0x0, 0x0, 0x0, 0x0};
    if (select == 1){ //go left
        msg[6] = 1;
        msg[7] = 50;
        msg[8] = 1;
        msg[9] = 10;
    }
    
    else if (select == 2){ // go right
        msg[6] = 1;
        msg[7] = 10;
        msg[8] = 1;
        msg[9] = 50;
    }
    
    else if (select == 3){ //go straight
        msg[6] = 1;
        msg[7] = 50;
        msg[8] = 1;
        msg[9] = 50;
    }
    
    sendit(msg, 10);
    return;
}

void follow(){
    //ADC ouputs a 10-bit value, which has a maximum of 0x3FF or 1023.
    //Use same threshold values as example code for now
    volatile const uint16_t wlvl = 600; //white if adc reads < 600
    volatile const uint16_t blvl = 850; //black if adc reads > 850
    
    //setup pins RB0, RB1, and RB2
    ANSELB = 0b0111; 
    TRISB = 0b0111;
    
    //ADC setup -> VREF+ is connected to pin RA3, so could I supply that with 5V?
    // Right now, Vref is connected to Vdd, which is 3V3
    
    
    ADCLK = 0b11111; //Fosc/128
    
    ADREFbits.ADPREF = 0b00; // V_DD
    ADREFbits.ADNREF = 0; // AV_SS
    
    //might have to put ADC on here
    
    ADACQ = 0b001; // 1 clock of ADC cycle to get data
    
    ADCON0bits.ADFRM0 = 1; // data right justified
    
    ADCON0bits.ADON = 1; //ADC on
    
    
    
    
    while(1){ // change to watch for switch position
        //case 1: L-W C-B R-W
        if (rd_adc(1) < wlvl && rd_adc(3) > blvl && rd_adc(2) < wlvl){
            runmotor(3);
        }

        //case 2: L-W C-B R-B
        else if (rd_adc(1) < wlvl && rd_adc(3) > blvl && rd_adc(2) > blvl){
            runmotor(2);        
        }

        //case 3: L-W C-W R-B
        
        else if (rd_adc(1) < wlvl && rd_adc(3) < wlvl && rd_adc(2) > blvl){
            runmotor(2);
        }

        //case 4: L-B C-W R-W
        
        else if (rd_adc(1) > blvl && rd_adc(3) < wlvl && rd_adc(2) < wlvl){
            runmotor(1);
        }

        //case 5: L-B C-B R-W
        else if (rd_adc(1) > blvl && rd_adc(3) > blvl && rd_adc(2) < wlvl){
            runmotor(1);        
        }
        
        else{
            runmotor(4);
        }
    }
    
    return;
}