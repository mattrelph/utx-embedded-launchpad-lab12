// TuningFork.c Lab 12
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3, PB3, or PE3.
// There is an output on PA2, PB2, or PE2. The output is 
//   connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include "TExaS.h"
#include "..//tm4c123gh6pm.h"

// Globals and defineds
#define IN_PA3 	(*((volatile unsigned long *)0x40004020))	//Our Input Pin
#define OUT_PA2   (*((volatile unsigned long *)0x40004010))	//Our Output Pin
	

unsigned long outputEnable;
unsigned long old_PA3;



// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode

// input from PA3, output from PA2, SysTick interrupts
void Sound_Init(void){ 
	SYSCTL_RCGC2_R |= 0x00000001; // activate port A
  //delay = SYSCTL_RCGC2_R;
  GPIO_PORTA_AMSEL_R &= ~0x0C;      // no analog
  //GPIO_PORTA_PCTL_R &= ~0x00F00000; // regular function
	GPIO_PORTA_PCTL_R = 0x00; // regular function
  GPIO_PORTA_DIR_R |= 0x04;     // make PA2 output, PA3 input
  GPIO_PORTA_DR8R_R |= 0x04;    // can drive up to 8mA out
  GPIO_PORTA_AFSEL_R &= ~0x0C;  // disable alt funct on PA3, PA2
  GPIO_PORTA_DEN_R |= 0x0C;     // enable digital I/O on PA3, PA2
	/*
	GPIO_PORTA_IS_R &= ~0x04;     // (d) PA2 is edge-sensitive
  GPIO_PORTA_IBE_R &= ~0x04;    //     PA2 is not both edges
  GPIO_PORTA_IEV_R &= 0x04;    //     PA2 rising edge event
  GPIO_PORTA_ICR_R = 0x04;      // (e) clear flag2
  GPIO_PORTA_IM_R |= 0x04;      // (f) arm interrupt on PA2
  NVIC_PRI0_R = (NVIC_PRI7_R&0xFFFFFF0F)|0x000000A0; // (g) priority 5 for Port A NVIC
  NVIC_EN0_R = 0x00000000;      // (h) enable interrupt 00 (Port A) in NVIC
	*/
	
  NVIC_ST_CTRL_R = 0;           // disable SysTick during setup
  NVIC_ST_RELOAD_R = 90908;     // reload value for 880Hz (assuming 80MHz) MATH: 1/(880)/(1/80000000)-1 = 90908.0909
  NVIC_ST_CURRENT_R = 0;        // any write to current clears it
  NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0               
  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts

	outputEnable =0;	//Set variables to initial expected state (No output)
	old_PA3=0;
}

// called at 880 Hz
void SysTick_Handler(void){	
	

	if (IN_PA3)		//Testing for rising edge, since we do not have an interrupt. 
	{
		if (old_PA3 != IN_PA3)	//Compare current input to last known input we want 0->1
		{
			if (outputEnable ==1)		//If we have a rising edge, we toggle the output on or off
			{
				outputEnable=0;		//Turns off output
			}
			else
			{
				outputEnable = 1;	//Turns on output
			}
		}
	}

	old_PA3 = IN_PA3;	//Store the the input so we can compare on next clock interrupt.
	
	if (outputEnable==1)
	{
  
		OUT_PA2 ^= 0x04;	// toggle PA2 output for speakers
	}
	else
	{
		OUT_PA2 ^= 0x00;		//Output PA2 to Zero (no sound)
	}
}

// Output Enable handler
/*void GPIOPortA_Handler(void){
	GPIO_PORTA_ICR_R = 0x10;      // acknowledge flag4
	if (enable == 1)
	{
		enable = 0;
	}
	else
	{
		enable = 1;
	}
}*/

int main(void){// activate grader and set system clock to 80 MHz
  TExaS_Init(SW_PIN_PA3, HEADPHONE_PIN_PA2,ScopeOn); 
  Sound_Init();         
  EnableInterrupts();   // enable after all initialization are done
  while(1){
    // main program is free to perform other tasks
    // do not use WaitForInterrupt() here, it may cause the TExaS to crash
  }
}
