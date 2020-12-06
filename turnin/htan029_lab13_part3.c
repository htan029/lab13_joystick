/*	Author: Heng Tan
 *  Partner(s) Name: 
 *	Lab Section: 024
 *	Assignment: Lab 13  Exercise 3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *  
 *  Demmo Link: https://youtu.be/sbWNn_1Vo2M
 */
#include <avr/io.h>
#include <Timer.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

// ========= A2D ==============================
void A2D_init(){
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}
// ========= A2D end ==========================

// ========= find GCD =========================
unsigned long int findGCD(unsigned long int a, unsigned long int b){
    unsigned long int c;
    while (1){
        c= a%b;
        if(c==0){return b;}
        a = b; 
        b = c;
    }
    return 0;
}
// ======== end find GCD ======================

// ======== task struct =======================

typedef struct task {
  signed char state; // Current state of the task
  unsigned long int period; // Rate at which the task should tick
  unsigned long int elapsedTime; // Time since task's previous tick
  int (*TickFct)(int); // Function to call for task's tick
} task;

// ======== End Task scheduler data structure =======

// ======== Shared Variables ========================
unsigned char upDown = 1;
unsigned char timeD = 0;
unsigned char row = 0xFE;
unsigned char pattern = 0x80;

// ======== End Shared Variables ====================

// ======== Tasks =============================
enum Joystick_States {update};
int Joystick_Tick(int state){
    static unsigned short x;
    switch(state){
        case update: break;
        default: state = update; break;
    }

    switch(state){
        case update: 
            x = ADC;
            x = (x & 0x03FF);
            if(x > 0x01C8 && x <= 0x0238){
                upDown = 1; //not move
                timeD = 0;
            } else if (x >= 0 && x <= 0x0072){
                upDown = 0; // left 100ms
                timeD = 1;
            } else if (x > 0x0072 && x <= 0x00E4){
                upDown = 0; // left 250ms
                timeD = 2;
            } else if (x > 0x00E4 && x <= 0x0156){
                upDown = 0; // left 500ms
                timeD = 5;
            } else if (x > 0x0156 && x <= 0x01C8){
                upDown = 0; // left 1000ms
                timeD = 10;
            } else if (x > 0x0238 && x <= 0x02AA){
                upDown = 2; // right 1000ms
                timeD = 10;
            } else if (x > 0x02AA && x <= 0x031C){
                upDown = 2; // right 500ms
                timeD = 5;
            } else if (x > 0x031C && x <= 0x038E){
                upDown = 2; // right 250ms
                timeD = 2;
            } else if (x > 0x038E && x <= 0x3FF){
                upDown = 2; // right 100ms
                timeD = 1;
            }
            break;
        default: break;
    }
    return state;
}

enum Button_States {wait, up_state, down_state};
int Button_Tick(int state){
    static unsigned char count = 0;
    switch(state){
        case wait: 
            if(upDown == 0){
                state = down_state;
            } else if (upDown == 2){
                state = up_state;
            } else if (upDown == 1){
                count = 0;
            }
            break;
        case up_state: 
            state = wait;
            break;
        case down_state: 
            state = wait;
            break;
        default: state = wait; break;
    }

    switch(state){
        case wait: break;
        case up_state: 
            if(count >= timeD){
                if(pattern != 0x01){
                    pattern = (pattern >> 1);
                } else {
                    pattern = 0x80;
                }
                count = 0;
            } else {
                count++;
            }
            break;
        case down_state: 
            if(count >= timeD){
                if(pattern != 0x80){
                    pattern = (pattern << 1);
                } else {
                    pattern = 0x01;
                }
                count = 0;
            } else {
                count++;
            }
            break;
        default: break;
    }
    PORTC = pattern;	// Pattern to display
	PORTD = row;		// Row(s) displaying pattern	
    return state;
}
// ====== Tasks struct end ===================

// ====== main ==============================
int main(void) {
    /* Insert DDR and PORT initializations */
    //DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    A2D_init();

    static task task1, task2, task3, task4;
    task *tasks[] = { &task1, &task2};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    const char start = -1;

    tasks[0]->state = start;
    tasks[0]->period = 50;
    tasks[0]->elapsedTime = tasks[0]->period;
    tasks[0]->TickFct = &Joystick_Tick;

    tasks[1]->state = start;
    tasks[1]->period = 50;
    tasks[1]->elapsedTime = tasks[1]->period;
    tasks[1]->TickFct = &Button_Tick;

    unsigned short i;
    unsigned long gcd = tasks[0]->period;
    for(i = 1; i < numTasks; i++){
        gcd = findGCD(gcd, tasks[i]->period);
    }

    TimerSet(gcd);
    TimerOn();
    
    /* Insert your solution below */
    while (1) {
        for(i = 0; i < numTasks; i++){
            if(tasks[i]->elapsedTime == tasks[i]->period){
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += gcd;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 0;
}