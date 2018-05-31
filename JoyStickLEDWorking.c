#include <avr/io.h>
#include "timer.h"
#include "bit.h"
#include "scheduler.h"
#include <avr/interrupt.h>


#define F_CPU 1000000
#include <util/delay.h>

int HORIZONTAL= 0;//neutral value on x-axis
int VERTICAl = 0;// neutral value on y-axis
int HORIZONTALMOV =0;
int VERTICAlMOV =0;
unsigned char col = 0x10;
unsigned char row = 0x10;


int inputTick(int state);
void transmit_column(unsigned char data);
void transmit_row(unsigned char data);

void transmit_column(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		PORTC = 0x08;
		// set SER = next bit of data to be sent.
		PORTC |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTC |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from ?Shift? register to ?Storage? register
	PORTC |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTC = 0x00;
}

void transmit_row(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTD = 0x08;
		// set SER = next bit of data to be sent.
		PORTD |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTD |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTD |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTD = 0x00;
}

void InitADC(void){
	ADMUX|=(1<<REFS0);
	//ADCSRA |=(1<<ADEN) |(1<ADPS2)|(1<ADPS1) |(1<<ADPS0);
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE); //ENABLE ADC, PRESCALER 128
}

//Enumeration of states.
// Monitors joystick connected to PA.
enum inputStates {Init, GetSetADC};
int inputTick(int state){
	switch(state){ //transition switch
		case Init:
			//init the components
			state = GetSetADC;
			break;
		case GetSetADC:
			state = GetSetADC;
			break;
		default:
			state = Init;
			break;
	}
	switch(state){ //action switch
		case Init:
			break;
		case GetSetADC:
			switch (ADMUX)//changing between channels by switch statement
			{
				case 0x40://When ADMUX==0x40
				{
					ADCSRA |=(1<<ADSC);//start ADC conversion
					while ( !(ADCSRA & (1<<ADIF)));//wait till ADC conversion
					HORIZONTALMOV = ADC;//moving value
					ADC=0;//reset ADC register
					ADMUX=0x41;//changing channel
					break;
				}
				case 0x41:
				{
					ADCSRA |=(1<<ADSC);// start ADC conversion
					while ( !(ADCSRA & (1<<ADIF)));// wait till ADC conversion
					VERTICAlMOV = ADC;// moving value
					ADC=0;// reset ADC register
					ADMUX=0x40;// changing channel
					break;
				}
			}
			if (HORIZONTALMOV<HORIZONTAL+400){  // Right
				 if(col >= 0x40){
					 col = 0x80;
				 }
				 else{
					 col = col << 1;
				 }
			 }

		if (HORIZONTALMOV>(HORIZONTAL+700)){ //Left:  if (HORIZONTALMOV>(HORIZONTAL+700))
			 if(col <= 0x01){
				 col = 0x01;
			 }
			 else{
				 col = col >> 1;
			 }
		 }
//if less than 400 right, if greater than 700 left

		  if (VERTICAlMOV<VERTICAl+400){ //if
		     if(row >= 0x40){
			     row = 0x80;
		     }
		     else{
			     row = row << 1;
		     }
		   }

		   if (VERTICAlMOV>VERTICAl+700){ //test1
			   if(row <= 0x01){
				   row = 0x01;
			   }
			   else{
				   row = row >> 1;
			   }
		   }
		    transmit_column(~col);
			transmit_row(row);
			break;
		default:
			state = Init;
			break;
	}

	return state;
}

int main(void)
{
	//set data direction registers
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	DDRA = 0x00; PORTA = 0xFF;

	unsigned long int tick1_calc = 50;
	unsigned long int GCD = 10;
	unsigned long int SMTick1_period = tick1_calc/GCD;

	static task task1;
	task *tasks[] = { &task1};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	task1.state = -1; //task init state
	task1.period = SMTick1_period;
	task1.elapsedTime = task1.period;
	task1.TickFct = &inputTick;

	InitADC();
	TimerSet(25);
	TimerOn();
	unsigned char i;
	while(1)
	{
		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
}
