/*
 * CS120BFinalProject.c
 *
 *Alexander Yee
 *CS120B Final Project - Memory Test
 *
 */

//All inclusions are property of UCR 120B
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "bit.h"
#include "scheduler.h"

/****************************/
//Global Shared Variable
unsigned char lockIn = 0x00;
unsigned char finalAnswer = 0x00;
unsigned char startGame = 0x00;
unsigned char index;
unsigned char activateJoystick = 0x00;
unsigned char gameEnd = 0x00;
unsigned char startPoint = 0x00;
/***************************/
//Set Starting Point
unsigned char column_select; // = 0x01;
unsigned char column_val; // = 0x01;
unsigned char col_arr[8];
unsigned char val_arr[8];

/********************************/
// Macros for Button Inputs
/********************************/
#define A2 (~PINA & 0x04)
#define A3 (~PINA & 0x08)
#define StartButton (~PINA & 0x10)

/********************************/
// Macros for Array Sound Mapping
/********************************/
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

/****************************/
//PWM Variables for Frequency
#define freq1  261.63
#define freq2  293.66
#define freq3  329.63
#define freq4  349.23
#define freq5  392.00
#define freq6  440.00
#define freq7  493.88
#define freq8  987.77
/****************************/

//Variables for Array, Equivalent to Frequencies
#define _C4  freq1
#define _D4  freq2
#define _E4  freq3
#define _F4  freq4
#define _G4  freq5
#define _A4  freq6
#define _B4  freq7
#define _LastKey   freq8


/***************************************************************************************/
double songToGuess[] = {_F4, _G4, _LastKey, _LastKey, _A4, _B4, _C4, _E4 };
unsigned char col_sel_default[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
unsigned char col_val_default[] = {0x08, 0x10, 0x80, 0x80, 0x20, 0x40, 0x01, 0x04};
/***************************************************************************************/
//
/***************************************************************************************/
//PWM Functions are property of UCR 120B
void set_PWM();
void PWM_off();
void PWM_on();

//Transmit data functions belong to UCR 120B
void transmit_data(unsigned char data);
void transmit_data1(unsigned char data);
/***********************************************/
void findIndex();
/***********************************************/

void set_PWM(double frequency) {

	static double current_frequency;

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
		PORTC = 0x80;
		PORTC |= (((data >> i) << 4) & 0x10);
		PORTC |= 0x20;
	}
	PORTC |= 0x40;
	PORTC = 0x00;
}

//ADC code sourced from UCR 120B
void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

void findIndex(){
	if(column_select == 0x01){
		index = 0;
	}
	if(column_select == 0x02){
		index = 1;
	}
	if(column_select == 0x04){
		index = 2;
	}
	if(column_select == 0x08){
		index = 3;
	}
	if(column_select == 0x10){
		index = 4;
	}
	if(column_select == 0x20){
		index = 5;
	}
	if(column_select == 0x40){
		index = 6;
	}
	if(column_select == 0x80){
		index = 7;
	}
	col_arr[index] = column_select;
	val_arr[index] = column_val;
	return;
}


