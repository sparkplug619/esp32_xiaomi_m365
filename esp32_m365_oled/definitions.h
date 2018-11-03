#ifndef DEFINITIONS_h
#define DEFINITIONS_h

#include "Arduino.h"
#include "secrets.h"

#define swversion "18.11.02"

#define ng

#define M365debugtx GPIO_NUM_25
#define dev_ttgo
//#define dev_wemos
//#define dev_pcb180723


#ifdef dev_ttgo

  #define usei2c //comment out for SPI
  #define useoled1 //comment out to disable oled functionality
  #define OLED1_ROTATION 0 //0 = normal, 1= 90, 2=180, 3=270° CW
  #define oled_scl 15
  #define oled_sda 4
  #define oled_reset 16
  #define oled_doreset true
  #define oled1_address 0x3C
  #ifdef ng
    #define M365SerialGPIO GPIO_NUM_23 //Wemos board
  #else
    #define UART2RX GPIO_NUM_23 //PCB v180723
    #define UART2TX GPIO_NUM_22 //PCB v180723
    #define UART2RXunused GPIO_NUM_21 //PCB v180723; ESP32 does not support RX or TX only modes - so we remap the rx pin to a unused gpio during sending
  #endif

#elif defined(dev_wemos)
  #define usei2c //comment out for SPI
  #define useoled1 //comment out to disable oled functionality
  #define useoled2 //comment out if you use only one display
  #define OLED1_ROTATION 0 //0 = normal, 1= 90, 2=180, 3=270° CW
  #define OLED2_ROTATION 0 //0 = normal, 1= 90, 2=180, 3=270° CW
  #define oled_scl GPIO_NUM_4 //working wemos fake board
  #define oled_sda GPIO_NUM_16 //working wemos fake board
  #define oled_reset -1
  #define oled_doreset false
  #define oled1_address 0x3C
  #define oled2_address 0x3D
  #ifdef ng
    #define M365SerialGPIO GPIO_NUM_25 //Wemos board
  #else
    #define UART2RX GPIO_NUM_23 //PCB v180723
    #define UART2TX GPIO_NUM_22 //PCB v180723
    #define UART2RXunused GPIO_NUM_21 //PCB v180723; ESP32 does not support RX or TX only modes - so we remap the rx pin to a unused gpio during sending
  #endif
#elif defined(dev_pcb180723)
  //#define usei2c //comment out for SPI
  #define useoled1 //comment out to disable oled functionality
  #define useoled2 //comment out if you use only one display
  #define OLED1_ROTATION 2 //0 = normal, 1= 90, 2=180, 3=270° CW
  #define OLED2_ROTATION 0 //0 = normal, 1= 90, 2=180, 3=270° CW
  #define OLED_MISO   GPIO_NUM_19
  #define OLED_MOSI   GPIO_NUM_33
  #define OLED_CLK    GPIO_NUM_32
  #define OLED_DC     GPIO_NUM_25
  #define OLED1_CS    GPIO_NUM_26
  #define OLED1_RESET GPIO_NUM_27
  #define OLED2_CS    GPIO_NUM_14
  #define OLED2_RESET GPIO_NUM_12
  #define M365SerialGPIO GPIO_NUM_25 //Wemos board

#else

//display config
 
  #define usei2c //comment out for SPI
  #define useoled1 //comment out to disable oled functionality
  //#define useoled2 //comment out if you use only one display
  #define OLED1_ROTATION 0 //0 = normal, 1= 90, 2=180, 3=270° CW
  #define OLED2_ROTATION 0 //0 = normal, 1= 90, 2=180, 3=270° CW


//pin definitions for i2c display:
  #if (defined usei2c && defined useoled1) //one display, I2C Mode
      //#define oled_scl GPIO_NUM_4 //working wemos fake board
      //#define oled_sda GPIO_NUM_16 //working wemos fake board
      //#define oled_scl GPIO_NUM_32 //SCLK Pad on PCB
      //#define oled_sda GPIO_NUM_33 //MOSI Pad on PCB
    //#define oled_reset -1

    //ttgo dev board
    #define oled_scl 15
    #define oled_sda 4
    #define oled_reset 16

    #define oled1_address 0x3C
  #endif


  #if (defined usei2c && defined useoled2) //2nd display, i2c mode
      #define oled2_address 0x3D
  #endif


//pin definitions for spi display:
  #if (!defined usei2c && defined useoled1 && defined ESP32) //one display, ESP32/Hardware SPI Mode
      #define OLED_MISO   GPIO_NUM_19
      //this is just a unused GPIO pin - SPI Lib needs a MISO Pin, display off course not :D
      #define OLED_MOSI   GPIO_NUM_33
      #define OLED_CLK    GPIO_NUM_32
      #define OLED_DC     GPIO_NUM_25
      //OLED1 on OLED1 Connector:
        #define OLED1_CS    GPIO_NUM_26
        #define OLED1_RESET GPIO_NUM_27
      //OLED1 on OLED2 Connector:
        //#define OLED1_CS    GPIO_NUM_14
        //#define OLED1_RESET GPIO_NUM_12
        //#define OLED1_ROTATION 0
  #endif

  #if (!defined usei2c && defined useoled2 && defined ESP32) //2nd display, ESP32/Hardware SPI Mode
      #define OLED2_CS    GPIO_NUM_14
      #define OLED2_RESET GPIO_NUM_12
  #endif

  #ifdef ng
    #define M365SerialGPIO GPIO_NUM_23 //Wemos board
  #else
    #define UART2RX GPIO_NUM_23 //PCB v180723
    #define UART2TX GPIO_NUM_22 //PCB v180723
    #define UART2RXunused GPIO_NUM_21 //PCB v180723; ESP32 does not support RX or TX only modes - so we remap the rx pin to a unused gpio during sending
  #endif
