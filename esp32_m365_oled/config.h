//display config

  #define useoled1 //comment out to disable oled functionality
  #define useoled2 //comment out if you use only one display
  #define OLED1_ROTATION 2 //0 = normal, 1= 90, 2=180, 3=270° CW
  #define OLED2_ROTATION 0 //0 = normal, 1= 90, 2=180, 3=270° CW

  //#define usei2c //comment out if using spi

  #if (defined usei2c && defined useoled1) //one display, I2C Mode
      //#define oled_scl GPIO_NUM_4 //working wemos fake board
      //#define oled_sda GPIO_NUM_16 //working wemos fake board
      #define oled_scl GPIO_NUM_32 //SCLK Pad on PCB
      #define oled_sda GPIO_NUM_33 //MOSI Pad on PCB
      //#define oled_scl GPIO_NUM_4 //should be used on esp8266
      //#define oled_sda GPIO_NUM_5 //should be used on esp8266
    #define oled_reset -1
    #define oled1_address 0x3C
  #endif


  #if (defined usei2c && defined useoled2) //2nd display, i2c mode
      #define oled2_address 0x3D
  #endif

  #if (!defined usei2c && defined useoled1 && defined ESP32) //one display, ESP32/Hardware SPI Mode
      #define OLED_MISO   GPIO_NUM_19 //this is just a unused GPIO pin - SPI Lib needs a MISO Pin, display off course not :D
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

//Serial UART Setup for Debugging and M365 Connection
  #ifdef ESP32
    #define DebugSerial Serial //Debuguart = default Serial Port/UART0
    HardwareSerial M365Serial(2);  // UART2 for M365
    /* wemos test board
    #define UART2RX GPIO_NUM_23 //Wemos board
    #define UART2TX GPIO_NUM_5 //Wemos board
    #define UART2RXunused GPIO_NUM_19 //TTGO Test board; ESP32 does not support RX or TX only modes - so we remap the rx pin to a unused gpio during sending
    */
    #define UART2RX GPIO_NUM_23 //PCB v180723
    #define UART2TX GPIO_NUM_22 //PCB v180723
    #define UART2RXunused GPIO_NUM_21 //PCB v180723; ESP32 does not support RX or TX only modes - so we remap the rx pin to a unused gpio during sending

    #define M365SerialFull M365Serial.begin(115200,SERIAL_8N1, UART2RX, UART2TX);
    //#define Serial1RX M365Serial.begin(115200,SERIAL_8N1, UART2RX, -1)
    #define M365SerialTX M365Serial.begin(115200,SERIAL_8N1, UART2RXunused, UART2TX);
  #elif defined(ESP8266)
    #define DebugSerial Serial1 //UART1 is TX only, will be mapped to GPIO2 (-> Debug output will be there)
    #define M365Serial Serial  // UART0 will be remapped to GPIO 13(RX) / 15 (TX) and used for M365 Communication
    #define M365SerialFull M365Serial.begin(115200,SERIAL_8N1, (SerialMode)UART_FULL); M365Serial.swap();
    #define M365SerialTX M365Serial.begin(115200,SERIAL_8N1, (SerialMode)UART_TX_ONLY); M365Serial.swap();
  #endif

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



//Wifi - CHANGE IT TO YOUR OWN SETTINGS!!!
    #define maxssids 1
    #define ssid1 "m365dev"
    #define password1 "h5fj8bvothrfd65b4"
    #define ssid2 "..."
    #define password2 "..."
    #define ssid3 "..."
    #define password3 "..."

//SSID/Pass for Access-Point Mode - CHANGE IT TO YOUR OWN SEQUENCE!!!
    const char *apssid ="m365oled";
    const char *appassword="365";

//Over The Air Firmwareupdates - password - CHANGE IT TO YOUR OWN SEQUENCE!!!
    #define OTApwd "h5fj8ovbthrfd65b4"

//#define staticip //use static IP in Client Mode, Comment out for DHCP
    #ifdef staticip //static IP for Client Mode, in AP Mode default is 192.168.4.1/24
      IPAddress ip(192,168,0,149);
      IPAddress gateway(192,168,0,1);
      IPAddress dns(192,168,0,1);
      IPAddress subnet(255,255,255,0);
    #endif


//advanced settings for debugging

  //#define developermode //"master switch" to remove all useful debug stuff, beside OLED-function only WiFi & OTA will be enabled
    #ifdef developermode
      //detailed config
        #define usetelnetserver //comment out to disable telnet status/telemetrie server, this also disables RAW Server (so only mqtt might be left for leaving wifi activated)
        #define usepacketserver //comment out to disable PACKET Decode on Port 36525
      //DEBUG Settings
        //#define debug_dump_states //dump state machines state
        //#define debug_dump_rawpackets //dump raw packets to Serial/Telnet
        //#define debug_dump_packetdecode //dump infos from packet decoder
    #endif
  

//just some checks of the above defines, don't change anything below!
//if something is wrong with the above defined things the next lines will throw an error
//solution is to fix the error in the defines above!

  //OLED
  #if (!defined useoled1 && defined useoled2)
      #error "useoled2 defined, but useoled1 not defined"
  #endif

  #if (!defined ESP32 && !defined usei2c)
      #error "Hardware SPI only tested on ESP32!!!"
  #endif

//and the required includes...
  #ifdef ESP32
    #include <WiFi.h>
    #include <WiFiUdp.h>
    #include <Update.h> //needed for arduinoOTA on esp32
    #include <Preferences.h>
    
  #elif defined(ESP8266)
    #error "Preferences not supported on ESP8266"
    #include <ESP8266WiFi.h>
    #include <WiFiUdp.h>
    WiFiEventHandler stationConnectedHandler;
    WiFiEventHandler stationDisconnectedHandler;
  #else
    #error Platform not supported
  #endif

  //#include <endian.h>
  #include <ArduinoOTA.h>

  #if (defined useoled1 || defined useoled2)
      #include <Adafruit_SSD1306.h>
  #endif

