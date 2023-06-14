
#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "aes.h"

unsigned const char key[]   = {'E', 'M', 'R','E', 'S', 'E','R', 'I',
                               'F','E', 'K', 'R','E', 'M', '0','6'};


/**********VARIABLES******************/

//NETWORK INFORMATION
#define ssid    "eee"
#define psw    "ele417ele"


unsigned char stringTx[50]; //TRANSMIT  DATA
unsigned char stringRx[50]; //RECEIVED  DATA
unsigned char data[16];     //ENCRYPTED DATA
unsigned char test[5];      //CHECK     DATA FOR ESP8266

volatile unsigned  int  index    = 0; //INDEX OF RECEIVED DATA
volatile unsigned  char c;            //RECEIVED CHARACTER
volatile unsigned  int  dataCnt  = 0; //ENCRYPTED DATA COUNTER
volatile unsigned  int  flag1    = 0; //FLAG FOR RECEIVE
volatile unsigned  int  flag2    = 0; //FKAG FOR RECEIVE


/********************FUNCTIONS*************************/

//HELPER FUNCTIONS
int is_substring(unsigned const char *string1,unsigned const char *string2);
void flush();
void delay_ms(unsigned int  time);

//SEND DATA BYTE BY BYTE
void SendData(unsigned char *sPtr);
void writeByte(unsigned  char byte);

//INITIALIZE UART
void UartInit(void);

//INITIALIZE ESP826 FOR RECEIVING
void ESP_Init();


//INITIALIZE ESP826 FOR TRANSMITTING
void ESP_Init2();


int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;


    UartInit();
    __bis_SR_register(GIE);
    ESP_Init();
    while(1);
}

void ESP_Init()
{
    //WIFI mode station, AP, station + AP
    sprintf(test,"OK");
    sprintf(stringTx, "AT+CWMODE=3\r\n");//wait until esp work properly
    SendData(stringTx);
    delay_ms(3000);
    while (!is_substring(test,stringRx));
    flush();

    //Commands ESP8266 to connect a SSID with supplied password.
    sprintf(stringTx,  "AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,psw);//wait until esp work properly
    SendData(stringTx);
    delay_ms(3000);
    while (!is_substring(test,stringRx));
    flush();

    //Enable multiple connections
    sprintf(stringTx, "AT+CIPMUX=1\r\n");//wait until esp work properly
    SendData(stringTx);
    delay_ms(3000);
    while (!is_substring(test,stringRx));
    flush();

    //Start a connection until as client with third ESP8266.
    do{
        flush();
        sprintf(stringTx, "AT+CIPSTART=1,\"TCP\",\"192.168.30.246\",80\r\n");//wait until esp work properly
        SendData(stringTx);
        delay_ms(3000);
       }while (!is_substring(test,stringRx));

    //Configure ESP8266 as server for receiving data for first ESP8266
    sprintf(stringTx, "AT+CIPSERVER=1,80\r\n");//wait until esp work properly
    SendData(stringTx);
    delay_ms(3000);
    while (!is_substring(test,stringRx));
    flush();

    //INITIALIZE IS OVER
    flag1 = 1;
}

void ESP_Init2()
{
    //Set length of the data that will be sent.
    sprintf(stringTx, "AT+CIPSEND=1,16\r\n");//wait until esp work properly
    SendData(stringTx);
    delay_ms(500);

    //SEND DECRYPTED DATA
    SendData(data);

    //INITIALIZE ALL VARIABLES INTO DEFAULT STATE
    flush();
    volatile unsigned int cnt = 0;
    cnt = 15;
    while(cnt < 16)
    {
        data[cnt] = ' ';
        --cnt;
    }
    dataCnt  = 0;
    index = 0;
    flag2  = 0;
    flag1 = 1;

    //OPEN RECEIVE INTERRUPT
    IE2 |= UCA0RXIE;
}


void UartInit(void)
{
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                            // 1MHz 9600
    UCA0BR1 = 0;
    UCA0MCTL = UCBRF_0 + UCBRS_1;
    UCA0CTL1 &= ~UCSWRST;
    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}



//Receive Data
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    if(IFG2 & UCA0RXIFG)
        {
            c = UCA0RXBUF;
                if(!flag1)
                {
                    stringRx[index++] = c;
                }
                else
                {
                    stringRx[index++] = c;
                    //IF STRING CONSIST OF  IPD : THEN DATA RECEIVING STARTS
                    if( ((stringRx[index-9] == 'P') &&  (stringRx[index-8] == 'D' )  &&  (stringRx[index-2] == ':' )) || (flag2))
                    {
                        flag2=1;
                        data[dataCnt++] = c;

                        //IF RECEIVING DATA LENGTH IS 16, OVER THE RECEIVING DATA
                        if(dataCnt >=16)
                        {
                            //CLOSE THE RECEIVE INTERRUPT
                            IE2 &= ~UCA0RXIE;
                            //DECRYPTE RECEIVED DATA
                            //aes_decrypt(data,key);

                            //TRANSMIT THE DECRYPTED DATA TO THE THIRD ESP8266
                            ESP_Init2();
                         }

                    }
                }
                //INITIALIZE THE RECEIVED STRING ARRAY
                if(index >50)
                {
                    index = 0;
                }

       }
}
void SendData(unsigned char *sPtr)
{
    for(;*sPtr!='\0';writeByte(*(sPtr++)));


}

void writeByte(unsigned char byte)
{
    while(!(IFG2 & UCA0TXIFG)); // wait for TX buffer to be empty
    UCA0TXBUF = byte;
}


int is_substring(unsigned const char *string1,unsigned const char *string2) {
  return strstr(string2, string1) != NULL;
}


void delay_ms(unsigned int time)
{
    while(time)
    {
        __delay_cycles(1000);  //By default clock is 1 MHZ. Hence 1 tick is 1us. 1ms means 1000 ticks.
        --time;
    }
}


void flush()
{
    volatile unsigned int del = 0;
    while(del < 5)
    {
        stringRx[del] = ' ';
        ++del;
    }
    index = 0;
}

