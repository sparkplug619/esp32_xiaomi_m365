#ifndef bssd_h
#define bssd_h

  #include "definitions.h"

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
  #define oled_clock 4000000UL

  //debug pins
  #define debuggpio1 GPIO_NUM_4
  #define debuggpio2 GPIO_NUM_16

  #ifndef usengfeatuart
    #define UART2RX GPIO_NUM_23 //PCB v180723
    #define UART2TX GPIO_NUM_22 //PCB v180723
    #define UART2RXunused GPIO_NUM_21 //PCB v180723; ESP32 does not support RX or TX only modes - so we remap the rx pin to a unused gpio during sending
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