#ifndef bsttgo_h
#define bsttgo_h

	#include "definitions.h"

	#define usei2c //comment out for SPI
	#define useoled1 //comment out to disable oled functionality
	#define OLED1_ROTATION 0 //0 = normal, 1= 90, 2=180, 3=270° CW
	#define oled_scl 15
	#define oled_sda 4
	#define oled_reset 16
	#define oled_doreset true
	#define oled1_address 0x3C
	#define oled_clock 800000UL
	
	//debug pins
	#define debuggpio1 GPIO_NUM_14
	#define debuggpio2 GPIO_NUM_27

	//headunit gpios
		//ttgo headunit testboard without leds
		#define sensor_brake GPIO_NUM_32
		#define sensor_throttle GPIO_NUM_33

	#ifndef usengfeatuart
		#define UART2RX GPIO_NUM_25
		#define UART2TX GPIO_NUM_26 
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