//New Version 6/5 4:12PM
//This version of the program has the tasks divided up
//Code has been cleaned up
//New states for game logic has been added, still need to fix timing
//Still need to program the LED to display the lights of songToGuess
//Need to add states in a state machine to assign output for game win/loss/gameOn/game inprogress

//This test bench includes PWM, Joystick and LED
//3 State Machines
//PWM triggered on input
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
/***************************/
//Set Starting Point
unsigned char column_select = 0x01;
unsigned char column_val = 0x01;

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
#define _R   freq8
#define _C5 523.25

//double songToGuess[] = {_C4, _C4, _C4, _G4, _G4, _G4, _F4, _E4};
double songToGuess[] = { _C4, _C4, _C4, _G4, _G4,
	_G4, _F4, _E4, _D4, _C5,
	_C5, _C5, _G4, _G4, _F4,
	_E4, _D4, _C5, _C5, _C5,
	_G4, _G4, _F4, _E4, _F4,
_D4, _D4, _D4, _D4, _D4};

/**********************************/
void set_PWM();
void PWM_off();
void PWM_on();
void transmit_data(unsigned char data);
void transmit_data1(unsigned char data);
/**********************************/

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
}


/************************/
// Button Press State Machine
/************************/
enum inputStates2 {Release, PressStart, PressLockIn, PressFinal, Wait};
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
				state = Release;
			}
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
			if(A2 || A3 || StartButton){
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
			lockIn = 0x00;
			finalAnswer = 0x00;
			startGame = 0x00;
			activateJoystick = 0x00;
			//temp = 0x00;
			break;
		case PressStart:
			activateJoystick = 0x01;
			startGame = 0x01;

			break;

		case PressLockIn:
		//PORTB = 0x01;
			lockIn = 0x01;
			//temp = 0x01;
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

			//activateJoystick = 0x01;
			//PORTB
			break;
	}
	return state;
}



