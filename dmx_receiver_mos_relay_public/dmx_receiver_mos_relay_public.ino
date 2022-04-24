/*
- customization 20/09/2012
 - ATMEGA32u4
 - Board DMX Receiver V2 - TinkerKit

 - customization 03/05/2019
 - SmartMode added 

 - customization 31/10/2021
 - Command line interface added & some bugs fixed
 
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

#include <Arduino.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <EEPROM.h>
#include "Cmd.h"
#include <utils.h>
#include <avr/wdt.h>

#define F_OSC 16000000 // Clock frequency
#define BAUD_RATE 250000

#define MOSFET 1
#define RELAY 2
/*
Here you have to select the output mode accordingly to the receiver type you are using.
 Choose MOSFET or RELAY for OUTPUT_MODE
 */
#define OUTPUT_MODE RELAY
//#define OUTPUT_MODE MOSFET
#define THR 20 //threshold for analogRead

#if OUTPUT_MODE == RELAY

#define PWMpin4 6 // PWM OUT4
#define PWMpin1 10 // PWM OUT1
#define PWMpin2 9 // PWM OUT2
#define PWMpin3 5 // PWM OUT3
#else

#define PWMpin1 6 // PWM OUT1
#define PWMpin4 10 // PWM OUT4
#define PWMpin3 9 // PWM OUT3
#define PWMpin2 5 // PWM OUT2
#endif

short outPins[4]={PWMpin1,PWMpin2,PWMpin3,PWMpin4};

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
  BREAK, STARTB, STARTADD, DATA, SMART};

volatile unsigned int dmxStatus;
volatile unsigned int dmxStartAddress;
volatile unsigned int dmxCount = 0 ;
volatile unsigned int ch1,ch2,ch3,ch4;
volatile unsigned int MASTER;
volatile unsigned int smartMode;
volatile unsigned int offDelay;


volatile bool learningMode;
byte dmxArray[512+1];


#define CH_1 1
#define CH_2 2
#define CH_3 4
#define CH_4 8
#define CHANNELS 4
volatile unsigned int chanDelay[CHANNELS];


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

int cmdFunctionHelp(int arg_cnt, char **args)
{

    Serial.println(F("\nUse these commands: 'help' - this text\n"
                          "'relay #' - set relay # setup for (1-4)\n"
                          "'set <DMX channel #> [<delay in 10s>]' - associate relay with channel\n"
                          "'del <DMX channel #>' - de-associate relay with channel \n"
                          "'print' - print association table\n"
                          "'save' - save current config in NVRAM\n"
                          "'load' - load config from NVRAM\n"
//                          "'log [serial_loglevel]' - define log level (0..7)\n"
                          "'kill' - test watchdog\n"
                          "'clear' - clear association table in RAM\n"
//                          "'reboot' - reboot controller"
));
return 1;                            
}

uint8_t currentRelay = 0;

int cmdFunctionRelay(int arg_cnt, char **args)
{
if (arg_cnt == 2) currentRelay=atoi(args[1]);

debugSerialPort.print(F("Current relay: "));
debugSerialPort.println(currentRelay);
return 1;                            
}

int cmdFunctionAdd(int arg_cnt, char **args)
{
int localDelay = 15;

if (!currentRelay)
    {
      debugSerialPort.println(F("No Current relay defined, use 'relay' command"));
      return 0;
    }  

if (arg_cnt<2)
    {
      debugSerialPort.println(F("usage: 'add #DMXchannel [delay_in_10sec]"));
      return 0;
    }  

int currentChan = atoi(args[1]);
if (currentChan<1 || currentChan>512)
    {
      debugSerialPort.println(F("DMX Channel must be in 1..512 range"));
      return 0;
    } 

if (arg_cnt>2)
    {
     localDelay = atoi(args[2]); 
     if (localDelay>15)
          {
            debugSerialPort.println(F("Delay must be in 0..14 10s range"));
            return 0;
          } 
    }    

                    dmxArray[currentChan] &= 15; //Reset previosly saved delay for the channel
                    dmxArray[currentChan]  |= ((1<<(currentRelay-1)) | (localDelay<<4)); 
return 1;                            
}

