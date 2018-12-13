#ifndef DEFINITIONS_h
#define DEFINITIONS_h

#include "Arduino.h"

//see readme.md -> put your wifi/ota passwords in secrects.h!
//#include "secrets.h"

//hardware setup
  //see readme.md -> you can add your own boards*.h file to keep your hardware-setup save when upgrading the code

  #include "boards_smartinick_ttgo.h"
  //#include "boards_smartinick_wemos.h"
  //#include "boards_smartinick_pcb180723spidual.h"
  //#include "boards_smartinick_pcb180723i2csingle.h"


//define screen language
  #define LANGUAGE_EN
  //#define LANGUAGE_FR //translation kindly provided by Technoo' Loggie (Telegram Handle @TechnooLoggie)
  //#define LANGUAGE_DE

//turn on/off minor features
  //#define staticip //use static IP in Client Mode, Comment out for DHCP, IP is defined in boards*.h file
  //#define usestatusled //Status LED? GPIO defined in boards*.h

//define which functional modules we use, uncomment to disable:
  //#define headunit //replace BLE Module
  //#define hudisplaydebug //activate debug dislplay for sensor readings,...
  //#define usedisplay

  //#define usetelnetserver //enable telnet dashboard server, port defined below
  //#define usepacketserver //enable telnet packet server, port defined below
  //#define userrawserver //enable telnet raw data server, port defined below

//DEBUG Settings
  //#define debug_dump_states //dump state machines state
  //#define debug_dump_rawpackets //dump raw packets to Serial/Telnet
  //#define debug_dump_packetdecode //dump infos from packet decoder
  
//usually nothing has to be changed below

//#define usengcode
#ifdef usengcode
  #include "ngcode.h"
#endif

#define swversion "v2.0.0"

//Serial UART Setup for Debugging and M365 Connection
  #define DebugSerial Serial //Debuguart = default Serial Port/UART0
  
//helpers
  #define _max(a,b) ((a)>(b)?(a):(b))
  #define _min(a,b) ((a)<(b)?(a):(b))

//timing
  #define m365packettimeout  500 //timeout in ms after last received packet for showing connection-error
  #define housekeepertimeout 3000 //duration between housekeeper-datarequests
  #define splashscreentimeout 3000 //how long to show splashscreen on power-up
  #define wlanconnecttimeout 10000 //timeout for connecting to one of  the known ssids
  #define wlanapconnecttimeout 600000 //timeout in ap mode until client needs to connect / ap mode turns off

//telnet ports
  #ifdef usetelnetserver
    #define telnet_port_statistics 36523
  #endif
  #ifdef usepacketserver
    #define telnet_port_packet 36525
  #endif
  #ifdef userrawserver  
    #define telnet_port_raw 36524
  #endif

//just some checks of the above defines, don't change anything below!
//if something is wrong with the above defined things the next lines will throw an error
//solution is to fix the error in the defines above!

  #ifndef ESP32
    #error "Only ESP32 Plattform supported!"
  #endif
  //OLED
  #if (defined usedisplay && !defined useoled1)
    #error "usedisplay defined but useoled1 not defined!"
  #endif

  #if (!defined useoled1 && defined useoled2)
      #error "useoled2 defined, but useoled1 not defined"
  #endif

  #if (!defined ESP32 && !defined usei2c)
      #error "Hardware SPI only tested on ESP32!!!"
  #endif

  #ifndef usetelnetserver
    #ifdef usepacketserver
      #error "packetserver requires telnetserver"
    #endif
    #ifdef userawserver
      #error "rawserver requires telnetserver"
    #endif
  #endif

  #ifndef headunit
    #ifdef hudisplaydebug
      #error "hudisplaydebug requires headunit"
    #endif
  #endif


#endif