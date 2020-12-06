/*	Author: Heng Tan
 *  Partner(s) Name: 
 *	Lab Section: 024
 *	Assignment: Lab 13  Exercise 1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *  
 *  Demmo Link: https://youtu.be/zXXjuVLNR8A
 */
#include <avr/io.h>
#include <Timer.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void ADC_init(){
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    unsigned char tmpB = 0;
    unsigned char tmpD = 0;
    unsigned short x;

    /* Insert your solution below */
    ADC_init();
    while (1) {
        x = ADC;
        if((x & 0x03FF) >= 0x01FC && (x & 0x03FF) <= 0x0207){
            x = 0x0200;
        }
        tmpB = (char)x;
        tmpD = (char)(x >> 8) & 0x03;
        PORTC = tmpB;
        PORTD = tmpD;
    }
    return 1;
}