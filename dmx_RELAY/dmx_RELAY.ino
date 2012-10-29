/*
- customization 20/09/2012
- ATMEGA32u4
- Board DMX Receiver V2 - TinkerKit
- 
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

#define F_OSC 16000000       // Clock frequency (before 8MHz)
// #define USART_BAUD(F_OSC) ((F_OSC)/((250000)*16)-1) // 250kBaud
#define BAUD_RATE   250000

#define PWMpin1 6    // PWM OUT1	
#define PWMpin4 10   // PWM OUT4			
#define PWMpin3 9    // PWM OUT3			
#define PWMpin2 5    // PWM OUT2			


#define SW1   8   // DIP1
#define SW2  12   // DIP2
#define SW3   4   // DIP3
#define SW4  11   // DIP4
#define SW5  A0   // DIP5
#define SW6  A1   // DIP6
#define SW7  A2   // DIP7
#define SW8  A3   // DIP8
#define SW9  A4   // DIP9
#define SW10 A5   // DIP10

#define DE 2
#define LED 7

enum {BREAK, STARTB, STARTADD, DATA};

volatile unsigned int dmxStatus;
volatile unsigned int dmxStartAddress;
volatile unsigned int dmxCount;
volatile unsigned int ch1,ch2,ch3,ch4;
volatile unsigned int MASTER;

int tmp0,tmp1,tmp2,tmp3,tmp4,tmp5; // variabili per uscite analogiche



void init_USART()
{
UBRR1L = (uint8_t)(F_CPU/(BAUD_RATE*16L)-1);
UBRR1H = (F_CPU/(BAUD_RATE*16L)-1) >> 8;
UDR1 = 0;
UCSR1A  = 0;					// clear error flags, disable U2X and MPCM
UCSR1B = (1<<RXCIE1)| (1<<RXEN1) ;		// Enable receiver and transmitter | (1<<TXEN0) and Interrupt
UCSR1C = (1<<USBS1) | (3<<UCSZ10); // 8bit 2 stop
}



void setup(){

 
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
  
  pinMode(DE,OUTPUT);  // enable Tx Rx
  pinMode(LED,OUTPUT); // led DMX
  
  // attivo pull-up SW
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
 
 // Serial1.begin(250000);
    init_USART();
     //digitalWrite( DE,LOW);

}


void loop(){
  

  volatile unsigned int address1,address2,address3,address4,address5,address6,address7,address8,address9;
  // calcolo indirizzi sw
   address1 = digitalRead(SW1);
   address2 = digitalRead(SW2);
   address3 = digitalRead(SW3);
   address4 = digitalRead(SW4);
   
   
   if(address1 == 1)address1 = 0;
      else
      {
         address1 = 1;
      }
   if(address2 == 1)address2 = 0;
     else
      {
        address2 = 1;
      }
   if(address3 == 1)address3 = 0;
      else
      {
        address3 = 1;
      }
   if(address4 == 1)address4 = 0;
      else
      {
      address4 = 1;
      }

   
   if( analogRead(SW5) <= 20) address5 = 1; else address5 = 0;
   if( analogRead(SW6) <= 20) address6 = 1; else address6 = 0;
   if( analogRead(SW7) <= 20) address7 = 1; else address7 = 0;
   if( analogRead(SW8) <= 20) address8 = 1; else address8 = 0;
   if( analogRead(SW9) <= 20) address9 = 1; else address9 = 0;
   
   // somma SW
   dmxStartAddress = (address1*1) + (address2*2) + (address3*4) + (address4*8) + (address5*16) + (address6*32) + (address7*64) + (address8*128) + (address9*256);
  
    if (dmxStartAddress == 0)
		demo(); 
    if (dmxStartAddress >= 509)
		dmxStartAddress = 509;
  
 // digitalWrite( LED,HIGH);
  digitalWrite(DE,LOW);
  	sei();     						//enable global interrupt
    for (;;)
    {
    	//high(LED_PORT, LED);			// infinite loop???
    }
  
}


void demo()
{
  for (;;)										// infinite loop
	{
	analogWrite(PWMpin1,255);
	analogWrite(PWMpin2,255);
	analogWrite(PWMpin3,255);
	analogWrite(PWMpin4,255);
        }
 }


SIGNAL(USART1_RX_vect){
 // digitalWrite(LED,HIGH);	
int temp    = UCSR1A;
int dmxByte = UDR1;
			// ok verified it calls this routine
if (temp&(1<<DOR1))				// Data Overrun?
	{
	//high(LED_PORT,LED);		// it never pass by here
       //digitalWrite(LED,HIGH);
	dmxStatus = BREAK;			// wait for reset (BREAK)
	UCSR1A &= ~(1<<DOR1);
	goto tail;
	}
	
if (temp&(1<<FE1))				//BREAK or FramingError?
	{
	//high(LED_PORT,LED);		// it pass by here
	dmxCount = 0;				// reset byte counter
	dmxStatus = STARTB;			// let's think it's a BREAK ;-) ->wait for start byte
	UCSR1A &= ~(1<<FE1);
	goto tail;
	}
  
  
  switch(dmxStatus)
	{
	case STARTB:
		//high(LED_PORT,LED);			// it pass by here
		if (dmxByte == 0) 
			{
			dmxStatus = STARTADD;	// the FE WAS a BREAK -> wait for the right channel
			//START byte readed in!!!
			}
		else
			{
			dmxStatus = BREAK;			// wait for reset (BREAK)	// ...it was a framing error :-(
			//low(LED_PORT,LED);
			}
		goto tail;
	case STARTADD:
		// digitalWrite(LED,LOW);
                  
		if (dmxCount == dmxStartAddress+1)
			{
			dmxStatus = DATA;
			//dmxCount=0;
                        digitalWrite(LED,HIGH);
			}
		// else
			{
			dmxCount++;
			}
	//	if (dmxStatus != DATA)
			//goto tail;
	case DATA:									// HERE YOU SHOULD PROCESS THE CHOSEN DMX CHANNELS!!!
		//low(LED_PORT,LED);
                //dmxStatus = DATA;
		dmxStatus = STARTADD;					// next interrupt go back to update dmxCount
		if (dmxCount == dmxStartAddress){
				ch1=dmxByte;
                              //   dmxCount++;
                                 
                     }
		if (dmxCount == (dmxStartAddress+1)){
				ch2=dmxByte;
   
                               //dmxCount++;
                    }
		if (dmxCount == (dmxStartAddress+2)){
				ch3=dmxByte;
                                //dmxCount++;  
                  }
		if (dmxCount == (dmxStartAddress+3))
		{
			ch4=dmxByte;
			dmxStatus = BREAK;					// ALL CHANNELS RECEIVED
			//digitalWrite(LED,LOW);
	/*		if (MASTER == 1)					// compare and set values to master value
				{
				if (ch1 > ch4)
					ch1 = ch4;
				if (ch2 > ch4)
					ch2 = ch4;
				if (ch3 > ch4)
					ch3 = ch4;
				}  
        */
			if(ch1 > 0)
                        {
                           analogWrite(PWMpin4,255);			// update outputs
			}else{ analogWrite(PWMpin4,0);  }

                        if(ch2 > 0)
                        {
                           analogWrite(PWMpin3,255);
                        }else{ analogWrite(PWMpin3,0);  }
                        if(ch3 > 0)
                        {
                           analogWrite(PWMpin2,255);
			}else{ analogWrite(PWMpin2,0);  }
                        if(ch4 > 0)
                         {
                          analogWrite(PWMpin1,255);
                         }else{ analogWrite(PWMpin1,0);  }
                          
		}	
	}
  
  
  tail:
  asm("nop");//SREG = sregBuf; 	

}


