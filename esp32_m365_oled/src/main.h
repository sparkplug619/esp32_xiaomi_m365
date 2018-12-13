#ifndef MAIN_H
#define MAIN_H

#include "definitions.h"

#ifdef usedisplay
  #include "display.h"
  extern M365Display *ptrm365d;
#endif

#ifdef headunit
  #include "m365headunit.h"
  extern M365Headunit *ptrm365bus;
#else
  #include "m365client.h"
  extern M365Client *ptrm365bus;
#endif


extern void hutask( void * pvParameters );
extern void displaytask( void * pvParameters );
extern void print_states(void);


  extern uint8_t i;
  extern char tmp1[200];
  extern char tmp2[200];
  //extern char chipid[7];

//timing & statistics
  extern unsigned long duration_mainloop;
  extern unsigned long timestamp_mainloopstart;
  extern unsigned long duration_oled;
  extern unsigned long timestamp_oledstart;
  extern unsigned long duration_oled1draw;
  extern unsigned long timestamp_oled1draw;
  extern unsigned long duration_oled1transfer;
  extern unsigned long timestamp_oled1transfer;
  extern unsigned long duration_oled2draw;
  extern unsigned long timestamp_oled2draw;
  extern unsigned long duration_oled2transfer;
  extern unsigned long timestamp_oled2transfer;
  extern unsigned long duration_telnet;
  extern unsigned long timestamp_telnetstart;
  extern unsigned long timestamp_showsplashscreen;

#ifdef usestatusled
  extern int ledontime;
  extern int ledofftime;
  extern unsigned long ledcurrenttime;
#endif


#endif