/****************************/
// Button Press State Machine
/***************************/
enum inputStates2 {Release, PressStart, Buffer, PressLockIn, PressFinal, Wait};
int inputTick(int state){
	switch(state){ //transition switch
		case Release:
			if(A2){
				state = PressLockIn;
			}
			else if(A3){
				state = PressFinal;
			}
			else if(StartButton){
				state = PressStart;
			}
			else{
				state = Release;
			}
			break;
		case PressStart:
			if(StartButton){
				state = Wait;
			}
			else{
				state = Buffer;
			}
			break;

		case Buffer:
			state = Release;
			break;

		case PressLockIn:
			if(A2){
				state = Wait;
			}
			else{
				state = Release;
			}
			break;

		case PressFinal:
			if(A3){
				state = Wait;
			}
			else{
				state = Release;
			}
			break;

		case Wait:
			if(A2 || A3){
				state = Wait;
			}
			else if(StartButton){
				state = Buffer;
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
			lockIn = 0x00;
			finalAnswer = 0x00;
			startGame = 0x00;
			activateJoystick = 0x00;
			break;

		case PressStart:
			startGame = 0x01;
			break;

		case Buffer:
			column_select = 0x01;
			column_val = 0x01;
			activateJoystick = 0x01;
			break;

		case PressLockIn:
			lockIn = 0x01;
			break;

		case PressFinal:
			finalAnswer = 0x01;
			break;

		case Wait:
			if(lockIn){
				lockIn = 0x01;
			}
			if(finalAnswer){
				finalAnswer = 0x01;
				activateJoystick = 0x01;
			}
			if(startGame){
				startGame = 0x01;
			}

			break;
	}
	return state;
}



/*******************************************/
// Tick function for the Joystick
/*******************************************/
//Joystick Code Source :
//learn.parallax.com/tutorials/language/propeller-c/propeller-c-simple-devices/joystick/
enum SM1_States {WaitActivation, ActivationRelease, ActivateMatrix, DisableMatrix};
int SM1_Tick(int state) {

	ADC_init();
	ADMUX = !ADMUX;
	unsigned short input;
	input = ADC;

	switch (state) {   //Transitions
		case WaitActivation:
			if(activateJoystick){
				state = ActivateMatrix;
			}
			else{
				state = WaitActivation;
			}
			break;

		case ActivationRelease:
			if(!activateJoystick){
				state = ActivateMatrix;
			}
			else{
				state = ActivationRelease;
			}

		case ActivateMatrix:
			if(finalAnswer){
				state = DisableMatrix;
			}
			else{
				state = ActivateMatrix;
			}
			break;

		case DisableMatrix:
			state = WaitActivation;
			break;

		default:
			state = WaitActivation;
			break;
	}

	switch (state) { //Actions
		case WaitActivation:
			break;
		case ActivationRelease:
			column_select = 0x01;
			column_val = 0x01;
			break;

		//LEFT
		case ActivateMatrix:
			if(input < 225 && ADMUX == 0){
				if(column_select <= 0x01){
					column_select = 0x01;
					findIndex();
				}
				else{
					column_select = column_select >> 1;
					findIndex();
				}
			}

			//DOWN
			if(input > 0 && input < 225 && ADMUX == 1){
				if(column_val <= 0x01){
					column_val = 0x01;
					findIndex();
				}
				else{
					column_val = column_val >> 1;
					findIndex();
				}
			}

			//RIGHT
			if(input > 700 && ADMUX == 0){
				if(column_select >= 0x80){
					column_select = 0x80;
					findIndex();
				}
				else{
					column_select = column_select << 1;
					findIndex();
				}
			}

			//UP
			else if(input > 700 && ADMUX == 1){
				if(column_val >= 0x40){
					column_val = 0x80;
					findIndex();
				}
				else{
					column_val = column_val << 1;
					findIndex();
				}
			}
			break;

		case DisableMatrix:
			column_val = 0x00;
			column_select = 0x00;
			break;

		default:
			break;
	}
	if(startPoint){
		transmit_data(~column_select); // PORTC displays column pattern
		transmit_data1(column_val); // PORTC selects column to display pattern
	}
	return state;
};

//Declare array to hold user input
double songArray[8];
enum GameStates{WaitGameStart, Start, PlaySongToGuess, WaitOptions, playSound, confirm, playback, gameOver, displayWin, displayLoss};
int songTick(int state) {
	static unsigned char count;
	static unsigned char k;

	switch(state){
		case WaitGameStart:
		if(startGame){
			count = 0x00;
			k = 0x00;
			state = Start;
		}
		else{
			state = WaitGameStart;
		}
		break;

		case Start:
			startPoint = 0x00;
			state = PlaySongToGuess;
			break;

		case PlaySongToGuess:
			if (count < 85){ //4 second playtime
				state = PlaySongToGuess;
			}
			else{
				set_PWM(0);
				startPoint = 0x01;
				state = WaitOptions;
			}
			break;

		case WaitOptions:
			if(lockIn){
				state = playSound;
			}
			else if(finalAnswer){
				state = confirm;
			}
			else{
				state = WaitOptions;
			}
			break;

		case playSound:
			if(lockIn){
				state = playSound;
			}
			else{
				set_PWM(0);
				state = WaitOptions;
			}
			break;

		case confirm:
			count = 0;
			if(!finalAnswer){
				state = playback;
			}
			else{
				state = confirm;
			}
			break;

		case playback:
			if (count < 85){ //Play for 4
				state = playback;
			}
			else{
				index = 0;
				state = gameOver;
			}
			break;

		case gameOver:
			if(songToGuess[index] == songArray[index]){
				if(index == 7){
					state = displayWin;
				}
			}
			else if(songToGuess[index] != songArray[index]){
				state = displayLoss;
			}
			else{
				state = gameOver;
			}
			break;

		case displayWin:
			state = displayWin;
			break;

		case displayLoss:
			state = displayLoss;
			break;

		default:
			state = WaitGameStart;
			break;
	}
	switch(state){
		case(WaitGameStart):
			PORTB = 0x01; //Assign yellow light on to show idle
			break;

		case Start:
			PORTB = 0x02; //Assign blue light to show game in progress, think about changing to PORTD for output
			break;

		case PlaySongToGuess:
			if (count%5 ==0){
				set_PWM(songToGuess[k]);
				transmit_data(~(col_sel_default[k]));
				transmit_data1(col_val_default[k]);
				k = (k > 7) ? 0 : k+1;
			}
			count++;
			break;

		case WaitOptions:
			set_PWM(0);
			break;

		case playSound:
			if(lockIn){
				if(C4){
					findIndex();
					songArray[index] = freq1;
					set_PWM(freq1);
				}
				if(D4){
					findIndex();
					songArray[index] = freq2;
					set_PWM(freq2);
				}
				if(E4){
					findIndex();
					songArray[index] = freq3;
					set_PWM(freq3);
				}
				if(F4){
					findIndex();
					songArray[index] = freq4;
					set_PWM(freq4);
				}
				if(G4){
					findIndex();
					songArray[index] = freq5;
					set_PWM(freq5);
				}
				if(A4){
					findIndex();
					songArray[index] = freq6;
					set_PWM(freq6);
				}
				if(B4){
					findIndex();
					songArray[index] = freq7;
					set_PWM(freq7);
				}
				if(LastKey){
					findIndex();
					songArray[index] = freq8;
					set_PWM(freq8);
				}
			}
			break;

		case confirm:
			index = 0;
			break;

		case playback:
			if (count%5 ==0){
				set_PWM(songArray[index]);
				transmit_data(~(col_arr[index]));
				transmit_data1(val_arr[index]);
				index = (index > 7) ? 0 : index+1;
			}
			count++;
			break;

		case gameOver:
			set_PWM(0);
			activateJoystick = 0x00;
			index++;
			break;

		case displayWin:
			PORTB = 0x04;
			break;

		case displayLoss:
			PORTB = 0x08;
			break;

		default:
		break;
	}
	return state;
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	static task task1, task2,task3;
	task *tasks[] = { &task1, &task2, &task3};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	//Task 1
	task1.state = -1; //task init
	task1.period = 50;
	task1.elapsedTime = task1.period;
	task1.TickFct = &inputTick;

	//Task 2
	task2.state = -1; //task init
	task2.period = 50;
	task2.elapsedTime = task2.period;
	task2.TickFct = &SM1_Tick;

	//Task 2
	task3.state = -1; //task init
	task3.period = 50;
	task3.elapsedTime = task3.period;
	task3.TickFct = &songTick;

	TimerSet(50);
	TimerOn();
	PWM_on();

	unsigned char i;
	while (1)
	{
		// Scheduler code is property of UCR CS120B
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 50;
		}
		while(!TimerFlag) { }
		TimerFlag = 0;
	}
}