#endif //board define groups


//define screen language
#define LANGUAGE_EN
//#define LANGUAGE_FR //translation kindly provided by Technoo' Loggie (Telegram Handle @TechnooLoggie)
//#define LANGUAGE_DE

//Serial UART Setup for Debugging and M365 Connection
  #define DebugSerial Serial //Debuguart = default Serial Port/UART0
  #define M365UARTINDEX 2
  // wemos test board
  
  #ifdef ng
    #define M365SerialInit M365Serial.begin(115200,SERIAL_8N1, M365SerialGPIO, -1);
    #define M365SerialRX switch2RX();
    #define M365SerialTX switch2TX();
  #else
  #endif  

  //#define Serial1RX M365Serial.begin(115200,SERIAL_8N1, UART2RX, -1)

//M365 - serial connection - regarding a patch in the sdk and general explanations:
  /* ATTENZIONE - the Arduino Serial Library uses HardwareSerial from the arduino-esp32 core which uses "esp32-hal-uart.cpp" driver from esp-idf which runs on RTOS
  * ESP32 has 128Byte Hardware RX Buffer
  * esp32-hal-uart uses interrupts, but default config toggles the rx-event only after 112 received bytes
  * this leads to the problem -> on m365 we must listen to the packets sent by other nodes (bms/esc/ble modules) and send our stuff with the right timing
  * so long explanation, short solution:
  * in esp32-hal-uart.c, in void uartEnableInterrupt(uart_t* uart) change
  * uart->dev->conf1.rxfifo_full_thrhd = 112;
  * to
  *  uart->dev->conf1.rxfifo_full_thrhd = 1;
  */
  //custom pins see here: http://www.iotsharing.com/2017/05/how-to-use-serial-arduino-esp32-print-debug.html?m=1
  //enable rx/tx only  modes see: https://github.com/esp8266/Arduino/blob/master/cores/esp8266/HardwareSerial.h

//helpers
  #define _max(a,b) ((a)>(b)?(a):(b))
  #define _min(a,b) ((a)<(b)?(a):(b))

//M365
  #define m365packettimeout  500 //timeout in ms after last received packet for showing connection-error
  #define housekeepertimeout 3000 //duration between housekeeper-datarequests



  extern uint8_t i;
  extern char tmp1[200];
  extern char tmp2[200];
  extern uint16_t tmpi;
  extern char chipid[7];
  extern char mac[12];
  extern unsigned int lastprogress;

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







    extern const char *apssid;
    extern const char *appassword;

//#define staticip //use static IP in Client Mode, Comment out for DHCP
    #ifdef staticip //static IP for Client Mode, in AP Mode default is 192.168.4.1/24
      IPAddress ip(192,168,0,149);
      IPAddress gateway(192,168,0,1);
      IPAddress dns(192,168,0,1);
      IPAddress subnet(255,255,255,0);
    #endif

#define wlanconnecttimeout 10000 //timeout for connecting to one of  the known ssids
#define wlanapconnecttimeout 600000 //timeout in ap mode until client needs to connect / ap mode turns off

//Status LED?
  //#define usestatusled
#ifdef usestatusled
  #define led GPIO_NUM_2
  extern int ledontime;
  extern int ledofftime;
  extern unsigned long ledcurrenttime;
#endif

//advanced settings for debugging

  //#define developermode //"master switch" to remove all useful debug stuff, beside OLED-function only WiFi & OTA will be enabled
    #ifdef developermode
      //detailed config
        #define usetelnetserver
        #define usepacketserver
        #define userrawserver      
      //DEBUG Settings
        //#define debug_dump_states //dump state machines state
        //#define debug_dump_rawpackets //dump raw packets to Serial/Telnet
        //#define debug_dump_packetdecode //dump infos from packet decoder
    #endif
  
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
  #if (!defined useoled1 && defined useoled2)
      #error "useoled2 defined, but useoled1 not defined"
  #endif

  #if (!defined ESP32 && !defined usei2c)
      #error "Hardware SPI only tested on ESP32!!!"
  #endif


  #if (defined useoled1 || defined useoled2)
      #include <Adafruit_SSD1306.h>
  #endif

  #ifndef usetelnetserver
    #ifdef usepacketserver
      #error "packetserver requires telnetserver"
    #endif
    #ifdef userawserver
      #error "rawserver requires telnetserver"
    #endif
  #endif

#endif