#ifndef TELNET_h
#define TELNET_h

#include "definitions.h"

#ifdef usetelnetserver
  extern WiFiServer telnetserver;
  extern WiFiClient telnetclient;
  #ifdef usepacketserver
    extern WiFiServer packetserver;
    extern WiFiClient packetclient;
  #endif
  #ifdef userawserver
    extern WiFiServer rawserver;
    extern WiFiClient rawclient;
    extern uint8_t telnetrawstate;
    extern uint8_t telnetrawstateold;
  #endif
  extern uint8_t telnetstate;
  extern uint8_t telnetstateold;
  
  #define telnetoff 0
  #define telnetturnon 1
  #define telnetlistening 2
  #define telnetclientconnected 3
  #define telnetturnoff 4
  #define telnetdisconnectclients 5

  #define userconnecttimeout 300000 //timeout for connecting to telnet/http after wlan connection has been established with ap or client
  #define telnetrefreshanyscreen 500 //refresh telnet screen every xx ms
  #define telnetrefreshrawarrayscreen 1000 //refresh telnet screen every xx ms
  extern unsigned long userconnecttimestamp;
  extern unsigned long telnetnextrefreshtimestamp;
  
  //telnet screens
  #define ts_telemetrie 0
  #define ts_statistics 1
  #define ts_esc_raw 2
  #define ts_ble_raw 3
  #define ts_bms_raw 4
  #define ts_x1_raw 5
  extern uint8_t telnetscreen;
  extern uint8_t telnetscreenold;

  //ANSI
  extern String ansiPRE;
  extern String ansiHOME;
  extern String ansiESC;
  extern String ansiCLC;
  extern String ansiEND;
  extern String ansiBOLD;
  extern String ansiRED;
  extern String ansiGRN;
  extern String ansiBLU;
  extern String ansiREDF;
  extern String ansiGRNF;
  extern String ansiBLUF;
  extern String BELL;

void telnet_refreshscreen(void);
void handle_telnet(void);
#endif

#endif
