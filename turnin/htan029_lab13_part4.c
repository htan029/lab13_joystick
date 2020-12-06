/*	Author: Heng Tan
 *  Partner(s) Name: 
 *	Lab Section: 024
 *	Assignment: Lab 13  Exercise 4
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *  
 *  Demmo Link: https://youtu.be/XQNO54hTpkY
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
void Set_A2D_Pin(unsigned char pinNum){
    ADMUX = (pinNum <= 0x07) ? pinNum :  ADMUX;
    static unsigned char i = 0;
    for(i = 0; i < 15; i++) asm("nop");
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
unsigned char upDown = 5;
unsigned char leftRight = 5;
unsigned char row = 0xFE;
unsigned char pattern = 0x80;

// ======== End Shared Variables ====================

// ======== Tasks =============================
enum Joystick_States {update};
int Joystick_Tick(int state){
    static unsigned short x;
    static unsigned char flex = 0x00;
    switch(state){
        case update: 
            flex = ~flex; // 0x00 = up and down, 0xFF = left and right
            if(flex == 0x00){
                Set_A2D_Pin(0x00);
            } else if (flex == 0xFF){
                Set_A2D_Pin(0x07);
            }
            break;
        default: state = update; break;
    }

    switch(state){
        case update:
            x = ADC;
            x = (x & 0x03FF);
            if(flex == 0xFF){ // up and down
                if (x >= 0x01C8 && x < 0x0238){
                    upDown = 5;
                } else if(x >= 0 && x < 0x01C8){ // down
                    upDown = 8;
                } else if(x >= 0x0238 && x <= 0x03FF){
                    upDown = 2;
                }
            } else if (flex == 0x00){ // left and right
                if (x >= 0x01C8 && x < 0x0238){
                    leftRight = 5;
                } else if(x >= 0 && x < 0x01C8){ // left
                    leftRight = 4;
                } else if(x >= 0x0238 && x <= 0x03FF){ // right
                    leftRight = 6;
                }
            }
            break;
        default: break;
    }
    return state;
}

enum Button_States {wait, up_state, down_state, left_state, right_state};
int Button_Tick(int state){
    static unsigned char count = 0;
    switch(state){
        case wait: 
            if(upDown == 8){
                state = down_state;
            } else if (upDown == 2){
                state = up_state;
            } else if (leftRight == 4){
                state = left_state;
            } else if (leftRight == 6){
                state = right_state;
            }
            break;
        case up_state: 
            state = wait;
            break;
        case down_state: 
            state = wait;
            break;
        case left_state:
            state = wait;
            break;
        case right_state:
            state = wait;
            break;
        default: state = wait; break;
    }

    switch(state){
        case wait: break;
        case right_state: 
            if(pattern != 0x01){
                pattern = (pattern >> 1);
            }
            break;
        case left_state: 
            if(pattern != 0x80){
                pattern = (pattern << 1);
            }
            break;
        case up_state:
            if(row != 0xFE){
                row = (row >> 1) | 0x80;
            }
            break;
        case down_state:
            if(row != 0xEF){
                row = (row << 1) | 0x01;
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