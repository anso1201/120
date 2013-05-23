/*	[cslogin]_lab8_part2.c - $date$
 *	Name & E-mail:  - 
 *	CS Login: 
 *	Partner(s) Name & E-mail:  - 
 *	Lab Section: 
 *	Assignment: Lab #  Exercise # 
 *	Exercise Description:
 *	
 *	
 *	I acknowledge all content contained herein, excluding template or example 
 *	code, is my own original work.
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <bit.h>
#include <timer.h>
#include <lcd_8bit_task.h>
#include <scheduler.h>

enum LT_States { LT_init, LT_s0, LT_WaitLcdRdy, LT_WaitButton, LT_FillAndDispString,
	LT_HoldGo1, LT_WaitBtnRelease,  LT_FADS1, LT_HoldGo2, LT_WaitBtnRelease2, LT_FAD2,
LT_HoldGo3, LT_WaitBtnRelease3 } LT_State;

int LT_Tick(int LT_State) {
	static unsigned short j;
	static unsigned char i, x, c;
	
	
	switch(LT_State) { // Transitions
		case LT_init:
		LT_State = LT_s0;
		break;
		case LT_s0:
		LT_State = LT_WaitLcdRdy;
		break;
		case LT_WaitLcdRdy:
		if (!LCD_rdy_g) {
			LT_State = LT_WaitLcdRdy;
		}
		else if (LCD_rdy_g) {
			LT_State = LT_WaitButton;
		}
		break;
		case LT_WaitButton:
		if (GetBit(PINA,0)==1) {
			LT_State = LT_WaitButton;
		}
		else if (GetBit(PINA,0)==0) { // Button active low
			LT_State = LT_FillAndDispString;
		}
		break;
		case LT_FillAndDispString:
		LT_State = LT_HoldGo1;
		break;
		case LT_HoldGo1:
		LCD_go_g=0;
		LT_State = LT_WaitBtnRelease;
		break;
		case LT_WaitBtnRelease:
		if (GetBit(PINA,0)==1) { // Wait for button release
			LT_State = LT_WaitBtnRelease;
		}
		else if (GetBit(PINA,0)==0) {
			LT_State = LT_FADS1; //LT_WaitLcdRdy;
		}
		break;
		case LT_FADS1:
		LT_State = LT_HoldGo2;
		break;
		case LT_HoldGo2:
		LCD_go_g = 0;
		LT_State = LT_WaitBtnRelease2;
		break;
		case LT_WaitBtnRelease2:
		if(GetBit(PINA, 0)==1) {
			LT_State = LT_WaitBtnRelease2;
		}
		else if(GetBit(PINA,0) ==0) {
			LT_State = LT_FAD2;
		}
		break;
		case LT_FAD2:
		LT_State = LT_HoldGo3;
		break;
		case LT_HoldGo3:
		LCD_go_g = 0;
		LT_State = LT_WaitBtnRelease3;
		break;
		case LT_WaitBtnRelease3:
		if(GetBit(PINA, 0)==1) {
			LT_State = LT_WaitBtnRelease3;
		}
		else if(GetBit(PINA,0) ==0) {
			LT_State = LT_WaitLcdRdy;
		}
		break;
		default:
		//LT_State = LT_s0;
		break;
	} // Transitions

	switch(LT_State) { // State actions
		case LT_s0:
		LCD_go_g=0;
		strcpy(LCD_string_g, "                "); // Init, but never seen, shows use of strcpy though
		break;
		case LT_WaitLcdRdy:
		strcpy(LCD_string_g, "                ");
		break;
		case LT_WaitButton:
		break;
		case LT_FillAndDispString:
		strcpy(LCD_string_g, "CS120B is Legend");
		
		//LCD_string_g[17] = '\0'; // End-of-string char
		LCD_go_g = 1; // Display string
		break;
		case LT_HoldGo1:
		LCD_go_g = 0;
		break;
		case LT_WaitBtnRelease:
		//strcpy(LCD_string_g, "                ");
		break;
		case LT_FADS1:
		strcpy(LCD_string_g, "... wait for it ");
		LCD_go_g = 1;
		break;
		case LT_HoldGo2:
		LCD_go_g = 0;
		break;
		
		case LT_WaitBtnRelease2:
		break;
		case LT_FAD2:
		strcpy(LCD_string_g, " Dary!         ");
		LCD_go_g = 1;
		break;
		case LT_HoldGo3:
		LCD_go_g = 0;
		break;
		case LT_WaitBtnRelease3:
		break;
		default:
			strcpy(LCD_string_g, "                ");//
		break;
	} // State actions
	return LT_State;
}


int main()
{
	DDRB = 0xFF;	// Set port B to output
	DDRD = 0xFF;	// Set port D to output
	DDRA = 0x00;	PORTA = 0xFF;	// Set port A to input

	// Period for the tasks
	unsigned long int SMTick1_calc = 10;
	unsigned long int SMTick2_calc = 60;
	
	
	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
	//tmpGCD = findGCD(tmpGCD, SMTick3_calc);

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;
	
	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTick2_period = SMTick2_calc/GCD;

	//Declare an array of tasks
	static task task1, task2;
	task *tasks[] = { &task1, &task2 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	// Task 1
	task1.state = LI_i;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.TickFct = &LCDI_SMTick;//Function pointer for the tick.

	// Task 2
	task2.state = LT_init;//Task initial state.
	task2.period = SMTick2_period;//Task Period.
	task2.elapsedTime = SMTick2_period;//Task current elapsed time.
	task2.TickFct = &LT_Tick;//Function pointer for the tick.

	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();

	unsigned short i; // Scheduler for-loop iterator
	while(1) {
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