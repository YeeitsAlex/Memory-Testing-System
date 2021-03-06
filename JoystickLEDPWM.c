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
#define A2 (~PINA & 0x04)

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
//PWM Variables
double freq1 = 261.63;
double freq2 = 293.66;
double freq3 = 329.63;
double freq4 = 349.23;
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


enum SM1_States {sm1_display} states;

int SM1_Tick(int state) {

	// === Local Variables ===
	ADC_init();
	ADMUX = !ADMUX;

	//static unsigned char column_val = 0x01; // sets the pattern displayed on columns
	//static unsigned char column_sel = 0x7F; // grounds column to display pattern
	unsigned short input;

	input = ADC;


	// === Transitions ===
	switch (state) {
		case sm1_display:
		break;
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

		default:
		break;
	}

	transmit_data(~column_select); // PORTA displays column pattern
	transmit_data1(column_val); // PORTB selects column to display pattern

	return state;
};

/******************/
// Local Variable
unsigned char temp = 0x00;
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
			temp = 0x00;
		break;

		case Press:
			//PORTB = 0x01;
			temp = 0x01;
		break;

		case Wait:
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

enum States {Start, WaitInput, ToneC4, ToneD4, ToneE4} State;
int Tick(int state){
	switch(State){//transition switch
		case Start:
			State = WaitInput;
			break;
		case WaitInput:
			if (A2){
				State = ToneC4;
				set_PWM(freq1);
			}
			else{
				State = WaitInput;
			}
			break;
		case ToneC4:
			if (A2){
				State = ToneD4;
				set_PWM(freq2);
			}
			else{
				State = WaitInput;
			}
			break;
		case ToneD4:
			if (A2){
				State = ToneE4;
				set_PWM(freq3);
			}
			else{
				State = WaitInput;
			}
			break;
		case ToneE4:
			if (A2){
				State = WaitInput;
			}
			else{
				State = Start;
			}
			break;
		default:
			State = Start;
			break;
	}

	switch(State){ //action switch
		case Start:
			break;
		case WaitInput:
			set_PWM(0);
			break;
		case ToneC4:
			break;
		case ToneD4:
			break;
		case ToneE4:
			break;
		default:
			State = Start;
			break;
	}
	return state;
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	DDRB = 0xFF; PORTB = 0x00;

	static task task2, task1, task3;
	task *tasks[] = { &task2, &task1, &task3 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	//Task 1
	task2.state = -1; //task init state
	task2.period = 5;
	task2.elapsedTime = task2.period;
	task2.TickFct = &SM1_Tick;


	task1.state = -1; //task init state
	task1.period = 5;
	task1.elapsedTime = task1.period;
	task1.TickFct = &inputTick;
	//Task 2

	task3.state = -1; //task init state
	task3.period = 5;
	task3.elapsedTime = task1.period;
	task3.TickFct = &Tick;
	//Task 2



	//ADC_init();
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
