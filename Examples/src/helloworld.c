// Hello World! for Padauk ICs, to be compiled with SDCC. Serial output on PA7 at 115200 baud.

#include <stdint.h>
#include <stdio.h>
#include "easypdk/pdk.h" 

volatile uint16_t txdata;                       //txdata, serial data shift register

void interrupt(void) __interrupt(0)
{
  if( _intrq & INTRQ_TM2 )                      //TM2 interrupt request?
  {
    _intrq &= ~INTRQ_TM2;                       //mark TM2 interrupt request processed
    if( txdata )                                //txdata contains bits to send?
    {
      if( txdata&1 )                            //check bit (1/0) for sending
        __set1( _pa, 7 );                       //send 1 on PA7
      else
        __set0( _pa, 7 );                       //send 0 on PA7
      txdata >>= 1;                             //shift txdata 
    }
  }
}

int putchar(int c)
{
  while(txdata);                                //wait for completion of previous transmission
  _inten &= ~INTEN_TM2;                         //disable TM2 (setup of 16 bit value txdata is non atomic)
  txdata = (c << 1) | 0x200;                    //setup txdata with start and stop bit
  _inten |= INTEN_TM2;                          //enable TM2
  return (c);
}

unsigned char _sdcc_external_startup(void)
{
  EASY_PDK_INIT_SYSCLOCK_8MHZ();                //use 8MHz sysclock
  EASY_PDK_CALIBRATE_IHRC(8000000,4000);        //tune SYSCLK to 8MHz @ 4.000V
  return 0;                                     //perform normal initialization
}

void main(void)
{
  //setup timer2 (TM2) interrupt for 115200 baud
  _tm2c = TM2C_CLK_IHRC;                        //use IHRC -> 16 Mhz
  _tm2s = TM2S_PRESCALE_NONE | TM2S_SCALE_DIV2; //no prescale, scale 2 ~> 8MHz
  _tm2b = 68;                                   //divide by (68+1) ~> 115942 Hz (apx. 115200)

  _pac = 0x80;                                  //enable PA.7 as output
  txdata = 0xD55F;                              //setup 2 stop bits, 0x55 char for autobaud, 1 start bit, 5 stop bits
  _inten = INTEN_TM2;                           //enable TM2 interrupt, send out initial stop bits and autobaud char
  __engint();                                   //enable global interrupts
  for(;;)
  {
    puts("Hello World!");
    for(uint32_t i=1500000; i>0; i--);          //wait approx. 3s
  }
}