//This version of the program has the array and sounds fully mapped
//If sound frequencies are to be changed, change macros freq1 - freq8
//Still need to add user chosen sounds to an array, need to add music playback
//Still need to add game logic

//This test bench includes PWM, Joystick and LED
//3 State Machines
//PWM triggered on input
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "bit.h"
#include "scheduler.h"


//Set Starting Point
unsigned char column_select = 0x01;
unsigned char column_val = 0x01;

/********************************/
// Macros for Array Sound Mapping
/********************************/
#define A2 (~PINA & 0x04)
#define C4 ((column_select == 0x01 && column_val == 0x01) || (column_select == 0x02 && column_val == 0x01) || (column_select == 0x04 && column_val == 0x01) || (column_select == 0x08 && column_val == 0x01) || (column_select == 0x10 && column_val == 0x01) || (column_select == 0x20 && column_val == 0x01) || (column_select == 0x40 && column_val == 0x01) || (column_select == 0x80 && column_val == 0x01))
#define D4 ((column_select == 0x01 && column_val == 0x02) || (column_select == 0x02 && column_val == 0x02) || (column_select == 0x04 && column_val == 0x02) || (column_select == 0x08 && column_val == 0x02) || (column_select == 0x10 && column_val == 0x02) || (column_select == 0x20 && column_val == 0x02) || (column_select == 0x40 && column_val == 0x02) || (column_select == 0x80 && column_val == 0x02))
#define E4 ((column_select == 0x01 && column_val == 0x04) || (column_select == 0x02 && column_val == 0x04) || (column_select == 0x04 && column_val == 0x04) || (column_select == 0x08 && column_val == 0x04) || (column_select == 0x10 && column_val == 0x04) || (column_select == 0x20 && column_val == 0x04) || (column_select == 0x40 && column_val == 0x04) || (column_select == 0x80 && column_val == 0x04))
#define F4 ((column_select == 0x01 && column_val == 0x08) || (column_select == 0x02 && column_val == 0x08) || (column_select == 0x04 && column_val == 0x08) || (column_select == 0x08 && column_val == 0x08) || (column_select == 0x10 && column_val == 0x08) || (column_select == 0x20 && column_val == 0x08) || (column_select == 0x40 && column_val == 0x08) || (column_select == 0x80 && column_val == 0x08))
#define G4 ((column_select == 0x01 && column_val == 0x10) || (column_select == 0x02 && column_val == 0x10) || (column_select == 0x04 && column_val == 0x10) || (column_select == 0x08 && column_val == 0x10) || (column_select == 0x10 && column_val == 0x10) || (column_select == 0x20 && column_val == 0x10) || (column_select == 0x40 && column_val == 0x10) || (column_select == 0x80 && column_val == 0x10))
#define A4 ((column_select == 0x01 && column_val == 0x20) || (column_select == 0x02 && column_val == 0x20) || (column_select == 0x04 && column_val == 0x20) || (column_select == 0x08 && column_val == 0x20) || (column_select == 0x10 && column_val == 0x20) || (column_select == 0x20 && column_val == 0x20) || (column_select == 0x40 && column_val == 0x20) || (column_select == 0x80 && column_val == 0x20))
#define B4 ((column_select == 0x01 && column_val == 0x40) || (column_select == 0x02 && column_val == 0x40) || (column_select == 0x04 && column_val == 0x40) || (column_select == 0x08 && column_val == 0x40) || (column_select == 0x10 && column_val == 0x40) || (column_select == 0x20 && column_val == 0x40) || (column_select == 0x40 && column_val == 0x40) || (column_select == 0x80 && column_val == 0x40))
#define LastKey ((column_select == 0x01 && column_val == 0x80) || (column_select == 0x02 && column_val == 0x80) || (column_select == 0x04 && column_val == 0x80) || (column_select == 0x08 && column_val == 0x80) || (column_select == 0x10 && column_val == 0x80) || (column_select == 0x20 && column_val == 0x80) || (column_select == 0x40 && column_val == 0x80) || (column_select == 0x80 && column_val == 0x80))
/************************/
//End Mapping
/***********************/

void set_PWM();
void PWM_off();
void PWM_on();
void transmit_data(unsigned char data);
void transmit_data1(unsigned char data);


void set_PWM(double frequency) {

	static double current_frequency; // Keeps track of the currently set frequency

	// Will only update the registers when the frequency changes, otherwise allows

	// music to play uninterrupted.

	if (frequency != current_frequency) {

		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter

		else { TCCR3B |= 0x03; } // resumes/continues timer/counter



		// prevents OCR3A from overflowing, using prescaler 64

		// 0.954 is smallest frequency that will not result in overflow

		if (frequency < 0.954) { OCR3A = 0xFFFF; }



		// prevents OCR0A from underflowing, using prescaler 64     // 31250 is largest frequency that will not result in underflow

		else if (frequency > 31250) { OCR3A = 0x0000; }



		// set OCR3A based on desired frequency

		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }



		TCNT3 = 0; // resets counter

		current_frequency = frequency; // Updates the current frequency

	}

}
void PWM_on() {

	TCCR3A = (1 << COM3A0);

	// COM3A0: Toggle PB3 on compare match between counter and OCR0A

	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);

	// WGM02: When counter (TCNT0) matches OCR0A, reset counter

	// CS01 & CS30: Set a prescaler of 64

	set_PWM(0);

}
void PWM_off() {

	TCCR3A = 0x00;

	TCCR3B = 0x00;

}