int cmdFunctionDel(int arg_cnt, char **args)
{

if (!currentRelay)
    {
      debugSerialPort.println(F("No Current relay defined, use 'relay' command"));
      return 0;
    }  

if (arg_cnt<2)
    {
      debugSerialPort.println(F("usage: 'del #DMXchannel"));
      return 0;
    }  

int currentChan = atoi(args[1]);
if (currentChan<1 || currentChan>512)
    {
      debugSerialPort.println(F("DMX Channel must be in 1..512 range"));
      return 0;
    } 

dmxArray[currentChan] &= ~(1<<(currentRelay-1));

return 1;                            
}

int cmdFunctionDelay(int arg_cnt, char **args)
{

return 1;                            
}
int cmdFunctionPrint(int arg_cnt, char **args)
{
for (short r=0; r<CHANNELS; r++)
    {
    debugSerialPort.print("Relay:");
    debugSerialPort.println(r+1);  
    for (short c=1; c<=512; c++)
        if (dmxArray[c] & (1<<r)) 
                {
                debugSerialPort.print(c);
                debugSerialPort.print(":");
                debugSerialPort.print(dmxArray[c]>>4);
                debugSerialPort.println(";");
                }
    }
return 1;                            
}

int cmdFunctionLog(int arg_cnt, char **args)
{

return 1;                            
}

int cmdFunctionClear(int arg_cnt, char **args)
{
memset(dmxArray,0,512);
return 1;                            
}

int cmdFunctionReboot(int arg_cnt, char **args)
{
softRebootFunc(); //Hmm just hung
return 1;                            
}

int cmdFunctionKill(int arg_cnt, char **args)
{  //WDT not working on Leonardo
    for (byte i = 1; i < 20; i++) {
        delay(1000);
        debugSerialPort.println(i);
    };
return 1;                            
}

int cmdFunctionLoad(int arg_cnt, char **args)
{
      Serial.println("Loading EEPROM data"); 
      EEPROM.get(0, dmxArray);
return 1;                            
}

int cmdFunctionSave(int arg_cnt, char **args)
{
      Serial.println("Saving EEPROM data"); 
      EEPROM.put(0, dmxArray);
return 1;                            
}


