#include <msp430.h> 
#define short 14000 //define short delay
#define long 39000 //define long delay

volatile unsigned int counter=1; //global volatile variable to use in interrupt
volatile unsigned int delays[]={short,short,long,long}; //an array containing the delays in order
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    P1OUT &=~BIT0;
    P1DIR |= BIT0;
    P1OUT |= BIT0; // LED is ON initially

    __bis_SR_register(GIE); // Enable CPU interrupts
    TACCR0 = short; // Set as short delay initially
    TACCTL0 = CCIE; // Enable Timer A interrupts
    TACTL |= TASSEL_1 | MC_1 | TACLR; // ACLK Counts UP and 1s

    while(1);

    return 0;
}
#pragma vector = TIMER0_A0_VECTOR; // Vector table for Timer A0 ISR
__interrupt void TA0_ISR(void){
    P1OUT ^= BIT0; // Switch off, on each time an interrupt occur
    TACCR0 = delays[counter]; // pick delay from array
    counter++;
    if(counter >3){ // if counter exceeds the array size which means starting from over
        counter = 0;
    }
}