/*******************************************/
// Tick function for the Joystick
/*******************************************/
//enum SM1_States {sm1_display, playSound, confirm, playback, gameOver};
enum SM1_States {WaitActivation, ActivationRelease, sm1_display};
int SM1_Tick(int state) {

	ADC_init();
	ADMUX = !ADMUX;
	unsigned short input;
	input = ADC;
	//static unsigned char count;


	switch (state) {   //Transitions
		case WaitActivation:
			if(activateJoystick){
				state = sm1_display;
			}
			else{
				state = WaitActivation;
			}
			break;
		case ActivationRelease:
			if(!activateJoystick){
				state = sm1_display;
			}
			else{
				state = ActivationRelease;
			}

		case sm1_display:
			state = sm1_display;
			break;


		default:
			state = WaitActivation;
			break;
	}

	switch (state) { //Actions
		case WaitActivation:
			break;
		case ActivationRelease:
			break;

		case sm1_display:
			if(input < 225 && ADMUX == 0){ //left
				if(column_select <= 0x01){
					column_select = 0x01;
					findIndex();
				}
				else{
					column_select = column_select >> 1;
					findIndex();
				}
			}

			if(input > 0 && input < 225 && ADMUX == 1){ //down
				if(column_val <= 0x01){
					column_val = 0x01;
					findIndex();
				}
				else{
					column_val = column_val >> 1;
					findIndex();
				}
			}
			if(input > 700 && ADMUX == 0){ //right
				if(column_select >= 0x80){
					column_select = 0x80;
					findIndex();
				}
				else{
					column_select = column_select << 1;
					findIndex();
				}
			}
			else if(input > 700 && ADMUX == 1){ //up
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

		default:
		break;
	}
	transmit_data(~column_select); // PORTC displays column pattern
	transmit_data1(column_val); // PORTC selects column to display pattern
	return state;
};

//Declare array to hold user input
double songArray[8];
enum GameStates{WaitGameStart, Start, PlaySongToGuess, WaitOptions, playSound, confirm, playback, gameOver};
int songTick(int state) {
	static unsigned char count;
	switch(state){
		case(WaitGameStart):
			if(startGame){
				//set_PWM(27.50);
				state = Start;
			}
			else{
				state = WaitGameStart;
			}
			break;

		case Start:
			//set_PWM(0);
			state = PlaySongToGuess;
			break;

		case PlaySongToGuess:
			state = WaitOptions;
			break;

		case WaitOptions:
			if(lockIn){
				state = playSound;
			}
			else if(finalAnswer){
				state = confirm;
			}
			else{
				state = Start;
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
			index = 0;
			//set_PWM(27.50);
			if(!finalAnswer){
				state = playback;
			}
			else{
				state = confirm;
			}
			break;

		case playback:
			if (count < 85){ // i < 4s
				state = playback;
			}
			else{
				state = gameOver;
			}
			//state = gameOver;
			break;

		case gameOver:
			state = WaitGameStart;
			break;

		default:
			state = WaitGameStart;
			break;
	}
	switch(state){
		case(WaitGameStart):
			//set_PWM(0);
			break;

		case Start:
// 			if (count%5 ==0){
// 				set_PWM(songToGuess[index]);
// 				index = (index > 7) ? 0 : index+1;
// 			}
// 			count++;
			//set_PWM(0);
			break;

		case PlaySongToGuess:
			if (count < 30){
					set_PWM(songToGuess[index]);
	 				index = (index > 30) ? 0 : index+1;
			}
			count++;
			if(count > 30){
				set_PWM(0);
			}
			break;

		case WaitOptions:
			break;

		case playSound:
			if(lockIn){
				//if((column_val == 0x01 && column_select == 0x01)){
				if(C4){
					//findIndex();
					songArray[index] = freq1;
					set_PWM(freq1);
				}
				if(D4){
					//findIndex();
					songArray[index] = freq2;
					set_PWM(freq2);
				}
				if(E4){
					//findIndex();
					songArray[index] = freq3;
					set_PWM(freq3);
				}
				if(F4){
					//findIndex();
					songArray[index] = freq4;
					set_PWM(freq4);
				}
				if(G4){
					//findIndex();
					songArray[index] = freq5;
					set_PWM(freq5);
				}
				if(A4){
					//findIndex();
					songArray[index] = freq6;
					set_PWM(freq6);
				}
				if(B4){
					//findIndex();
					songArray[index] = freq7;
					set_PWM(freq7);
				}
				if(LastKey){
					//findIndex();
					songArray[index] = freq8;
					set_PWM(freq8);
				}
			}
			break;

		case confirm:
			index = 0;
			//set_PWM(27.50);
			break;


		case playback:
			if (count%5 ==0){
				set_PWM(songArray[index]);
				index = (index > 7) ? 0 : index+1;
			}
			count++;
			break;

		case gameOver:
			set_PWM(0);
			column_select = 0x00;
			column_val = 0x00;
			PORTB = 0x07;
			activateJoystick = 0x00;

			break;

		default:
		break;
	}
	transmit_data(~column_select); // PORTC displays column pattern
	transmit_data1(column_val); // PORTC selects column to display pattern
	return state;
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	DDRB = 0xFF; PORTB = 0x00;

	static task task2, task1, task3;
	task *tasks[] = { &task2, &task1, &task3};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	//Task 1
	task2.state = -1; //task init state
	task2.period = 50;
	task2.elapsedTime = task2.period;
	task2.TickFct = &SM1_Tick;


	task1.state = -1; //task init state
	task1.period = 50;
	task1.elapsedTime = task1.period;
	task1.TickFct = &inputTick;
	//Task 2

	task3.state = -1; //task init state
	task3.period = 50;
	task3.elapsedTime = task3.period;
	task3.TickFct = &songTick;
	//Task 2


	//ADC_init();
	TimerSet(50);
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
			tasks[i]->elapsedTime += 50;
		}
		while(!TimerFlag) { }
		TimerFlag = 0;
	}
}
