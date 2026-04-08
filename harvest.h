/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   harvest_tasks.h
 * Author:  Adam Bateman
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef HARVEST_H
#define	HARVEST_H

#include <xc.h> // include processor files - each processor file is guarded. 
#define _XTAL_FREQ 32000000

extern volatile uint8_t control_data[26];
extern volatile int data_type;
extern volatile int data_size;
extern volatile int data_index;
extern volatile int receive_ready;
extern volatile int receive_flag;
extern volatile int SWA;
extern volatile int SWB;
extern volatile int SWC;
extern volatile int SWD;
extern volatile uint8_t shield_code_flag;
extern volatile uint8_t repair_code_flag;


// Insert declarations
void GetUID();

void SPISetup();

void ShootShield();

void ShootAttack();

void ShootRepair();

void ShootLaser();

void sendit(char it[], int it_size);

void GetInfoController();

void GetInfoPCU();

void SetPCUInfo();

void motor(char data[26]);

void follow();

void sendit(char it[], int it_size);



#endif	/* HARVEST_TASKS_H */

