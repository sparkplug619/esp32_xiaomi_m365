#ifndef bswemos_h
#define bswemos_h

  #include "definitions.h"

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
  #define oled_clock 800000UL

  //debug pins
  #define debuggpio1 GPIO_NUM_26
  #define debuggpio2 GPIO_NUM_27


  #ifndef usengfeatuart
    #define UART2RX GPIO_NUM_23 //Wemos board
    #define UART2TX GPIO_NUM_5 //Wemos board for scooter usage :D
    //#define UART2TX GPIO_NUM_27 //Wemos board for logic-analyzer-testing :D
    #define UART2RXunused GPIO_NUM_19 //PCB v180723; ESP32 does not support RX or TX only modes - so we remap the rx pin to a unused gpio during sending
  #endif

  #ifdef usestatusled
    #define led GPIO_NUM_2
  #endif

    #ifdef staticip //static IP for Client Mode, in AP Mode default is 192.168.4.1/24
      IPAddress ip(192,168,0,149);
      IPAddress gateway(192,168,0,1);
      IPAddress dns(192,168,0,1);
      IPAddress subnet(255,255,255,0);
    #endif

#endif