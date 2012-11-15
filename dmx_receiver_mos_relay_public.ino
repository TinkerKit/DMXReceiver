/*
- customization 20/09/2012
 - ATMEGA32u4
 - Board DMX Receiver V2 - TinkerKit
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define F_OSC 16000000 // Clock frequency
#define BAUD_RATE 250000

#define MOSFET 1
#define RELAY 2
/*
Here you have to select the output mode accordingly to the receiver type you are using.
Choose MOSFET or RELAY for OUTPUT_MODE
*/
#define OUTPUT_MODE MOSFET

#define TRH 20 //treshold for analogRead

#define PWMpin1 6 // PWM OUT1
#define PWMpin4 10 // PWM OUT4
#define PWMpin3 9 // PWM OUT3
#define PWMpin2 5 // PWM OUT2


#define SW1 8 // DIP1
#define SW2 12 // DIP2
#define SW3 4 // DIP3
#define SW4 11 // DIP4
#define SW5 A0 // DIP5
#define SW6 A1 // DIP6
#define SW7 A2 // DIP7
#define SW8 A3 // DIP8
#define SW9 A4 // DIP9
#define SW10 A5 // DIP10

#define DE 2
#define LED 7
enum {
  BREAK, STARTB, STARTADD, DATA};

volatile unsigned int dmxStatus;
volatile unsigned int dmxStartAddress;
volatile unsigned int dmxCount = 0 ;
volatile unsigned int ch1,ch2,ch3,ch4;
volatile unsigned int MASTER;

int tmp0,tmp1,tmp2,tmp3,tmp4,tmp5; //variables for analog outputs

/*Initialization of USART*/
void init_USART()
{
  UBRR1L = (uint8_t)(F_CPU/(BAUD_RATE*16L)-1); //Set Baud rate at 250 kbit/s
  UBRR1H = (F_CPU/(BAUD_RATE*16L)-1) >> 8;     // 
  UDR1 = 0;
  UCSR1A = 0;	                        // clear error flags, disable U2X and MPCM
  UCSR1B = (1<<RXCIE1)| (1<<RXEN1) ;	// Enable receiver
  UCSR1C = (1<<USBS1) | (3<<UCSZ10);	// 8bit 2 stop
}



void setup()
{	
  /*Initialization of INPUT*/
  pinMode(SW1,INPUT);
  pinMode(SW2,INPUT);
  pinMode(SW3,INPUT);
  pinMode(SW4,INPUT);
  pinMode(SW5,INPUT);
  pinMode(SW6,INPUT);
  pinMode(SW7,INPUT);
  pinMode(SW8,INPUT);
  pinMode(SW9,INPUT);
  pinMode(SW10,INPUT);

  /*Initialization of OUTPUT*/
  pinMode(DE,OUTPUT); // enable Tx Rx
  pinMode(LED,OUTPUT); // led DMX

  /*Setting the pull-up resistors of inputs*/
  digitalWrite( SW1,HIGH);
  digitalWrite( SW2,HIGH);
  digitalWrite( SW3,HIGH);
  digitalWrite( SW4,HIGH);
  digitalWrite( SW5,HIGH);
  digitalWrite( SW6,HIGH);
  digitalWrite( SW7,HIGH);
  digitalWrite( SW8,HIGH);
  digitalWrite( SW9,HIGH);
  digitalWrite( SW10,HIGH);

  init_USART(); //Call to initialization of USART
}


void loop()
{
  /*Declaration of variables*/
  volatile unsigned int address1,address2,address3,address4,address5,address6,address7,address8,address9;
  /*Calculating address*/
  address1 = digitalRead(SW1);
  address2 = digitalRead(SW2);
  address3 = digitalRead(SW3);
  address4 = digitalRead(SW4);

  if(address1 == 1)
    address1 = 0;
  else
    address1 = 1;
  if(address2 == 1)
    address2 = 0;
  else
    address2 = 1;
  if(address3 == 1)
    address3 = 0;
  else
    address3 = 1;

  if(address4 == 1)
    address4 = 0;
  else
    address4 = 1;


  /*Initialization of address bit 5-6-7-8-9 based on a threshold. The threshold avoid noise*/
  if( analogRead(SW5) <= TRH)
    address5 = 1;
  else
    address5 = 0;

  if( analogRead(SW6) <= TRH)
    address6 = 1;
  else
    address6 = 0;

  if( analogRead(SW7) <= TRH)
    address7 = 1;
  else
    address7 = 0;

  if( analogRead(SW8) <= TRH)
    address8 = 1;
  else
    address8 = 0;

  if( analogRead(SW9) <= TRH)
    address9 = 1;
  else
    address9 = 0;

  /*Calculation of dmxStartAddress*/
  dmxStartAddress = (address1*1) + (address2*2) + (address3*4) + (address4*8) + (address5*16) + (address6*32) + (address7*64) + (address8*128) + (address9*256);

  if (dmxStartAddress == 0){ //If all dipswitches are 0 
      demo();                //call the demo function
  }  

  else if (dmxStartAddress >= 509)  //The receiver manages 4 channels, so you can't set a start address above 509;
    dmxStartAddress = 509;

  digitalWrite(DE,LOW);

  sei(); //enable global interrupt

  for (;;)
  {
    // infinite loop, data will be updated throught interrupt
  }

}

