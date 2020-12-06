/*	Author: Heng Tan
 *  Partner(s) Name: 
 *	Lab Section: 024
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *  
 *  Demmo Link: 
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

// !!!!!!!!!!!!!!!!!!!!!

// ======== End Shared Variables ====================

// ======== Tasks =============================
enum Joystick_States {update};
int Joystick_Tick(int state){
    static unsigned char tmpC = 0;
    static unsigned char tmpD = 0;
    static unsigned short x;
    switch(state){
        case update: break;
        default: state = update; break;
    }

    switch(state){
        case update: 
            x = ADC;
            tmpC = (char)x;
            tmpD = (char)(x >> 8) & 0x03;
            PORTC = tmpC;
            PORTD = tmpD;
            break;
        default: break;
    }
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
    task *tasks[] = { &task1};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    const char start = -1;

    tasks[0]->state = start;
    tasks[0]->period = 1;
    tasks[0]->elapsedTime = tasks[0]->period;
    tasks[0]->TickFct = &Joystick_Tick;


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