void setup()
{	

 #ifdef WDT_ENABLE 
  wdt_reset(); // reset watchdog counter
  wdt_disable();
  wdt_enable(WDTO_8S);
 #endif 
  /*Declaration of variables*/
  volatile unsigned int address1,address2,address3,address4,address5,address6,address7,address8,address9;
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


  /*Initialization of addresses 5-6-7-8-9 based on a threshold. The threshold avoid noise*/
  if( analogRead(SW5) <= THR)
    address5 = 1;
  else
    address5 = 0;

  if( analogRead(SW6) <= THR)
    address6 = 1;
  else
    address6 = 0;

  if( analogRead(SW7) <= THR)
    address7 = 1;
  else
    address7 = 0;

  if( analogRead(SW8) <= THR)
    address8 = 1;
  else
    address8 = 0;

  if( analogRead(SW9) <= THR)
    address9 = 1;
  else
    address9 = 0;

  if( analogRead(SW10) <= THR)
    smartMode = 1;
  else
    smartMode = 0;

  /*Calculation of dmxStartAddress*/
  dmxStartAddress = (address1*1) + (address2*2) + (address3*4) + (address4*8) + (address5*16) + (address6*32) + (address7*64) + (address8*128) + (address9*256);

  if (dmxStartAddress >= 509)  //The receiver manages 4 channels, so you can't set a start address above 509;
    dmxStartAddress = 509;
    
 
  sei(); //enable global interrupt
  
  Serial.begin(9600);  
  delay(1000);
  cmdInit();
  cmdAdd("relay", cmdFunctionRelay);
  cmdAdd("help", cmdFunctionHelp);
  cmdAdd("save", cmdFunctionSave);
  cmdAdd("load", cmdFunctionLoad);
  cmdAdd("print", cmdFunctionPrint);
  cmdAdd("set", cmdFunctionAdd);
  cmdAdd("del", cmdFunctionDel);
  //cmdAdd("delay", cmdFunctionDelay);
  //cmdAdd("log", cmdFunctionLog);
  cmdAdd("clear", cmdFunctionClear);
  //cmdAdd("reboot", cmdFunctionReboot);
  cmdAdd("kill", cmdFunctionKill);

 


  learningMode = 0;
  
  if (smartMode)
  {
   if (analogRead(SW9) <= THR) 
        {
        Serial.println("Entering learning mode & clear settings");
        learningMode = 1;  
        memset(dmxArray,0,sizeof(dmxArray));
        }  
    else
    {   
      Serial.println("Loading EEPROM data"); 
      EEPROM.get(0, dmxArray);
    }
               
  for (byte i=0;i<CHANNELS;i++) 
        {
          analogWrite(outPins[i],0);
          chanDelay[i]=0;
        } 
  offDelay = ((address1*1) + (address2*2) + (address3*4) + (address4*8) + (address5*16) + (address6*32) + (address7*64) + (address8*128));
  
  }
  else //Legacy mode
  {
   Serial.print("Start Address: "); Serial.println(dmxStartAddress);  
  }
  
  digitalWrite(DE,LOW);
 
   if (dmxStartAddress || smartMode) init_USART(); //Call to initialization of USART
}

uint32_t printTimer;
uint32_t processTimer;

void loop()
{
  #ifdef WDT_ENABLE  
   wdt_reset();
  #endif
  cmdPoll();
  if (smartMode)
  {

  if (isTimeOver(printTimer,millis(),10000))
              {
              Serial.print("[");  
              for (byte i=0; i<CHANNELS; i++)
                {
                  Serial.print(chanDelay[i]);Serial.print(" ");     
                  }
              Serial.println("]");
              printTimer=millis();
              }

  if (isTimeOver(processTimer,millis(),100))
              {
              for (byte i=0; i<CHANNELS; i++)
                {
                  if (chanDelay[i]) chanDelay[i]--;
                    else  analogWrite(outPins[i],0);      
                  }

              processTimer=millis();
              digitalWrite(LED,LOW);
              }

    if (!learningMode && (analogRead(SW9) <= THR)) 
        {
        Serial.println("Entering learning mode");
        learningMode = 1;  
       // memset(dmxArray,0,512);
        }  
    

  if( learningMode && (analogRead(SW9) > THR))
    {
      Serial.println("Leaving learning mode, saving data to EEPROM");
      EEPROM.put(0,dmxArray);
      learningMode = 0;
    }
    
  } //SmartMode handler 
  else if (dmxStartAddress == 0){ //If all dipswitches are 0 
    demo();                //call the demo function
  }
  
//delay (1000);  
//digitalWrite(LED,LOW);



}

