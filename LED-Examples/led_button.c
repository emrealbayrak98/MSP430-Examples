#include <msp430.h>

#define RED_LED  BIT0
#define BUTTON  BIT3

void delay_ms(unsigned int ms){
    while (ms){
        __delay_cycles(1000);
        ms--;
    }
}

void main(){
    WDTCTL = WDTPW | WDTHOLD; // Disable the watchdog timer

    P1DIR = RED_LED; // Set Pin 1.0 as an output for the LED
    P1REN = BUTTON; // Enable internal resistor for the button (Pin 1.3)
    P1OUT = BUTTON; // Set initial state of Pin 1.3 to activate the pull-up resistor

    while(1){
        if(!(P1IN & BIT3)){ // Check if the button (Pin 1.3) is pressed
            P1OUT ^= BIT0; // Toggle the state of the LED (Pin 1.0)
        }
        while(!(P1IN & BIT3)); // Wait until the button is released

        delay_ms(50); // Introduce a 50ms delay for button debouncing
    }
}
