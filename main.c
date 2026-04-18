//  Jonathon Hughes
//  RFID Poker Table
//  6/25/2024
//  Master Controller 

#include "msp430fr2355.h"
#include <msp430.h> 
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

char char_In;
#define Slave_Address1 0x010; //Player one
#define Slave_Address3 0x030;  // Player Two


// String of cards in hand
char playerOneHand[2];
char playerTwoHand[2];


char commandPacket[] = {0x00, 0x00};

char setPacket[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

int greenFlag = 0;
int blueFlag  = 0;
int yellowFlag  = 0;


// Hex values sent from slave
int cardOne;
int cardTwo;

//Flags to tell who is being read from
int p1Flag = 0;
int p2Flag = 0;

int timer = 0;

// Hex Byte to be sent to retrive cards
char I2C_Message;

// Hex byte that is read in
char Data_in;

void UART_INIT();
void I2C_INIT();
void timerINIT();
void readPlayerTwo();
void readPlayerOne();

int sendP1 = 0;
int sendP2 = 0;

int main(void) {

    WDTCTL = WDTPW | WDTHOLD;
    I2C_INIT();
    timerInit();

    P4DIR &= ~BIT4;
    P4REN |= BIT4;
    P4OUT |= BIT4;
    P4IES |= BIT4;

    P2DIR &= ~BIT5;
    P2REN |= BIT5;
    P2OUT |= BIT5;
    P2IES |= BIT5;

    P3DIR &= ~BIT1;
    P3REN |= BIT1;
    P3OUT |= BIT1;
    P3IES |= BIT1;

    PM5CTL0 &= ~LOCKLPM5;
    __delay_cycles(1000);
    UCB0CTLW0 &= ~UCSWRST;
    __delay_cycles(1000);


    P2IFG &= ~BIT5;
    P2IE |= BIT5;

    P3IFG &= ~BIT1;
    P3IE |= BIT1;

    P4IFG &= ~BIT4;
    P4IE |= BIT4;

    TB0CCTL0 &= ~CCIFG;
    TB0CCTL0 |= CCIE;

    UCB0IE |= UCRXIE0;
    UCB0IE |= UCTXIE0;

    __enable_interrupt();

    while (1) {
        readPlayerOne();
        readPlayerTwo();
        __delay_cycles(10000);

    }
    return 0;
}
//-----------------------------------END MAIN-------------------------------

void readPlayerOne(){
    p1Flag = 1;
    UCB0I2CSA = Slave_Address1;
    UCB0TBCNT = 1;
    UCB0CTLW1 |= UCASTP_2;
    UCB0CTLW0 &= ~UCTR;
    UCB0CTLW0 |= UCTXSTT;
    while ((UCB0IFG & UCSTPIFG) == 0){}
    UCB0IFG &= ~UCSTPIFG;

}

void readPlayerTwo(){
    p2Flag = 1;
    UCB0I2CSA = Slave_Address3;
    UCB0TBCNT = 1;
    UCB0CTLW1 |= UCASTP_2;
    UCB0CTLW0 &= ~UCTR;
    UCB0CTLW0 |= UCTXSTT;
    while ((UCB0IFG & UCSTPIFG) == 0){}
    UCB0IFG &= ~UCSTPIFG;
}

//-----------------------------------START MAIN------------------------
// I2C INIT
//-------------------------------------------------------------------------------
void I2C_INIT(){
    UCB0CTLW0 |= UCSWRST;
    UCB0CTLW0 |= UCSSEL_3;
    UCB0BRW = 10;
    UCB0CTLW0 |= UCMODE_3;
    UCB0CTLW0 |= UCMST;
    UCB0CTLW1 |= UCASTP_2;
    P1SEL1 &= ~BIT2;
    P1SEL0 |= BIT2;
    P1SEL1 &= ~BIT3;
    P1SEL0 |= BIT3;
}

//-------------------------------------------------------------------------------
//Timer Init
//-------------------------------------------------------------------------------
void timerInit(){
    TB0CTL |= TBCLR;
    TB0CTL |= TBSSEL__ACLK;
    TB0CTL |= MC__UP;
    TB0CCR0 = 12542;
}
//-----------------------------END_Timer_INIT---------------------------------



//--------------------------------------------------------------
// I2C ISR
//--------------------------------------------------------------
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) {

    switch(UCB0IV){
    // RECEIVE PACKET FROM PLAYERS
    case 0x16:
        // PLAYER ONE
        if(p1Flag == 1){
            /*
            for(i = 0; i<2; i++){
                if(i == 0){
                    cardOne = UCB0RXBUF;
                    __delay_cycles(100);
                    playerOneHand[0] = CardIndex(cardOne, 1, 0);

                }else if(i == 1){
                    cardTwo = UCB0RXBUF;
                    __delay_cycles(100);
                    playerOneHand[1] = CardIndex(cardTwo, 1, 1);

                }
            }
            p1Flag = 0;
            */

        }
        // PLAYER TWO
        if(p2Flag == 1){
            /*
            for(i = 0; i<2; i++){
                if(i == 0){
                    cardOne = UCB0RXBUF;
                    __delay_cycles(100);
                    playerTwoHand[0] = CardIndex(cardOne, 2, 0);
                }else if(i == 1){
                    cardTwo = UCB0RXBUF;
                    __delay_cycles(100);
                    playerTwoHand[1] = CardIndex(cardTwo, 2, 1);

                }
            }
            p2Flag = 0;
            */

        }

        break;

    // TRANSMITTING
    case 0x18:

        break;
    }
}


// BUTTON PORTS???
// NEEDED????
// IDK ILL ASK TULLY