/*Demo function if dmxStartAddress = 0*/
void demo()
{
  int bright;
  if (OUTPUT_MODE == MOSFET) 
  {
    for (;;)
    {

      for (bright = 0; bright < 255; bright++)	// infinite loop
      {
        analogWrite(PWMpin1,bright);
        analogWrite(PWMpin2,bright);
        analogWrite(PWMpin3,bright);
        analogWrite(PWMpin4,bright);
        delay(10);
      }

      for (bright = 255; bright >= 0; bright--)	// infinite loop
      {
        analogWrite(PWMpin1,bright);
        analogWrite(PWMpin2,bright);
        analogWrite(PWMpin3,bright);
        analogWrite(PWMpin4,bright);
        delay(10);
      }

      analogWrite(PWMpin1,255);
      analogWrite(PWMpin3,255);
      delay(10);
      analogWrite(PWMpin1,0);
      analogWrite(PWMpin3,0);
      analogWrite(PWMpin2,255);
      analogWrite(PWMpin4,255);
      delay(10);
    }
  } 
  else if (OUTPUT_MODE == RELAY)
  {
    for(;;)
    {
        Serial.println("Relay Demo");

      analogWrite(PWMpin1,255);
      analogWrite(PWMpin2,0);
      analogWrite(PWMpin3,0);
      analogWrite(PWMpin4,0);
      delay(1000);
      analogWrite(PWMpin1,0);
      analogWrite(PWMpin2,255);
      analogWrite(PWMpin3,0);
      analogWrite(PWMpin4,0);

      delay(1000);
      analogWrite(PWMpin1,0);
      analogWrite(PWMpin2,0);
      analogWrite(PWMpin3,255);
      analogWrite(PWMpin4,0);

      delay(1000);
      analogWrite(PWMpin1,0);
      analogWrite(PWMpin2,0);
      analogWrite(PWMpin3,0);
      analogWrite(PWMpin4,255);
      delay(1000);
    }
  }
}


SIGNAL(USART1_RX_vect)
{
  int temp = UCSR1A;
  volatile int dmxByte = UDR1;


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

    if (dmxByte == 0)  //This is our start byte
    {
      if (smartMode) dmxStatus = SMART;      
      else if (dmxStartAddress==1) dmxStatus = DATA; // the FE WAS a BREAK -> the next byte is our first channel
      else dmxStatus = STARTADD;	// the FE WAS a BREAK -> wait for the right channel
      dmxCount=1;
    }
    else
    {
      dmxStatus = BREAK;	// wait for reset (BREAK) it was a framing error
    }
    goto tail;
    break;

  case SMART:
    if (dmxCount>512) {dmxStatus = BREAK;break;}
    
    digitalWrite(LED,HIGH);
    if (learningMode)
    {
      if (dmxByte) // if light is on on appropriate channel - set bit
                  { 
                    byte localDelay = 0;
                    
                    if( analogRead(SW5) <= THR) localDelay |= 16;
                    if( analogRead(SW6) <= THR) localDelay |= 32;
                    if( analogRead(SW7) <= THR) localDelay |= 64;
                    if( analogRead(SW8) <= THR) localDelay |= 128;
                    dmxArray[dmxCount] &= 15; //Reset previosly saved delay for the channel
                    if (!digitalRead(SW1)) dmxArray[dmxCount]  |= (CH_1 | localDelay); 
                    if (!digitalRead(SW2)) dmxArray[dmxCount]  |= (CH_2 | localDelay); 
                    if (!digitalRead(SW3)) dmxArray[dmxCount]  |= (CH_3 | localDelay); 
                    if (!digitalRead(SW4)) dmxArray[dmxCount]  |= (CH_4 | localDelay); 
                    
                  }
    }
    else //operating Mode
    {
     // if (dmxByte)  
        for (byte i=0;i<CHANNELS;i++)
          if (dmxArray[dmxCount] & (1<<i)) 
          { 
            unsigned int localDelay = dmxArray[dmxCount] >> 4;
            if (localDelay == 15) localDelay=offDelay;  //Default Delay
            
            if (dmxByte)  
            { //ON               
              analogWrite(outPins[i],255);
              if (!localDelay) localDelay =1;
                 else localDelay=localDelay * 100; //tens of second

              if (chanDelay[i]<localDelay) chanDelay[i] = localDelay;  

            }
            else //OFF
            {
              if (!localDelay) analogWrite(outPins[i],0); 
            }

          }
     }
    dmxCount++;  
    break;  
  case STARTADD:
    if (dmxCount == dmxStartAddress-1) //Is the next byte channel one?
    {
      dmxStatus = DATA; //Yes, so let's wait the data
    }
    dmxCount++;

    break;


  case DATA:	// HERE YOU SHOULD PROCESS THE CHOSEN DMX CHANNELS!!!
    digitalWrite(LED,HIGH); 
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
