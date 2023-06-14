#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "aes.h"

#define ssid "eee"  //ssid used for ESP
#define psw "ele417ele" //pw used for ESP

/******* VERIABLE INITIALIZATIONS*******/
volatile unsigned int sensor_read = 0;
volatile int temp[50];
volatile int diff[50];
volatile unsigned int i=0;
volatile unsigned int j=0;
volatile int hh = 0;
volatile int hl = 0;
volatile int th = 0;
volatile int tl = 0;
volatile unsigned int flag = 0;
unsigned char stringTx[50];
unsigned char data[] = {'2', '5', '3', '8', '3', '5', '2', '2',
                        0x88, 0x99, 0xaa, 0xbb, 0xcc,0xaa, 0xbb, 0xcc};
unsigned const char key[]   = {'E', 'M', 'R','E', 'S', 'E','R', 'I',
                               'F','E', 'K', 'R','E', 'M', '5','8'}; //key for AES

/******* FUNCTION INITIALIZATIONS*******/
void get_values(void);
void pull_values(void);
void sensor(void);
void init(void);
void SendData(unsigned char *sPtr);
void writeByte(unsigned  char byte);
void UartInit(void);
void delay_ms(unsigned int  time);
void ESP_Init(void);
void ESP_con(void);

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; //Stop Watchdog

    UartInit(); //Function to configurate UART
    __bis_SR_register(GIE); //enable interrups
    ESP_Init(); //Function to initialize ESP
    ESP_con(); //Continuous ESP function
}

void ESP_con()
{
    sensor();
    pull_values();
    aes_encrypt(data,key);
    sprintf(stringTx, "AT+CIPSEND=1,16\r\n");
    SendData(stringTx);
    delay_ms(500);
    SendData(data);
    WDTCTL = WDT_MRST_0_064;
}

void ESP_Init()
{

    sprintf(stringTx,  "AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,psw);//wait until esp work properly
    SendData(stringTx);
    delay_ms(3000);
    while (!is_substring(test,stringRx));
    flush();

    sprintf(stringTx, "AT+CIPMUX=1\r\n");// Configure ESP as multiple input
    SendData(stringTx); //send to ESP
    delay_ms(1000);

    sprintf(stringTx, "AT+CIPSTART=1,\"TCP\",\"192.168.30.6\",80\r\n"); //Start ESP connection with TCP and PORT
    SendData(stringTx);
    delay_ms(2000);
}
//Configure UART
void UartInit(void)
{
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                              // 1MHz 115200
    UCA0BR1 = 0;                              // 1MHz 115200
    UCA0MCTL = UCBRF_0 + UCBRS_1;               // Modulation UCBRSx = 5 BR=9600
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine
}

//Function to send data
void SendData(unsigned char *sPtr)
{
    for(;*sPtr!='\0';writeByte(*(sPtr++))); //increase pointer each iter
}

void writeByte(unsigned char byte)
{
    while(!(IFG2 & UCA0TXIFG)); // wait for TX buffer to be empty
    UCA0TXBUF = byte; //TX the char element
}

//Function do delay
void delay_ms(unsigned int time)
{
    while(time)
    {
        __delay_cycles(1000);  //By default clock is 1 MHZ. Hence 1 tick is 1us. 1ms means 1000 ticks.
        --time;
    }
}
//Function to read values
void sensor(void)
{
    flag = 1;
    i = 0;
    while(flag) //Perform until data recieved
    {
        init();
        __delay_cycles(1500000); //1.5s delay
    }
    sensor_read = 1; //flag as read
    get_values();
}

void init() //Function to initialize ESP
{
    __disable_interrupt();
    __delay_cycles(20); //First set P2 as input and activate
     P2DIR |= BIT4;
     P2OUT &= ~BIT4;
     __delay_cycles(20000);//Send '0' for 20ms
     P2OUT |= BIT4;
     __delay_cycles(30);//Send '1' for 20ms
     P2DIR &= ~BIT4; //Set BIT4 as input to receive
     P2SEL |= BIT4;
     TA1CTL = TASSEL_2|MC_2 ; //Mode UP, Capture Compare
     TA1CCTL2 = CAP | CCIE | CCIS_0 | CM_2 | SCS ; //Capture settings
     __enable_interrupt();
}
//Function to separate values read from Sensor
void get_values(void)
{
    if (i>=40)
    {
        for (j = 1; j <= 8; j++) //This part contains Humidity integer
        {
            if (diff[j] >= 110)
                hh |= (0x01 << (8-j));
        }
        for (j = 9; j <= 16; j++) //This part contains Humidity fraction
        {
            if (diff[j] >= 110)
                hl |= (0x01 << (16-j));
        }
        for (j = 17; j <= 24; j++)  //This part contains Temperature integer
        {
            if (diff[j] >= 110)
                th |= (0x01 << (24-j));
        }
        for (j = 25; j <= 32; j++)  //This part contains Temperature fraction
        {
            if (diff[j] >= 110)
                tl |= (0x01 << (32-j));
        }
        __delay_cycles(1100000); //1s delay to receive data from sensor
    }
}
//Transform values into ASCII and write it to DATA array to be sent.
void pull_values(void)
{
    volatile unsigned int rem;
    rem = th / 10; //First digit of Temperature integer
    rem = rem +48;
    data[0] = (unsigned char)rem;

    rem = th % 10;  //Second digit of Temperature integer
    rem = rem +48;
    data[1] = (unsigned char)rem;

    rem = tl / 10;  //First digit of Temperature fraction
    rem = rem +48;
    data[2] = (unsigned char)rem;

    rem = tl % 10;  //Second digit of Temperature fraction
    rem = rem +48;
    data[3] = (unsigned char)rem;

    if(hh>99)
    {
        hh = 99;
    }
    rem = hh / 10;  //First digit of Humidity integer
    rem = rem +48;
    data[4] = (unsigned char)rem;

    rem = hh % 10;  //Second digit of Humidity integer
    rem = rem +48;
    data[5] = (unsigned char)rem;

    if(hl>99)
    {
        hh = 99;
    }

    rem = hl / 10;  //First digit of Humidity fraction
    rem = rem +48;
    data[6] = (unsigned char)rem;

    rem = hl % 10;  //First digit of Humidity fraction
    rem = rem +48;
    data[7] = (unsigned char)rem;
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void Timer_A1(void)
{
    temp[i] = TA1CCR2; //Write Capture value to temporary array
    i += 1;
    TA1CCTL2 &= ~CCIFG ; //Reset CCP flag
    if (i>=2) diff[i-1]=temp[i-1]-temp[i-2]; //Delete info elements
    flag = 0;
}