#pragma vector = PORT4_VECTOR
__interrupt void ISR_Port4_LSN(void){
    greenFlag = 1;
    P4IFG &= ~BIT4;
}

#pragma vector = PORT3_VECTOR
__interrupt void ISR_Port3_BIT1(void){
    yellowFlag = 1;
    P3IFG &= ~BIT1;
}

#pragma vector = PORT2_VECTOR
__interrupt void ISR_Port2_BIT5(void){
    blueFlag = 1;
    P2IFG &= ~BIT5;
}


//-------------------------------------------------------------------------------
// Timer Interrupts
//-------------------------------------------------------------------------------
#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_Overflow(void) {
    timer = 1;
    TB0CCTL0 &= ~CCIFG;

}
//-------------------------END_ADC_ISR----------------------------------




// Card Index Converter
// Takes input that is unique to the card
// Also input of playerHand
// Converts to Name
// Assigns player cards to hand

// TO DO:
// Change output to send code to computer via UART.
// Get rid of old LCD Code 
//


char CardIndex(int cardHex, int player, int card){

    if(cardHex == 0x00){
        // WTF HAPPENS ON 0X00???

    }else if(cardHex == 0x01){

    }else if(cardHex == 0x02){

        //Serial.println("3 of Spades\n");
    }else if(cardHex == 0x03){

        //Serial.println("4 of Spades\n");
    }else if(cardHex == 0x04){

        //Serial.println("5 of Spades\n");
    }else if(cardHex == 0x05){

        //Serial.println("6 of Spades\n");
    }else if(cardHex == 0x06){
        //Serial.println("7 of Spades\n");
    }else if(cardHex == 0x07){

        //Serial.println("8 of Spades\n");
    }else if(cardHex == 0x08){

        //Serial.println("9 of Spades\n");
    }else if(cardHex == 0x09){

        //Serial.println("10 of Spades\n");
    }else if(cardHex == 0x0A){

        //Serial.println("J of Spades\n");
    }else if(cardHex == 0x0B){

        // Serial.println("Q of Spades\n");
    }else if(cardHex == 0x0C){

        //Serial.println("K of Spades\n");

    //Clubs
    }else if(cardHex == 0x0D){

        //Serial.println("Ace of Clubs\n");
    }else if(cardHex == 0x0E){

        //Serial.println("2 of Clubs\n");
    }else if(cardHex == 0x0F){

        //Serial.println("3 of Clubs\n");
    }else if(cardHex == 0x10){

        // Serial.println("4 of Clubs\n");
    }else if(cardHex == 0x11){

        // Serial.println("5 of Clubs\n");
    }else if(cardHex == 0x12){

        //Serial.println("6 of Clubs\n");
    }else if(cardHex == 0x13){

        //Serial.println("7 of Clubs\n");
    }else if(cardHex == 0x14){

        //Serial.println("8 of Clubs\n");
    }else if(cardHex == 0x15){

        //Serial.println("9 of Clubs\n");
    }else if(cardHex == 0x16){

        //Serial.println("10 of Clubs\n");
    }else if(cardHex == 0x17){

        //Serial.println("J of Clubs\n");
    }else if(cardHex == 0x18){

        //Serial.println("Q of Clubs\n");
    }else if(cardHex == 0x19){

        //Serial.println("K of Clubs\n");

      //Diamonds
    }else if(cardHex == 0x1A){

        //Serial.println("Ace of Diamonds\n");

    }else if(cardHex == 0x1B){

        // Serial.println("2 of Diamonds\n");
    }else if(cardHex == 0x1C){

        // Serial.println("3 of Diamonds\n");
    }else if(cardHex == 0x1D){

        //Serial.println("4 of Diamonds\n");
    }else if(cardHex == 0x1E){

        //Serial.println("5 of Diamonds\n");
    }else if(cardHex == 0x1F){

        //Serial.println("6 of Diamonds\n");
    }else if(cardHex == 0x20){

        //Serial.println("7 of Diamonds\n");
    }else if(cardHex == 0x21){

        //Serial.println("8 of Diamonds\n");
    }else if(cardHex == 0x22){

        //Serial.println("9 of Diamonds\n");
    }else if(cardHex == 0x23){

        // Serial.println("10 of Diamonds\n");
    }else if(cardHex == 0x24){

        //Serial.println("J of Diamonds\n");
    }else if(cardHex == 0x25){

        //Serial.println("Q of Diamonds\n");
    }else if(cardHex == 0x26){

        //Serial.println("K of Diamonds\n");

      //Hearts
    }else if(cardHex == 0x27){

        // Serial.println("Ace of Hearts\n");
    }else if(cardHex == 0x28){

        // Serial.println("2 of Hearts\n");
    }else if(cardHex == 0x29){

        // Serial.println("3 of Hearts\n");
    }else if(cardHex == 0x2A){

        //Serial.println("4 of Hearts\n");
    }else if(cardHex == 0x2B){

        // Serial.println("5 of Hearts\n");
    }else if(cardHex == 0x2C){

        // Serial.println("6 of Hearts\n");
    }else if(cardHex == 0x2D){

        //Serial.println("7 of Hearts\n");
    }else if(cardHex == 0x2E){

        //Serial.println("8 of Hearts\n");
    }else if(cardHex == 0x2F){

        //Serial.println("9 of Hearts\n");
    }else if(cardHex == 0x30){

        //Serial.println("10 of Hearts\n");
    }else if(cardHex == 0x31){

        // Serial.println("J of Hearts\n");
    }else if(cardHex == 0x32){

        //Serial.println("Q of Hearts\n");
    }else if(cardHex == 0x33){

        //Serial.println("K of Hearts\n");
    }else{

    }
}

