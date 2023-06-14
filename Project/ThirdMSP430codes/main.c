#include <msp430.h>
#include <string.h>
#include <stdio.h>

//defines for server
#define ssid "eee"                             //server informations
#define psw  "ele417ele"

//defines for ports
#define setRS P2OUT   |= BIT4
#define clearRS P2OUT &= ~BIT4
#define setEN P1OUT   |= BIT5
#define clearEN P1OUT &= ~BIT5

//Esp variables
unsigned char stringTx[100];
unsigned char stringRx[100];
unsigned char test[50];
volatile unsigned int index = 0;
volatile unsigned int len   = 0;
volatile unsigned  int i    = 0;
volatile unsigned char c;
volatile unsigned  int len2 = 0;
unsigned int state = 0;
unsigned int flag = 0;
volatile unsigned int countd =0;

//Lcd functions
void lcdInit();
void lcdCmd(unsigned char cmd);
void lcdData(unsigned char data);
void lcdPos(unsigned char x,unsigned char y);
void lcdStr(unsigned char x,unsigned char y,const unsigned char *string);
void startdelay();

 //Uart and ESP functions
void SendData(unsigned char *sPtr);
void writeByte(unsigned  char byte);
void UartInit(void);
int is_substring(unsigned const char *string1,unsigned const char *string2);
void flush();
void delay_ms(unsigned int  time);
void ESP_Init();

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;
    UartInit();//Uart ayarlari cagirildi
    P1DIR|=BIT5;                             //LCD ports connections
    P2DIR|=BIT0|BIT1|BIT2|BIT3|BIT4;
    startdelay();
    lcdInit();
    lcdStr(1,1,"Temp:");                    //This is writing template of temp and humidity to the LCD
    lcdStr(1,8,".");
    lcdStr(1,11,"C");
    lcdStr(2,1,"Humidity:");
    lcdStr(2,10,"\x25");
    lcdStr(2,13,".");
    __bis_SR_register(GIE);                 // Enter LPM3 w/ int until Byte RXed
    ESP_Init();
    while(1);
}
void ESP_Init()
{
    sprintf(test, "OK");

    sprintf(stringTx, "AT+RST\r\n");        //wait until esp work properly
    SendData(stringTx);
    delay_ms(5000);                         //reset ESP

    flush();

    sprintf(stringTx, "AT\r\n");            //wait until esp work properly
    SendData(stringTx);
    delay_ms(3000);                         //test for ESP ready or not
    while (!is_substring(test,stringRx));
    flush();

    sprintf(stringTx, "AT+CWMODE=3\r\n");    //wait until esp work properly
    SendData(stringTx);                     //open wifi settings
    delay_ms(3000);
    while (!is_substring(test,stringRx));
    flush();

    sprintf(stringTx,  "AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,psw);//wait until esp work properly
    SendData(stringTx);                                      //set ssid and password for connection
    delay_ms(3000);
    while (!is_substring(test,stringRx));
    flush();

    sprintf(stringTx, "AT+CIPMUX=1\r\n");   //wait until esp work properly
    SendData(stringTx);
    delay_ms(3000);                         //for multi-ip connection
    while (!is_substring(test,stringRx));
    flush();

    sprintf(stringTx, "AT+CIPSERVER=1,80\r\n");             //wait until esp work properly
    SendData(stringTx);                                     //creates TCP server
    delay_ms(3000);
    while (!is_substring(test,stringRx));
    flush();
    state = 1;
}
//Configure Uart
void UartInit(void)
{
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                            // 1MHz 115200
    UCA0BR1 = 0;                              // 1MHz 115200
    UCA0MCTL = UCBRF_0 + UCBRS_1;             // Modulation UCBRSx = 5
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine
    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}
//Send Data
void SendData(unsigned char *sPtr)
{
    for(;*sPtr!='\0';writeByte(*(sPtr++)));
}
void writeByte(unsigned char byte)
{
    while(!(IFG2 & UCA0TXIFG));                      // wait for TX buffer to be empty
    UCA0TXBUF = byte;
}
int is_substring(unsigned const char *string1,unsigned const char *string2) {
    return strstr(string2, string1) != NULL;        //for data correction
}
//add delay
void delay_ms(unsigned int time)
{
    while(time)
    {
    __delay_cycles(1000);                           //By default clock is 1 MHZ. Hence 1 tick is 1us. 1ms means 1000 ticks.

    --time;
    }
}
void flush()
{
    volatile unsigned int del = 0;
    while(del < 100)                               //delete string rx content
    {
    stringRx[del] = ' ';
    ++del;
    }
    index = 0;
}
//LCD configurations
void startdelay(){
    __delay_cycles(1000);                          //This delay cycle used for LCD delay requirement depending on the datasheet
}
void lcdInit(){
    lcdCmd(0x33);   //datasheet says send 0x3x three times
    lcdCmd(0x32);   //after that send a 0x2x
    lcdCmd(0x28);   //4 bit interface 2lines 5x8 font setting   This part for initializing the lcd depend on the datasheet restricitions
    lcdCmd(0x01);   //clear display
    lcdCmd(0x0C);   //Display on, curser off, curser blink off
}
void lcdCmd(unsigned char cmd){
    clearRS;                               //register select 0 to instruction
    P2OUT = (P2OUT & 0xF0) | (cmd >> 4);   // first 4 bit (MSB)
    startdelay();
    setEN;                                 //enable comm.
    startdelay();
    clearEN;                              //This part used for LCD settings
    startdelay();
    P2OUT = (P2OUT & 0xF0) | (cmd & 0x0F);   //last 4 bit (LSB)
    setEN;
    startdelay();
    clearEN;
    startdelay();
}
void lcdData(unsigned char data){
    setRS;
    P2OUT = (P2OUT & 0xF0) | (data >> 4);
    startdelay();
    setEN;
    startdelay();
    clearEN;                       //this part writing only 1 character
    startdelay();
    P2OUT = (P2OUT & 0xF0) | (data & 0x0F);
    setEN;
    startdelay();
    clearEN;
    startdelay();
}
void lcdPos(unsigned char x,unsigned char y){
    lcdCmd(0x80 | (x-1) << 6 | (y-1) );           //This part for setting lcd cursor position
}
void lcdStr(unsigned char x,unsigned char y,const unsigned char *string){
    lcdPos(x,y);
    unsigned short i=0;                           //This part for writing string step by step
    while(string[i] != 0 && i<16 ){
        lcdData(string[i]);
        i++;
    }
}
//Receive Data
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    if(IFG2 & UCA0RXIFG){
     c = UCA0RXBUF;
    if(!state){
     stringRx[index++] = c;
    }
    else{
     stringRx[index++] = c;
    if( ((stringRx[index-9] == 'P') &&  (stringRx[index-8] == 'D' )  &&  (stringRx[index-2] == ':' )) || (flag)){
     flag=1;
     test[len2++] = c;

    if(len2 >=13){
     IE2 &= ~UCA0RXIE;          // disable USCI_A0 RX interrupt
     len2 = 0;
     index = 0;
     flag = 0;
     state = 1;
     ++countd;
                                //LCD display
     lcdPos(2,11);
     lcdData(test[4]);
     lcdPos(2,12);              //humidity
     lcdData(test[5]);
     lcdPos(2,14);
     lcdData(test[6]);
     lcdPos(2,15);
     lcdData(test[7]);

     lcdPos(1,6);               //temparature
     lcdData(test[0]);
     lcdPos(1,7);
     lcdData(test[1]);
     lcdPos(1,9);
     lcdData(test[2]);
     lcdPos(1,10);
     lcdData(test[3]);

     IE2 |= UCA0RXIE;           // enable USCI_A0 RX interrupt
     }}}
    if(index >100){
     index = 0;
     }}}