/*Demo function if dmxStartAddress = 0*/
void demo()
{
  for (;;)	// infinite loop
  {
    analogWrite(PWMpin1,255);
    analogWrite(PWMpin2,255);
    analogWrite(PWMpin3,255);
    analogWrite(PWMpin4,255);
  }
}


SIGNAL(USART1_RX_vect)
{
  int temp = UCSR1A;
  int dmxByte = UDR1;

   
  if (temp&(1<<DOR1))	// Data Overrun?
  {
    dmxStatus = BREAK;	// wait for reset (BREAK)
    UCSR1A &= ~(1<<DOR1);
    goto tail;
  }

  if (temp&(1<<FE1))	//BREAK or FramingError?
  {

    dmxCount = 0;	// reset byte counter
    dmxStatus = STARTB;	// let's think it's a BREAK ;-) ->wait for start byte
    UCSR1A &= ~(1<<FE1);
    goto tail;
  }


  switch(dmxStatus)
  {
  case STARTB:

    if (dmxByte == 0)  //This is our star byte
    {

      if (dmxStartAddress==1) dmxStatus = DATA; // the FE WAS a BREAK -> the next byte is our first channel
      else dmxStatus = STARTADD;	// the FE WAS a BREAK -> wait for the right channel
      dmxCount=1;

      
    }
    else
    {
      dmxStatus = BREAK;	// wait for reset (BREAK) it was a framing error
    }
    goto tail;
    break;

  case STARTADD:

    if (dmxCount == dmxStartAddress-1) //Is the next byte channel one?
    {
      dmxStatus = DATA; //Yes, so let's wait the data


    }

    dmxCount++;

    break;


  case DATA:	// HERE YOU SHOULD PROCESS THE CHOSEN DMX CHANNELS!!!
    if (dmxCount == dmxStartAddress)
    {

      ch1=dmxByte;      
      dmxCount++;	

    }
    else if (dmxCount == (dmxStartAddress+1))
    {
      ch2=dmxByte;
      dmxCount++;



    }
    else if (dmxCount == (dmxStartAddress+2))
    {

      ch3=dmxByte;
      dmxCount++;


    }
    else  if (dmxCount == (dmxStartAddress+3))
    {
      ch4=dmxByte;
      dmxCount=1;
      dmxStatus = BREAK;	// ALL CHANNELS RECEIVED

      if (OUTPUT_MODE == MOSFET)        //Mosfet or Relay receiver?
      {
        analogWrite(PWMpin1,ch1);	// update mosfet outputs
        analogWrite(PWMpin2,ch2);
        analogWrite(PWMpin3,ch3);
        analogWrite(PWMpin4,ch4);
      }
      else if (OUTPUT_MODE == RELAY)
      {
        if(ch1 > 0)
        {
          analogWrite(PWMpin1,255);	// update relay outputs
        }
        else
        {
          analogWrite(PWMpin1,0);
        }

        if(ch2 > 0)
        {
          analogWrite(PWMpin2,255);
        }
        else
        {
          analogWrite(PWMpin2,0);
        }

        if(ch3 > 0)
        {
          analogWrite(PWMpin3,255);
        }
        else
        {
          analogWrite(PWMpin3,0);
        }

        if(ch4 > 0)
        {
          analogWrite(PWMpin4,255);
        }
        else
        {
          analogWrite(PWMpin4,0);
        }

      }

    }	

  }	

tail:
  asm("nop");
}