/***************/
//PWM Variables for Frequency
double freq1 = 261.63;
double freq2 = 293.66;
double freq3 = 329.63;
double freq4 = 349.23;
double freq5 = 392.00;
double freq6 = 440.00;
double freq7 = 493.88;
double freq8 = 987.77;
/***************/

void transmit_data(unsigned char data) {
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

void transmit_data1(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTC = 0x80;
		// set SER = next bit of data to be sent. left shift four times to match.
		PORTC |= (((data >> i) << 4) & 0x10);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTC |= 0x20;
	}
	// set RCLK = 1. Rising edge copies data from ìShiftî register to ìStorageî register
	PORTC |= 0x40;
	// clears all lines in preparation of a new transmission
	PORTC = 0x00;
}


void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}


/******************/
// Local Variable
unsigned char temp = 0x00;
//Global Shared
unsigned char button = 0x00;
/******************/
enum inputStates2 {Release, Press, Wait};
int inputTick(int state){
	switch(state){ //transition switch
		case Release:
			if(A2){
				state = Press;
			}
			else{
				state = Release;
			}
			break;

		case Press:
			if(A2){
				state = Wait;
			}
			else{
				state = Release;
			}
			break;

		case Wait:
			if(A2){
				state = Wait;
			}
			else{
				state = Release;
			}
			break;

		default:
			state = Release;
			break;
	}

	switch(state){ //transition switch
		case Release:
		//PORTB = 0x00;
		button = 0x00;
		temp = 0x00;
		break;

		case Press:
		//PORTB = 0x01;
		button = 0x01;
		temp = 0x01;
		break;

		case Wait:
		button= 0x01;
		//PORTB
		if(temp < 0x07){
			temp = temp << 1;
		}
		else{
			temp = 0x01;
		}
	}
	PORTB = temp;
	return state;
}

enum SM1_States {sm1_display, playSound} states;

int SM1_Tick(int state) {

	// === Local Variables ===
	ADC_init();
	ADMUX = !ADMUX;
	unsigned short input;
	input = ADC;

	// === Transitions ===
	switch (state) {
		case sm1_display:
			set_PWM(0);
			if(button){
				state = playSound;
			}
			else{
				state = sm1_display;
			}
			break;
		case playSound:
			if(!button){
				set_PWM(0);
				state = sm1_display;
			}
			else{
				state = playSound;
			}
		default:
			state = sm1_display;
			break;
	}
	// === Actions ===
	switch (state) {
		case sm1_display:
			if(input < 225 && ADMUX == 0){ //left
				if(column_select <= 0x01){
					column_select = 0x01;
				}
				else{
					column_select = column_select >> 1;
				}
			}

			if(input > 0 && input < 225 && ADMUX == 1){ //down
				if(column_val <= 0x01){
					column_val = 0x01;
				}
				else{
					column_val = column_val >> 1;
				}
			}
			if(input > 700 && ADMUX == 0){ //right
				if(column_select >= 0x80){
					column_select = 0x80;
				}
				else{
					column_select = column_select << 1;
				}
			}
			else if(input > 700 && ADMUX == 1){ //up
				if(column_val >= 0x40){
					column_val = 0x80;
				}
				else{
					column_val = column_val << 1;
				}
			}
			break;

		case playSound:
			if(button){
				//if((column_val == 0x01 && column_select == 0x01)){
				if(C4){
					set_PWM(freq1);
				}
				if(D4){
					set_PWM(freq2);
				}
				if(E4){
					set_PWM(freq3);
				}
				if(F4){
					set_PWM(freq4);
				}
				if(G4){
					set_PWM(freq5);
				}
				if(A4){
					set_PWM(freq6);
				}
				if(B4){
					set_PWM(freq7);
				}
				if(LastKey){
					set_PWM(freq8);
				}
			}
			break;

		default:
		break;
	}
	transmit_data(~column_select); // PORTC displays column pattern
	transmit_data1(column_val); // PORTC selects column to display pattern
	return state;
};


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	DDRB = 0xFF; PORTB = 0x00;

	static task task2, task1; //task3;
	task *tasks[] = { &task2, &task1};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

  //Task 2
	task2.state = -1; //task init state
	task2.period = 5;
	task2.elapsedTime = task2.period;
	task2.TickFct = &SM1_Tick;

  //Task 1
	task1.state = -1; //task init state
	task1.period = 5;
	task1.elapsedTime = task1.period;
	task1.TickFct = &inputTick;


	TimerSet(25);
	TimerOn();
	PWM_on();

	unsigned char i;
	while (1)
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
		while(!TimerFlag) { }
		TimerFlag = 0;
	}
}
