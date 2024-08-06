/*
 * File:   Prueba.c
 * Author: Mateo
 *
 * Created on 12 March 2024, 11:57
 */


#include "Prueba.h"

#define _XTAL_FREQ 4000000      // 4MHz internal oscillator

// Define segment patterns for 0-9
#define SEG_0 0b00111111
#define SEG_1 0b00000110
#define SEG_2 0b01011011
#define SEG_3 0b01001111
#define SEG_4 0b01100110
#define SEG_5 0b01101101
#define SEG_6 0b01111101
#define SEG_7 0b00000111
#define SEG_8 0b01111111
#define SEG_9 0b01101111

// Digit select pins
#define DIGIT_1 PORTDbits.RD0
#define DIGIT_2 PORTDbits.RD1

volatile unsigned char count = 0;
volatile unsigned char display_update = 0;
volatile unsigned char fast_count = 0;

const unsigned char segment_patterns[] = {SEG_0, SEG_1, SEG_2, SEG_3, SEG_4, SEG_5, SEG_6, SEG_7, SEG_8, SEG_9};

void init() {
    TRISD = 0x00;  // Set PORTD as output for digit select
    TRISB = 0x00;  // Set PORTB as output for segments
    // Set up Timer0 for multiplexing
    OPTION_REGbits.TMR0CS = 0;  // Timer0 clock source is internal instruction cycle clock
    OPTION_REGbits.PSA = 0;     // Prescaler is assigned to Timer0
    OPTION_REGbits.PS = 0b011;  // Prescaler 1:16
    TMR0IE = 1;                 // Enable Timer0 interrupt
    
    // Set up external interrupt on RB0
    TRISBbits.TRISB0 = 1;       // Set RB0 as input
    INTCONbits.INT0IE = 1;      // Enable RB0/INT external interrupt
    OPTION_REGbits.INTEDG = 0;  // Interrupt on falling edge
    
    // Enable global and peripheral interrupts
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

void display(unsigned char num) {
    unsigned char tens = num / 10;
    unsigned char ones = num % 10;
    
    // Display tens digit
    DIGIT_1 = 1;
    DIGIT_2 = 0;
    PORTB = segment_patterns[tens];
    __delay_ms(5);
    
    // Display ones digit
    DIGIT_1 = 0;
    DIGIT_2 = 1;
    PORTB = segment_patterns[ones];
    __delay_ms(5);
}

void __interrupt() ISR() {
    if (INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = 0;  // Clear Timer0 interrupt flag
        display_update = 1;     // Set flag to update display in main loop
        
        // Check if button is still pressed for fast count
        if (!PORTBbits.RB0) {
            fast_count++;
            if (fast_count >= 50) {  // Adjust this value to change the fast count rate
                count = (count + 1) % 100;
                fast_count = 0;
            }
        } else {
            fast_count = 0;
        }
    }
    
    if (INTCONbits.INT0IF) {
        INTCONbits.INT0IF = 0;  // Clear external interrupt flag
        count = (count + 1) % 100;  // Increment count and wrap around at 99
        __delay_ms(20);  // Simple debounce
    }
}

void main() {
    init();
    
    while(1) {
        if (display_update) {
            display(count);
            display_update = 0;
        }
    }
}
