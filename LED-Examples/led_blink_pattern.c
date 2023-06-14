#include <msp430.h>

#define RED_LED  BIT0
#define LONG_DELAY 800
#define SHORT_DELAY 200

void delay_ms(unsigned int ms){
    while (ms){
        __delay_cycles(1000);
        ms--;
    }
}

void main(){
    WDTCTL = WDTPW | WDTHOLD; // Disable the watchdog timer

    P1DIR = RED_LED; // Set Pin 1.0 as an output for the LED
    P1OUT &= ~P1OUT; // Turn off the LED.

    while(1){
        P1OUT ^= RED_LED; // Toggle the state of the LED (Pin 1.0)
        delay_ms(SHORT_DELAY); // Introduce a short delay

        P1OUT ^= RED_LED; // Toggle the state of the LED again
        delay_ms(SHORT_DELAY); // Introduce another short delay

        P1OUT ^= RED_LED; // Toggle the state of the LED
        delay_ms(LONG_DELAY); // Introduce a long delay

        P1OUT ^= RED_LED; // Toggle the state of the LED again
        delay_ms(LONG_DELAY); // Introduce another long delay
    }
}
