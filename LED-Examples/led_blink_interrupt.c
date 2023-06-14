#include <msp430.h>
#define short 18000
#define long 39000
int main(void){
    WDTCTL = WDTPW | WDTHOLD; // stop watch dog timer

    TACCR0 = short;//short delay first
    TACTL = TASSEL_1 | MC_1 | TACLR;//configure and reset timer A beforehand
    P1OUT &= ~BIT0; // Led on P1.0 is turned off before setting as output for precaution
    P1DIR |= BIT0; // P1.0 is configured as output
    P1OUT |= BIT0;// LED is ON initially
    volatile unsigned int delay[]={short,short,long,long};
    volatile unsigned int counter=1;
    while(1){
       if(TACTL&TAIFG==TAIFG){//if timer A interrupt flag set
        P1OUT^=BIT0;//toggle led
        TACTL&=~TAIFG;//clear timer A interrupt flag

        TACCR0=delay[counter];//set delay different each cycle
        counter++;
       }
       if(counter>3){
           counter=0;//start from beginning
       }
    }
}
