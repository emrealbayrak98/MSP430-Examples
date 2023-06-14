#include <msp430.h>

volatile unsigned int counter=1; // global variable to use in interrupt
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    P1OUT &=~BIT0;
    P1DIR |= BIT0; // P1.0 (LED) as an input

    __bis_SR_register(GIE); // Enable interrupts
    TACCR0 = 200; // A small arbitrary number
    TACCTL0 = CCIE; // Timer interrupts enabled
    TACTL |= TASSEL_1 | MC_1 | TACLR; // configured to count UP,ACLK so 1s

    while(1);

    return 0;
}
#pragma vector = TIMER0_A0_VECTOR; // Vector Table for ISR
__interrupt void TA0_ISR(void){
    P1OUT ^= BIT0; // LED is on the first time,then XOR always
    if((TACCR0<32768) && (counter==0)){ //to make it go up until max value of TACCR0 which is 65535
        TACCR0 = TACCR0 * 2; //double the timer each time if the cycle is completed.
        counter=1;
    }
    else if((TACCR0<32768) && (counter==1)){
        counter=0;
        }
    else{
        TACCR0 = 200; //if it will exceed the max value reset.
        counter = 0;
    }
}
