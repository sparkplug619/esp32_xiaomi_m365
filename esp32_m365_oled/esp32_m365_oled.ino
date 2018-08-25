//Screens/Einheitenanzeige wie bei single-screen /Tripinfo

//current version working on esp32 & esp8266
//needs patch in esp32-arduino-core/esp32-hal-uart.c -> see comments in uart-section below and patch yourself or use https://github.com/smartinick/arduino-esp32
//needs patched Adafruit_SSD1306 Library (custom pins, higher clock speer) -> compare with base or clone from https://github.com/smartinick/Adafruit_SSD1306

//Test on scooter:
  //dual-timeout-screen: add ssid/ip display on 2nd screen
  //i2c on custom pcb with clock/data on spi pads
  //#define chargesubscreens 2 -> correct for dual display version?

//preferences: 8.5 / 10", UART-Send-Off, Beep on backward, 10s/12s, Reboot esp32 ???, wlan restart (ssid search/reset timeouts)Komoot ON/Connect


//julien lenkerverbinder

#ifdef ESP32
  #include <WiFi.h>
  #include <WiFiUdp.h>
  #include <Update.h> //needed for arduinoOTA on esp32
  #include <Preferences.h>
  
#elif defined(ESP8266)
  #warning "Preferences not supported on ESP8266"
  #include <ESP8266WiFi.h>
  #include <WiFiUdp.h>
  WiFiEventHandler stationConnectedHandler;
  WiFiEventHandler stationDisconnectedHandler;
#else
  #error Platform not supported
#endif

//#include <endian.h>
#include <ArduinoOTA.h>

#define swversion "18.08.25"

//scooter config
  //#define batt12s //shows 12 cells on battery/charge screens
  //#define 10inch //NOT IMPLEMENTED -> apply 8,5" -> 10" wheel speed/distance factor = 10/8,5=1,1764705882352941176470588235294

//Hardwareconfig
  #define useoled1 //comment out to disable oled functionality
  #define useoled2
  //#define usei2c //if not defined, spi will be used - SPI NOT IMPLEMENTED YET


  //#define developermode //"master switch" to remove all useful debug stuff, beside OLED-function only WiFi & OTA will be enabled

  #ifdef developermode
    //detailed config
      //#define usewlanclientmode //NOT IMPLEMENTED comment out to disable Wifi Client functionality
      //#define usewlanapmode //NOT IMPLEMENTED  comment out to disable Wifi Access Poiint functionality
      #define usetelnetserver //comment out to disable telnet status/telemetrie server, this also disables RAW Server (so only mqtt might be left for leaving wifi activated)
      //#define userawserver //comment out to disable RAW Serial Data BUS Stream on Port 36524, NOT VERIFIED against wired-data-stream
      #define usepacketserver //comment out to disable PACKET Decode on Port 36525
      //#define usemqtt //NOT IMPLEMENTED comment out to disable mqtt functionality
      //#define usestatusled //if enabled update "#define led" with gpionum; 10% dutycycle while searching for known wlans, 50% dutycycle while in wlan client/accesspoint mode, ON if client connected, off if wlan is turned off
    //DEBUG Settings
      //#define debug_dump_states //dump state machines state
      //#define debug_dump_rawpackets //dump raw packets to Serial/Telnet
      //#define debug_dump_packetdecode //dump infos from packet decoder
  #endif
  

//Configuration Settings (should be adopted to personal desires)

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
  
  //#define staticip //use static IP in Client Mode, Comment out for DHCP
    #ifdef staticip //static IP for Client Mode, in AP Mode default is 192.168.4.1/24
      IPAddress ip(192,168,0,149);
      IPAddress gateway(192,168,0,1);
      IPAddress dns(192,168,0,1);
      IPAddress subnet(255,255,255,0);
    #endif

  //MQTT - NOT USED YET
    #ifdef usemqtt
      #include <PubSubClient.h>
      #define mqtt_clientID "m365"
      #define mqtt_server "192.168.0.31"
      #define mqtt_port 1883
    #endif

  //Over The Air Firmwareupdates - password - CHANGE IT TO YOUR OWN SEQUENCE!!!
    #define OTApwd "h5fj8ovbthrfd65b4"



//usually nothing must/should be changed below

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

#if (defined usei2c && defined useoled1) //one display, I2C Mode
  #ifdef ESP32
    //#define oled_scl GPIO_NUM_4 //working wemos fake board
    //#define oled_sda GPIO_NUM_16 //working wemos fake board
    #define oled_scl GPIO_NUM_32 //SCLK Pad on PCB
    #define oled_sda GPIO_NUM_33 //MOSI Pad on PCB
  #endif
  #ifdef ESP8266
    #define oled_scl GPIO_NUM_4
    #define oled_sda GPIO_NUM_5
  #endif
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
      #define OLED1_ROTATION 2
    //OLED1 on OLED2 Connector:
      //#define OLED1_CS    GPIO_NUM_14
      //#define OLED1_RESET GPIO_NUM_12
      //#define OLED1_ROTATION 0
#endif

#if (!defined usei2c && defined useoled2 && defined ESP32) //2nd display, ESP32/Hardware SPI Mode
    #define OLED2_CS    GPIO_NUM_14
    #define OLED2_RESET GPIO_NUM_12
#endif


#if (defined usei2c && defined useoled1)
    Adafruit_SSD1306 display1(oled_reset);
#endif
#if (defined usei2c && defined useoled2)
    Adafruit_SSD1306 display2(oled_reset);
#endif

#if (!defined usei2c && defined useoled1)
    //software spi
    //Adafruit_SSD1306 display1(OLED_MOSI, OLED_CLK, OLED_DC, OLED1_RESET, OLED1_CS);
    //hardware spi with patched Adafruit_SSD1306 Library:
    Adafruit_SSD1306 display1(OLED_MISO, OLED_MOSI, OLED_CLK, OLED_DC, OLED1_RESET, OLED1_CS);
#endif
#if (!defined usei2c && defined useoled2)
    //software spi
    //Adafruit_SSD1306 display2(OLED_MOSI, OLED_CLK, OLED_DC, OLED1_RESET, OLED2_CS);
    //hardware spi
    Adafruit_SSD1306 display2(OLED_MISO, OLED_MOSI, OLED_CLK, OLED_DC, OLED2_RESET, OLED2_CS);
#endif

#if (defined useoled1 || defined useoled2)
    #include <Adafruit_GFX.h>
    #include <Fonts/FreeSansOblique24pt7b.h>
    #include <Fonts/FreeSans18pt7b.h>
    #define fontbig FreeSansOblique24pt7b
    #define fontbigbaselinezero 32
    #define fontbigheight 36
    #include <Fonts/FreeSansBold9pt7b.h>
    #include <Fonts/FreeSans9pt7b.h>
    #define fontsmall FreeSansBold9pt7b
    #define fontsmallbaselinezero 6
    #define fontsmallheight 18
    #define line1 0
    #define line2 8
    #define line3 16
    #define line4 24
    #define line5 32
    #define line6 40
    #define line7 48
    #define line8 56
    boolean updatescreen = false;
    boolean updatescreen2 = false; //workaround for i2c/uart timing for 2 displays
    #define oledrefreshanyscreen 200 //refresh oled screen every xx ms (if there is new data)
    unsigned long olednextrefreshtimestamp = 0;
    unsigned long timeout_oled = 0;
    boolean oled_blink=true;
    #define oledwidth 128
    #define oledheight 64
    #define baselineoffset 13
    #define linespace 1
    #define dataoffset 8

    static const unsigned char PROGMEM scooter [] PROGMEM = {
      0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x30, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9c, 0x80, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x76, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x80, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x80, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x01, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x0a, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x13, 0x80, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x30, 0xe0, 
      0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x70, 0x30, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x98, 0x78, 
      0x00, 0x22, 0x80, 0x00, 0x00, 0x01, 0x27, 0x18, 0x00, 0x20, 0x40, 0x00, 0x00, 0x1e, 0x59, 0xd8, 
      0x00, 0x21, 0x20, 0x00, 0x07, 0xc0, 0xb6, 0x10, 0x00, 0x66, 0x90, 0x01, 0xf0, 0x09, 0xbd, 0x30, 
      0x01, 0xc5, 0x08, 0x7c, 0x00, 0x06, 0xd3, 0xb0, 0x02, 0x02, 0x87, 0x00, 0x00, 0x03, 0x79, 0x90, 
      0x04, 0x61, 0x40, 0x00, 0x00, 0xec, 0x2f, 0xf0, 0x09, 0x20, 0x80, 0x00, 0x32, 0x21, 0x0a, 0xa0, 
      0x08, 0xc5, 0x8c, 0x04, 0x80, 0xfc, 0x90, 0x40, 0x10, 0x4c, 0x73, 0x60, 0x3f, 0x50, 0x61, 0x80, 
      0x10, 0x0a, 0x73, 0x0f, 0xc0, 0x50, 0x1e, 0x00, 0x13, 0x62, 0xcf, 0xf0, 0x00, 0x50, 0x00, 0x00, 
      0x14, 0x13, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x11, 0x2d, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 
      0x10, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xa6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
#endif

//WLAN
  uint8_t currentssidindex = 0;
  uint8_t apnumclientsconnected=0;
  uint8_t apnumclientsconnectedlast=0;

  uint8_t wlanstate = 0;
  uint8_t wlanstateold = 0;
  #define wlanoff 0
  #define wlanturnapon 1
  #define wlanturnstaon 2
  #define wlansearching 3
  #define wlanconnected 4
  #define wlanap 5
  #define wlanturnoff 6 

  #define wlanconnecttimeout 10000 //timeout for connecting to one of  the known ssids
  #define wlanapconnecttimeout 60000 //timeout in ap mode until client needs to connect / ap mode turns off
  unsigned long wlanconnecttimestamp = 0;

#ifdef usetelnetserver
//TELNET
  WiFiServer telnetserver(36523);
  WiFiClient telnetclient;
  #ifdef userawserver
    WiFiServer rawserver(36524);
    WiFiClient rawclient;
  #endif
  #ifdef usepacketserver
    WiFiServer packetserver(36525);
    WiFiClient packetclient;
  #endif
  uint8_t telnetstate = 0;
  uint8_t telnetstateold = 0;
  uint8_t telnetrawstate = 0;
  uint8_t telnetrawstateold = 0;
  
  #define telnetoff 0
  #define telnetturnon 1
  #define telnetlistening 2
  #define telnetclientconnected 3
  #define telnetturnoff 4
  #define telnetdisconnectclients 5

  #define userconnecttimeout 300000 //timeout for connecting to telnet/http after wlan connection has been established with ap or client
  #define telnetrefreshanyscreen 100 //refresh telnet screen every xx ms
  #define telnetrefreshrawarrayscreen 500 //refresh telnet screen every xx ms
  unsigned long userconnecttimestamp = 0;
  unsigned long telnetnextrefreshtimestamp = 0;
  
  //telnet screens
  #define ts_telemetrie 0
  #define ts_statistics 1
  #define ts_esc_raw 2
  #define ts_ble_raw 3
  #define ts_bms_raw 4
  #define ts_x1_raw 5
  #define ts_esc_array 6
  #define ts_ble_array 7
  #define ts_bms_array 8
  #define ts_x1_array 9
  uint8_t telnetscreen = ts_statistics;
  uint8_t telnetscreenold = telnetscreen;

  //ANSI
  String ansiPRE  = "\033"; // escape code
  String ansiHOME = "\033[H"; // cursor home
  String ansiESC  = "\033[2J"; // esc
  String ansiCLC  = "\033[?25l"; // invisible cursor
  String ansiEND  = "\033[0m";   // closing tag for styles
  String ansiBOLD = "\033[1m";
  String ansiRED  = "\033[41m"; // red background
  String ansiGRN  = "\033[42m"; // green background
  String ansiBLU  = "\033[44m"; // blue background
  String ansiREDF = "\033[31m"; // red foreground
  String ansiGRNF = "\033[34m"; // green foreground
  String ansiBLUF = "\033[32m"; // blue foreground
  String BELL     = "\a";
#endif


//M365 - serial
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
  //#define UART2RX GPIO_NUM_5 //TTGO Test board
  //#define UART2TX GPIO_NUM_17 //TTGO Test board
  //#define UART2RXunused GPIO_NUM_23 //TTGO Test board; ESP32 does not support RX or TX only modes - so we remap the rx pin to a unused gpio during sending

  //custom pins see here: http://www.iotsharing.com/2017/05/how-to-use-serial-arduino-esp32-print-debug.html?m=1
  //enable rx/tx only  modes see: https://github.com/esp8266/Arduino/blob/master/cores/esp8266/HardwareSerial.h
  #ifdef ESP32
    #define DebugSerial Serial //Debuguart = default Serial Port/UART0
    HardwareSerial M365Serial(2);  // UART2for M365
    /* wemos test board
    #define UART2RX GPIO_NUM_23 //Wemos board
    #define UART2TX GPIO_NUM_5 //Wemos board
    #define UART2RXunused GPIO_NUM_19 //TTGO Test board; ESP32 does not support RX or TX only modes - so we remap the rx pin to a unused gpio during sending
    */
    #define UART2RX GPIO_NUM_23 //PCB v180723
    #define UART2TX GPIO_NUM_22 //PCB v180723
    //#define UART2RX GPIO_NUM_22 //PCB v180723 FIX SWAP
    //#define UART2TX GPIO_NUM_23 //PCB v180723 FIX SWAP
    #define UART2RXunused GPIO_NUM_21 //PCB v180723; ESP32 does not support RX or TX only modes - so we remap the rx pin to a unused gpio during sending

    #define M365SerialFull M365Serial.begin(115200,SERIAL_8N1, UART2RX, UART2TX);
    //#define Serial1RX M365Serial.begin(115200,SERIAL_8N1, UART2RX, -1)
    #define M365SerialTX M365Serial.begin(115200,SERIAL_8N1, UART2RXunused, UART2TX);
  #elif defined(ESP8266)
    #define DebugSerial Serial1 //UART1 is TX only, will be mapped to GPIO2 (-> Debug output will be there)
    #define M365Serial Serial  // UART0 will be remapped to GPIO 13(RX) / 15 (TX) and used for M365 Communication
    //#define UART2RX 2 //Wemos board
    //#define UART2TX 4 //Wemos board

    #define M365SerialFull M365Serial.begin(115200,SERIAL_8N1, (SerialMode)UART_FULL); M365Serial.swap();
    //#define Serial1RX M365Serial.begin(115200,SERIAL_8N1, UART_RX_ONLY, UART2RX, UART2TX); M365Serial.swap();
    #define M365SerialTX M365Serial.begin(115200,SERIAL_8N1, (SerialMode)UART_TX_ONLY); M365Serial.swap();


    //TEST M365Serial.setRxBufferSize(1);
  #endif

//M365 - serial receiver
  #define maxlen 256 //serial buffer size in bytes
  uint8_t crc1=0;
  uint8_t crc2=0;
  uint16_t crccalc=0;
  uint8_t sbuf[maxlen];
  uint8_t len=0;
  uint8_t readindex=0;
  uint8_t m365receiverstate = 0;
  uint8_t m365receiverstateold = 0;
  #define m365receiveroff 0 //Serial1 will not be read...
  #define m365receiverready 1 //reading bytes until we find 0x55
  #define m365receiverpacket1 2 //preamble 0x55 received, waiting for 0xAA
  #define m365receiverpacket2 3 //len preamble received waiting for LEN
  #define m365receiverpacket3 4 //payload this state is kept until LEN bytes received
  #define m365receiverpacket4 5 //checksum receiving checksum
  #define m365receiverpacket5 6 //received a packet, test for checksum
  #define m365receiverstorepacket 7 //packet received, checksum valid, store in array, jump back to receiverready for next packet, set newpacket flag

//M365 - packets
  uint8_t m365packetstate = 0;
  uint8_t m365packetstateold = 0;
  #define m365packetidle 0
  #define m365newpacket 1
  //offsets in sbuf
  #define i_address 0
  #define i_hz 1
  #define i_offset 2
  #define i_payloadstart 3
  #define address_ble 0x20 //actively sent data by BLE Module with gas/brake & mode values
  #define address_x1 0x21 //actively sent data by ?BLE? with status like led on/off, normal/ecomode...
  #define address_esc 0x23 
  #define address_bms_request 0x22 //used by apps to read data from bms
  #define address_bms 0x25 //data from bms sent with this address (only passive if requested via address_esc_request)

//M365 - Statistics
  uint16_t packets_rec=0;
  uint16_t packets_crcok=0;
  uint16_t packets_crcfail=0;
  uint16_t packets_rec_bms=0;
  uint16_t packets_rec_esc=0;
  uint16_t packets_rec_x1=0;
  uint16_t packets_rec_ble=0;
  uint16_t packets_rec_unhandled=0;

  uint16_t packetsperaddress[256];
  uint16_t requests_sent_bms=0;
  uint16_t requests_sent_esc=0;

  int16_t speed_min = 0;
  int16_t speed_max = 0;
  int16_t current_min = 0;
  int16_t current_max = 0;
  int32_t watt_min = 0;
  int32_t watt_max = 0;
  uint8_t tb1_min = 255; //temperature batt 1 min
  uint8_t tb1_max = 0; //temperature batt 1 max
  uint8_t tb2_min = 255;  //temperature batt 2 min
  uint8_t tb2_max = 0;  //temperature batt 2 max
  uint16_t te_min = 1000;  //temperature esc min
  uint16_t te_max = 0;  //temperature esc max
  uint16_t lowest=10000; //Cell Voltages - Lowest
  uint16_t highest=0; //Cell Voltages - Highest



  #define m365packettimeout  500 //timeout in ms after last received packet for showing connection-error
  unsigned long m365packettimestamp = 0;
  unsigned long m365packetlasttimestamp = 0;
  unsigned long duration_requestcycle=0;
  unsigned long timestamp_requeststart=0;


//M365 Device Buffers & Structs
  typedef struct {
    uint8_t u1[1];
    uint8_t throttle;
    uint8_t brake;
    //uint8_t u2[509];
  }__attribute__((packed))   blestruct;

  typedef struct {
    uint8_t mode; //offset 0x00 mode: 0-stall, 1-drive, 2-eco stall, 3-eco drive
    uint8_t battleds;  //offset 0x01 battery status 0 - min, 7(or 8...) - max
    uint8_t light;  //offset 0x02 0= off, 0x64 = on
    uint8_t beepaction;  //offset 0x03 "beepaction" ?
    //uint8_t u1[508];
  }__attribute__((packed))   x1struct;

  typedef struct {
    uint16_t u1[0x10]; //offset 0-0x1F
    char serial[14]; //offset 0x20-0x2D
    char pin[6]; //offset 0x2E-0x33
    uint8_t fwversion[2]; //offset 0x34-035,  e.g. 0x133 = 1.3.3
    uint16_t u2[10]; //offset 0x36-0x49
    uint16_t remainingdistance; //offset 0x4a-0x4B e.g. 123 = 1.23km
    uint16_t u3[20]; //offset 0x4C-0x73
    uint16_t ontime1; //offset 0x74-0x75 power on time in seconds
    uint16_t triptime; //offset 0x76-0x77 trip time in seconds
    uint16_t u4[2]; //offset 0x78-0x7C
    uint16_t frametemp1; //offset 0x7C-0x7D /10 = temp in °C //Unused on M365/FW1.4.0
    uint16_t u5[54]; //offset 0x7e-0xe9
    uint16_t ecomode; //offset 0xEA-0xEB; on=1, off=0
    uint16_t u6[5]; //offset 0xec-0xf5
    uint16_t kers; //offset 0xf6-0xf7; 0 = weak, 1= medium, 2=strong
    uint16_t cruisemode; //offset 0xf8-0xf9, off 0, on 1
    uint16_t taillight; //offset 0xfa-0xfb, off 0, on 2
    uint16_t u7[50]; //offset  0xfc-0x15f
    uint16_t error; //offset 0x160-0x161
    uint16_t u8[3]; //offset 0x162-0x167
    uint16_t battpercent; //offset 0x168-0x169
    int16_t speed; //0x16A-0x16B /1000 in km/h, negative value = backwards...
    uint16_t averagespeed; //0x16C-0x16D /1000 in km/h?
    uint32_t totaldistance; //0x16e-0x171 /1000 in km
    uint16_t tripdistance; //0x172-0x173
    uint16_t ontime2; //offset 0x174-0x175 power on time 2 in seconds
    uint16_t frametemp2; //0x176-0x177 /10 = temp in °C
    uint16_t u10[68]; //offset 0x178-0x200 
  }__attribute__((packed))   escstruct;

  typedef struct {
    uint16_t u1[0x10]; //offset 0-0x1F
    char serial[14]; //offset 0x20-0x2D
    uint8_t fwversion[2]; //offset 0x2E-0x2f e.g. 0x133 = 1.3.3
    uint16_t totalcapacity; //offset 0x30-0x31 mAh
    uint16_t u2a[2]; //offset 0x32-0x35
    uint16_t cycles; //offset 0x36-0x37
    uint16_t chargingtimes; //offset 0x38-0x39
    uint16_t u2b[3]; //offset 0x3a-0x3f
    uint16_t proddate; //offset 0x40-0x41
        //fecha a la batt 7 MSB->año, siguientes 4bits->mes, 5 LSB ->dia ex:
      //b 0001_010=10, año 2010
      //        b 1_000= 8 agosto
      //            b  0_1111=15 
      //  0001_0101_0000_1111=0x150F 
    uint16_t u3[0x10]; //offset 0x42-0x61
    uint16_t remainingcapacity; //offset 0x62-0x63
    uint16_t remainingpercent; //offset 0x64-0x65
    int16_t current; //offset 0x66-67 - negative = charging; /100 = Ampere
    uint16_t voltage; //offset 0x68-69 /10 = Volt
    uint8_t temperature[2]; //offset 0x6A-0x6B -20 = °C
    uint16_t u4[5]; //offset 0x6C-0x75
    uint16_t health; //offset 0x76-0x77; 0-100, 60 schwellwert "kaputt"
    uint16_t u5[4]; //offset 0x78-0x7F
    uint16_t Cell1Voltage; //offset 0x80-0x81
    uint16_t Cell2Voltage; //offset 0x82-0x83
    uint16_t Cell3Voltage; //offset 0x84-0x85
    uint16_t Cell4Voltage; //offset 0x86-0x87
    uint16_t Cell5Voltage; //offset 0x88-0x89
    uint16_t Cell6Voltage; //offset 0x8A-0x8B
    uint16_t Cell7Voltage; //offset 0x8C-0x8D
    uint16_t Cell8Voltage; //offset 0x8E-0x8F
    uint16_t Cell9Voltage; //offset 0x90-0x91
    uint16_t Cell10Voltage; //offset 0x92-0x93
    uint16_t Cell11Voltage; //offset 0x94-0x95 not stock, custom bms with 12S battery, 0 if not connected
    uint16_t Cell12Voltage; //offset 0x96-0x97 not stock, custom bms with 12S battery, 0 if not connected
    //uint16_t u6[178]; //offset 0x98-0x
  }__attribute__((packed))  bmsstruct;

  uint8_t bledata[512];
  uint8_t x1data[512];
  uint8_t escdata[512];
  uint8_t bmsdata[512];

  blestruct* bleparsed = (blestruct*)bledata;
  x1struct* x1parsed = (x1struct*)x1data;
  escstruct* escparsed = (escstruct*)escdata;
  bmsstruct* bmsparsed = (bmsstruct*)bmsdata;

  bool newdata = false; //flag - we have update at least one byte in one of the data arrays
  bool senddata = false; //flag - we should send our data request _now_

//M365 - request stuff 
  /*
  request data from esc:   (Read 0x7x 2 words)
  PREAMBLE  LEN  Adr  HZ   Off  LEN  FIX1 FIX2 FIX30x
  0x55 0xaa 0x06 0x20 0x61 0x7c 0x02 0x02 0x28 0x27 CRC1 CRC2
  request data from bms: (Read Serial @ Offset 0x10, 0x10 words)
  PREAMBLE    LEN   Adr   HZ    Off   Len   CRC1  CRC2
  0x55  0xAA  0x03  0x22  0x01  0x10  0x12  0xB7  0xFF
  */

  //packet for bms requests and offsets
    uint8_t request_bms[9] = { 0x55,0xAA,0x03,0x22,0x01,0x10,0x3A,0xB7,0xFF};
    //uint8_t request_bms_serial[9] = { 0x55,0xAA,0x03,0x22,0x01,0x10,0x12,0xB7,0xFF};
    #define bms_request_offset 5
    #define bms_request_len 6
    #define bms_request_crcstart 2
    #define bms_request_crc1 7
    #define bms_request_crc2 8
  //packets for esc requests and offsets
    uint8_t request_esc[12] = { 0x55,0xAA,0x06,0x20,0x61,0x10,0xAC,0x02,0x28,0x27,0xB7,0xFF};
    //uint8_t request_esc_speed[12] = { 0x55,0xAA,0x06,0x20,0x61,0x7c,0x02,0x02,0x28,0x27,0xB7,0xFF};
    #define esc_request_offset 5 
    #define esc_request_len 6
    #define esc_request_throttle 8
    #define esc_request_brake 9
    #define esc_request_crcstart 2
    #define esc_request_crc1 10
    #define esc_request_crc2 11

  //request arrays  
    uint8_t requestindex = 0;
    #define requestmax 8
    uint8_t requests[requestmax][3]= {
        { address_esc, 0xB0, 0x20}, //error, battpercent,speed,averagespeed,totaldistance,tripdistance,ontime2,frametemp2
        { address_esc, 0x25, 0x02}, //remaining distance
        { address_esc, 0x3A, 0x0A}, //ontime, triptime, frametemp1
        { address_bms, 0x31, 0x0A}, //remaining cap & percent, current, voltage, temperature
        { address_bms, 0x40, 0x18}, //cell voltages (Cells 1-10 & 11-12 for 12S Batt/Custom BMS)
        { address_bms, 0x3B, 0x02}, //Battery Health
        { address_bms, 0x10, 0x22}, //serial,fwversion,totalcapacity,cycles,chargingtimes,productiondate
        { address_esc, 0x10, 0x16} //serial,pin,fwversion
    };

  //friendly names
    #define rq_esc_essentials 0x01
    #define rq_esc_remdist 0x02
    #define rq_esc_essentials2 0x04
    #define rq_bms_essentials 0x08
    #define rq_bms_cells 0x10
    #define rq_bms_health 0x20
    #define rq_bms_versioninfos 0x40
    #define rq_esc_versioninfos 0x80

  //mapping oled screens / requeired data
    #define numscreens 5
    uint8_t rqsarray[numscreens] = {
      rq_esc_essentials|rq_esc_remdist|rq_esc_essentials2|rq_bms_essentials|rq_bms_cells|rq_bms_health|rq_bms_versioninfos|rq_esc_versioninfos, //rq_esc_essentials|rq_bms_essentials|rq_esc_essentials2, //screen_stop
      //0xff, //screen_stop, TODO: Request infos like Serial/FW Version only _once_ (when entering subscreen)
      rq_esc_essentials|rq_bms_essentials, //screen_drive
      rq_esc_essentials, //screen_error
      rq_esc_essentials, //screen_timeout
      rq_esc_essentials|rq_bms_essentials|rq_bms_cells //charging
    };

  uint8_t subscribedrequests =  rqsarray[0];

//Screen Switching
  uint8_t screen = 0;
  #define screen_stop 0
  #define screen_drive 1
  #define screen_error 2
  #define screen_timeout 3
  #define screen_charging 4

  uint8_t subscreen = 0;
  uint8_t subscreen = 0;
  uint8_t windowsubpos=0;
  #if !defined useoled2
    #define stopsubscreens 8
  #else  
    #define stopsubscreens 5
  #endif
  #define chargesubscreens 2
  #define gasmin 50
  #define gasmax 190
  #define stopwindowsize (uint8_t)((gasmax-gasmin)/stopsubscreens)
  #define chargewindowsize (uint8_t)((gasmax-gasmin)/chargesubscreens)

//Misc
#ifdef usestatusled
  #ifdef ESP32
    #define led GPIO_NUM_2
  #endif
  #ifdef ESP8266
    #define led 2
  #endif
  int ledontime = 0;
  int ledofftime = 10000;
  unsigned long ledcurrenttime = 100;
#endif
  uint8_t i;
  char tmp1[200];
  char tmp2[200];
  uint16_t tmpi;
  char chipid[7];
  char mac[12];
  unsigned int lastprogress=0;

  unsigned long duration_mainloop=0;
  unsigned long timestamp_mainloopstart=0;
  unsigned long duration_oled=0;
  unsigned long timestamp_oledstart=0;
  unsigned long duration_oled1draw=0;
  unsigned long timestamp_oled1draw=0;
  unsigned long duration_oled1i2c=0;
  unsigned long timestamp_oled1i2c=0;
  unsigned long duration_oled2draw=0;
  unsigned long timestamp_oled2draw=0;
  unsigned long duration_oled2i2c=0;
  unsigned long timestamp_oled2i2c=0;
  unsigned long duration_telnet=0;
  unsigned long timestamp_telnetstart=0;


  #define _max(a,b) ((a)>(b)?(a):(b))
  #define _min(a,b) ((a)<(b)?(a):(b))

//preferences
  Preferences preferences;



void reset_statistics() {
  packets_rec=0;
  packets_crcok=0;
  packets_crcfail=0;
  for(uint16_t j = 0; j<=255; j++) {
    packetsperaddress[j]=0;  
  }
  for(uint16_t j = 0; j<=511; j++) {
    escdata[j]=0;  
    bmsdata[j]=0;  
    bledata[j]=0;  
    x1data[j]=0;  
  }
  speed_min = 0;
  speed_max = 0;
  current_min = 0;
  current_max = 0;
  watt_min = 0;
  watt_max = 0;
  tb1_min = 255;
  tb1_max = 0;
  tb2_min = 255;
  tb2_max = 0;
  te_min = 1000;
  te_max = 0;
} //reset_statistics

void start_m365() {
  subscribedrequests=rqsarray[0];
  M365SerialFull
  m365receiverstate = m365receiverready;
  m365packetstate=m365packetidle;
  reset_statistics();
}  //startm365

void m365_updatestats() {
  //Trip
    if (speed_min>escparsed->speed) { speed_min = escparsed->speed; }
    if (speed_max<escparsed->speed) { speed_max = escparsed->speed; }
    if (current_min>bmsparsed->current) { current_min = bmsparsed->current; }
    if (current_max<bmsparsed->current) { current_max = bmsparsed->current; }
    int32_t currentwatt = (int32_t)((float)(bmsparsed->voltage/100.0f)*(float)bmsparsed->current/100.0f);
    if (watt_min>currentwatt) { watt_min = currentwatt; }
    if (watt_max<currentwatt) { watt_max = currentwatt; }
  //Temperatures
    if (tb1_min>bmsparsed->temperature[0] | tb1_min==0) { tb1_min = bmsparsed->temperature[0]; }
    if (tb1_max<bmsparsed->temperature[0]) { tb1_max = bmsparsed->temperature[0]; }
    if (tb2_min>bmsparsed->temperature[1] | tb2_min==0) { tb2_min = bmsparsed->temperature[1]; }
    if (tb2_max<bmsparsed->temperature[1]) { tb2_max = bmsparsed->temperature[1]; }
    if (te_min>escparsed->frametemp2 | te_min==0) { te_min = escparsed->frametemp2; }
    if (te_max<escparsed->frametemp2) { te_max = escparsed->frametemp2; }
  //Cell Voltages
    highest = 0;
    lowest =10000;
    if (bmsparsed->Cell1Voltage>0) { lowest=_min(lowest,bmsparsed->Cell1Voltage); highest=_max(highest,bmsparsed->Cell1Voltage); }
    if (bmsparsed->Cell2Voltage>0) { lowest=_min(lowest,bmsparsed->Cell2Voltage); highest=_max(highest,bmsparsed->Cell2Voltage); }
    if (bmsparsed->Cell3Voltage>0) { lowest=_min(lowest,bmsparsed->Cell3Voltage); highest=_max(highest,bmsparsed->Cell3Voltage); }
    if (bmsparsed->Cell4Voltage>0) { lowest=_min(lowest,bmsparsed->Cell4Voltage); highest=_max(highest,bmsparsed->Cell4Voltage); }
    if (bmsparsed->Cell5Voltage>0) { lowest=_min(lowest,bmsparsed->Cell5Voltage); highest=_max(highest,bmsparsed->Cell5Voltage); }
    if (bmsparsed->Cell6Voltage>0) { lowest=_min(lowest,bmsparsed->Cell6Voltage); highest=_max(highest,bmsparsed->Cell6Voltage); }
    if (bmsparsed->Cell7Voltage>0) { lowest=_min(lowest,bmsparsed->Cell7Voltage); highest=_max(highest,bmsparsed->Cell7Voltage); }
    if (bmsparsed->Cell8Voltage>0) { lowest=_min(lowest,bmsparsed->Cell8Voltage); highest=_max(highest,bmsparsed->Cell8Voltage); }
    if (bmsparsed->Cell9Voltage>0) { lowest=_min(lowest,bmsparsed->Cell9Voltage); highest=_max(highest,bmsparsed->Cell9Voltage); }
    if (bmsparsed->Cell10Voltage>0) { lowest=_min(lowest,bmsparsed->Cell10Voltage); highest=_max(highest,bmsparsed->Cell10Voltage); }
    #ifdef batt12s
      if (bmsparsed->Cell11Voltage>0) { lowest=_min(lowest,bmsparsed->Cell11Voltage); highest=_max(highest,bmsparsed->Cell11Voltage); }
      if (bmsparsed->Cell12Voltage>0) { lowest=_min(lowest,bmsparsed->Cell12Voltage); highest=_max(highest,bmsparsed->Cell12Voltage); }
    #endif

    //if (bmsparsed->Cell11Voltage>0) { lowest=_min(lowest,bmsparsed->Cell11Voltage); highest=_max(highest,bmsparsed->Cell11Voltage); }
    //if (bmsparsed->Cell12Voltage>0) { lowest=_min(lowest,bmsparsed->Cell12Voltage); highest=_max(highest,bmsparsed->Cell12Voltage); }

} //m365_updatestats


void m365_handlerequests() {
  //find next subscribed index
    uint8_t startindex = requestindex;
    while (!(subscribedrequests&(1<<requestindex)))  {
      //DebugSerial.printf("skipping RQ index %d\r\n",requestindex+1);
      requestindex++;
      if (requestindex==requestmax) {
          //DebugSerial.println("rollover in rqloop1");
          requestindex=0; 
          duration_requestcycle=millis()-timestamp_requeststart;
          timestamp_requeststart=millis(); 
      }
      if (requestindex==startindex) { break; }
    }
  //request data for current index if we are interested in
  if (subscribedrequests&(1<<requestindex)) {
    //DebugSerial.printf("requesting RQ index %d\r\n",requestindex+1);
    if (requests[requestindex][0]==address_bms) {
      request_bms[bms_request_offset] = requests[requestindex][1];
      request_bms[bms_request_len] = requests[requestindex][2];
      crccalc = 0x00;
      for(uint8_t i=bms_request_crcstart;i<bms_request_crc1;i++) {
        crccalc=crccalc + request_bms[i];
      }
      crccalc = crccalc ^ 0xffff;
      request_bms[bms_request_crc1]=(uint8_t)(crccalc&0xff);
      request_bms[bms_request_crc2]=(uint8_t)((crccalc&0xff00)>>8);
      M365Serial.write((unsigned char*)&request_bms,9);
      requests_sent_bms++;
    } //if address address_bms
    if (requests[requestindex][0]==address_esc) {
      request_esc[esc_request_offset] = requests[requestindex][1];
      request_esc[esc_request_len] = requests[requestindex][2];
      request_esc[esc_request_throttle] = bleparsed->throttle;
      request_esc[esc_request_brake] = bleparsed->brake;
      crccalc = 0x00;
      for(uint8_t i=esc_request_crcstart;i<esc_request_crc1;i++) {
        crccalc=crccalc + request_esc[i];
      }
      crccalc = crccalc ^ 0xffff;
      request_esc[esc_request_crc1]=(uint8_t)(crccalc&0xff);
      request_esc[esc_request_crc2]=(uint8_t)((crccalc&0xff00)>>8);
      M365Serial.write((unsigned char*)&request_esc,12);
      requests_sent_esc++;
    } //if address address_esc
  } else {
    //DebugSerial.printf("skipping RQ index %d\r\n",requestindex+1);
  }
  //prepare next requestindex for next call
  requestindex++;
  if (requestindex==requestmax) { 
    //DebugSerial.println("rollover in rqloop2");
    requestindex=0; 
    duration_requestcycle=millis()-timestamp_requeststart;
    timestamp_requeststart=millis();
  }
  //DebugSerial.printf("---REQUEST-- %d\r\n",millis());
} //m365_handlerequests

void m365_handlepacket() {
 if (m365packetstate==m365newpacket) {
    m365packetlasttimestamp = millis()-m365packettimestamp;
    m365packettimestamp=millis();
    packetsperaddress[sbuf[i_address]]++;  
    

    #ifdef debug_dump_packetdecode
      sprintf(tmp1,"[%03d] PACKET Len %02X Addr %02X HZ %02X Offset %02X CRC %04X Payload: ",m365packetlasttimestamp,len,sbuf[i_address],sbuf[i_hz],sbuf[i_offset], crccalc);
      DebugSerial.print(tmp1);
      for(i = 0; i < len-3; i++){
        DebugSerial.printf("%02X ",sbuf[i_payloadstart+i]);
      }
      DebugSerial.println("");
    #endif
    switch (sbuf[i_address]) {
      case address_bms:
          packets_rec_bms++;
          memcpy((void*)& bmsdata[sbuf[i_offset]<<1], (void*)& sbuf[i_payloadstart], len-3);
        break;
      case address_esc:
          packets_rec_esc++;
          memcpy((void*)& escdata[sbuf[i_offset]<<1], (void*)& sbuf[i_payloadstart], len-3);
        break;
      case address_ble:
          packets_rec_ble++;
          memcpy((void*)& bledata[sbuf[i_offset]<<1], (void*)& sbuf[i_payloadstart], len-3);
        break;
      case address_x1:
          packets_rec_x1++;
          memcpy((void*)& x1data[sbuf[i_offset]<<1], (void*)& sbuf[i_payloadstart], len-3);
        break;
      default:
          packets_rec_unhandled++;
        break;
    }
    newdata=true;
    m365packetstate=m365packetidle;
 } //if (m365packetstate==m365newpacket)
 else {
  if ((m365packettimestamp+m365packettimeout)>millis()) {
    //packet timeout
  }
 }
} //m365_handlepacket
  
void m365_receiver() { //recieves data until packet is complete
  uint8_t newbyte;
  if (M365Serial.available()) {
    newbyte = M365Serial.read();
#ifdef userawserver
     if (rawclient && rawclient.connected()) { rawclient.write(newbyte); }
#endif    
    switch(m365receiverstate) {
      case m365receiverready: //we are waiting for 1st byte of packet header 0x55
          if (newbyte==0x55) {
            m365receiverstate = m365receiverpacket1;
          }
        break;
      case m365receiverpacket1: //we are waiting for 2nd byte of packet header 0xAA
          if (newbyte==0xAA) {
            m365receiverstate = m365receiverpacket2;
            }
        break;
      case m365receiverpacket2: //we are waiting for packet length
          len = newbyte+1; //next byte will be 1st in sbuf, this is the packet-type1, len counted from 2nd byte in sbuf
          crccalc=newbyte;
          readindex=0;
          m365receiverstate = m365receiverpacket3;
        break;
      case m365receiverpacket3: //we are receiving the payload
          sbuf[readindex]=newbyte;
          readindex++;
          crccalc=crccalc+newbyte;
          if (readindex==len) {
            m365receiverstate = m365receiverpacket4;
          }
        break;
      case m365receiverpacket4: //we are waiting for 1st CRC byte
          crc1 = newbyte;
          m365receiverstate = m365receiverpacket5;
        break;
      case m365receiverpacket5: //we are waiting for 2nd CRC byte
          crc2 = newbyte;
          crccalc = crccalc ^ 0xffff;
          #ifdef usepacketserver
            if (packetclient && packetclient.connected()) { 
              sprintf(tmp1,"55 AA %02X ", len-1);
              sprintf(tmp2,"%02X %02X [CRCCalc:%04X]\r\n", crc1,crc2,crccalc);
              switch (sbuf[i_address]) {
                case address_bms:
                    packetclient.print(ansiRED);
                    packetclient.print("BMS: ");
                  break;
                case address_esc:
                    packetclient.print(ansiGRN);
                    packetclient.print("ESC: ");
                  break;
                case address_ble:
                    packetclient.print(ansiBLU);
                    packetclient.print("BLE: ");
                  break;
                case address_x1:
                    packetclient.print(ansiBLUF);
                    packetclient.print("X1 : ");
                  break;
                default:
                    packetclient.print(ansiREDF);
                    packetclient.print("???: ");
                  break;
                } //switch                else {
              packetclient.print(ansiEND);
              packetclient.print(tmp1);
              for(i = 0; i < len; i++){
                packetclient.printf("%02X ",sbuf[i]);
              } //for i...
              packetclient.print(tmp2);
            }
          #endif
          #ifdef debug_dump_rawpackets
            sprintf(tmp1,"Packet received: 55 AA %02X ", len-1);
            sprintf(tmp2,"CRC %02X %02X %04X\r\n", crc1,crc2,crccalc);
            DebugSerial.print(tmp1);
            for(i = 0; i < len; i++){
             DebugSerial.printf("%02X ",sbuf[i]);
            }
            DebugSerial.print(tmp2);
          #endif
          packets_rec++;
          if (crccalc==((uint16_t)(crc2<<8)+(uint16_t)crc1)) {
            m365packetstate = m365newpacket;
            packets_crcok++;
          } else {
            packets_crcfail++;
          }
        m365receiverstate = m365receiverready; //reset and wait for next packet
        if (sbuf[i_address]==0x20 && sbuf[i_hz]==0x65 && sbuf[i_offset]==0x00 ) {
          //senddata = true;
          m365_handlerequests();
          DebugSerial.printf("---REQUEST-- %d\r\n",millis());
        }
        break;
    } //switch
    //DebugSerial.printf("#S# %d\r\n",M365Serial.available());
  }//serial available
} //m365_receiver
 

#ifdef usetelnetserver
void telnet_refreshscreen() {
  uint8_t k=0;
  //for(i = 0; i < MAX_SRV_CLIENTS; i++){
    if (telnetclient && telnetclient.connected()){
        if (telnetscreenold!=telnetscreen) { telnetclient.print(ansiESC); telnetscreenold=telnetscreen; }
        telnetclient.print(ansiHOME);
        telnetclient.print(ansiCLC); 
        switch (telnetscreen) {
          case ts_telemetrie:
              telnetclient.print("M365 Telemetrie Screen"); 
            break;
          case ts_esc_raw:
              telnetclient.print("M365 ESC RAW Screen"); 
            break;
          case ts_ble_raw:
              telnetclient.print("M365 BLE RAW Screen"); 
            break;
          case ts_bms_raw:
              telnetclient.print("M365 BMS RAW Screen"); 
            break;
          case ts_x1_raw:
              telnetclient.print("M365 X1 RAW Screen"); 
            break;
          case ts_esc_array:
              telnetclient.print("M365 ESC Array Screen"); 
            break;
          case ts_ble_array:
              telnetclient.print("M365 BLE ARRAY Screen"); 
            break;
          case ts_bms_array:
              telnetclient.print("M365 BMS ARRAY Screen"); 
            break;
          case ts_x1_array:
              telnetclient.print("M365 X1 ARRAY Screen"); 
            break;
          case ts_statistics:
              telnetclient.print("M365 Statistics Screen");
            break;
        }
        tmpi++;
        if (tmpi % 2) {
          telnetclient.println(" .\r\n");
        } else {
          telnetclient.println("  \r\n");
        }
        switch (telnetscreen) {
          case ts_telemetrie:
                telnetclient.printf("\r\nBLE\r\n Throttle: %03d Brake %03d\r\n",bleparsed->throttle,bleparsed->brake);
                sprintf(tmp1,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c",bmsparsed->serial[0],bmsparsed->serial[1],bmsparsed->serial[2],bmsparsed->serial[3],bmsparsed->serial[4],bmsparsed->serial[5],bmsparsed->serial[6],bmsparsed->serial[7],bmsparsed->serial[8],bmsparsed->serial[9],bmsparsed->serial[10],bmsparsed->serial[11],bmsparsed->serial[12],bmsparsed->serial[13]);
                telnetclient.printf("\r\n\r\nBMS Serial: %s Version: %x.%x.%x\r\n", tmp1,bmsparsed->fwversion[1],(bmsparsed->fwversion[0]&0xf0)>>4,bmsparsed->fwversion[0]&0x0f);
                telnetclient.printf(" Capacity Total: %5d mAh Remaining %5d mAh Percent %03d%%\r\n",bmsparsed->totalcapacity, bmsparsed->remainingcapacity, bmsparsed->remainingpercent);
                telnetclient.printf(" Temperature1 %3d°C Temperature2 %3d°C Health %05d\r\n",bmsparsed->temperature[1]-20, bmsparsed->temperature[0]-20, bmsparsed->health);
                telnetclient.printf(" Production Date: %d-%d-%d ",((bmsparsed->proddate)&0xFE00)>>9,((bmsparsed->proddate)&0x1E0)>>5,(bmsparsed->proddate)&0x1f);
                telnetclient.printf(" Charging Cycles: %d Charging Times: %d\r\n",bmsparsed->cycles,bmsparsed->chargingtimes);
                telnetclient.printf(" Voltage: %2.2f V Current %05d mA\r\n",(float)bmsparsed->voltage/100.0f, bmsparsed->current);
                telnetclient.printf(" C1: %1.3f C2: %1.3f C3: %1.3f C4: %1.3f C5: %1.3f C6: %1.3f C7: %1.3f C8: %1.3f C9: %1.3f C10: %1.3f\r\n",(float)bmsparsed->Cell1Voltage/1000.0f,(float)bmsparsed->Cell2Voltage/1000.0f,(float)bmsparsed->Cell3Voltage/1000.0f,(float)bmsparsed->Cell4Voltage/1000.0f,(float)bmsparsed->Cell5Voltage/1000.0f,(float)bmsparsed->Cell6Voltage/1000.0f,(float)bmsparsed->Cell7Voltage/1000.0f,(float)bmsparsed->Cell8Voltage/1000.0f,(float)bmsparsed->Cell9Voltage/1000.0f,(float)bmsparsed->Cell10Voltage/1000.0f);
                sprintf(tmp1,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c",escparsed->serial[0],escparsed->serial[1],escparsed->serial[2],escparsed->serial[3],escparsed->serial[4],escparsed->serial[5],escparsed->serial[6],escparsed->serial[7],escparsed->serial[8],escparsed->serial[9],escparsed->serial[10],escparsed->serial[11],escparsed->serial[12],escparsed->serial[13]);
                telnetclient.printf("\r\n\r\nESC Serial: %s Version: %x.%x.%x\r\n", tmp1,escparsed->fwversion[1],(escparsed->fwversion[0]&0xf0)>>4,escparsed->fwversion[0]&0x0f);
                sprintf(tmp1,"%c%c%c%c%c%c",escparsed->pin[0],escparsed->pin[1],escparsed->pin[2],escparsed->pin[3],escparsed->pin[4],escparsed->pin[5]);
                telnetclient.printf(" Pin: %s Error %05d\r\n",tmp1,escparsed->error);
                telnetclient.printf(" Distance Total: %.2f km Trip: %.2f km Remaining %.2f km\r\n", (float)escparsed->totaldistance/1000.0f,(float)escparsed->tripdistance/100.0f,(float)escparsed->remainingdistance/100.0f);
                telnetclient.printf(" Power On Time1: %d s, Power On Time1: %d s, Trip Time: %d s\r\n",escparsed->ontime1,escparsed->ontime2,escparsed->triptime);
                telnetclient.printf(" FrameTemp1: %05d FrameTemp2: %.1f °C\r\n", (float)escparsed->frametemp1/10.0f, (float)escparsed->frametemp2/10.0f);
                telnetclient.printf(" Speed: %.2f km/h Avg: %.2f km/h\r\n", (float)escparsed->speed/1000.0f, (float)escparsed->averagespeed/1000.0f);
                telnetclient.printf(" Batt Percent: %3d%%\r\n",escparsed->battpercent);
                telnetclient.printf(" Ecomode: %05d Kers: %05d Cruisemode: %05d Taillight: %05d\r\n", escparsed->ecomode, escparsed->kers, escparsed->cruisemode, escparsed->taillight);
                telnetclient.printf("\r\n\r\nX1 Mode %03d LEDs %03d Light ",x1parsed->mode, x1parsed->battleds);
                if (x1parsed->light==0) { telnetclient.print("OFF "); } else {
                  if (x1parsed->light==100) { telnetclient.print("ON "); } else {
                   telnetclient.print("    ");
                  }
                }
                telnetclient.printf("%03d BeepAction %03d\r\n", x1parsed->light, x1parsed->beepaction);
              telnetnextrefreshtimestamp=millis()+telnetrefreshanyscreen;
            break;
          case ts_statistics:
              telnetclient.printf("Requests Sent:\r\n  ESC: %05d   BMS: %05d   Total: %05d\r\n", requests_sent_esc, requests_sent_bms, requests_sent_bms+requests_sent_esc);
              telnetclient.printf("Packets Received:\r\n  ESC: %05d   BMS: %05d   BLE: %05d   X1: %05d   unhandled: %05d\r\n", packets_rec_esc,packets_rec_bms,packets_rec_ble,packets_rec_x1,packets_rec_unhandled);
              telnetclient.printf("  CRC OK: %05d   CRC FAIL: %05d\r\n   Total: %05d\r\n",packets_crcok,packets_crcfail,packets_rec);
              telnetclient.printf("\r\nTimers:\r\n  Main:   %04.3f ms\r\n  Telnet: %04.3f ms\r\n",(float)duration_mainloop/1000.0f, (float)duration_telnet/1000.0f);
              telnetclient.printf("  OLED Main: %04.3f ms\r\n",(float)duration_oled/1000.0f);
              telnetclient.printf("  OLED1 Draw: %04.3f ms\r\n  OLED1 I2C:  %04.3f ms\r\n",(float)duration_oled1draw/1000.0f,(float)duration_oled1i2c/1000.0f);
              telnetclient.printf("  OLED2 Draw: %04.3f ms\r\n  OLED2 I2C:  %04.3f ms\r\n",(float)duration_oled2draw/1000.0f,(float)duration_oled2i2c/1000.0f);
              telnetclient.printf("  Request Cycle Loop: %04d ms\r\n  Request last Index: %03d\r\n", duration_requestcycle,requestindex);
              telnetclient.printf("  Time since last Packet: %05d ms\r\n",m365packetlasttimestamp);
              telnetclient.printf("\r\nPackets per device Address:\r\n");
              k = 0;
              for(uint16_t j = 0; j <=255; j++) {
                if (packetsperaddress[j]>=1) {
                  k++;
                  telnetclient.printf("%02X -> %05d  ", j, packetsperaddress[j]);
                  if ((k % 5)==0) { telnetclient.println(""); }
                }
              }
              telnetnextrefreshtimestamp=millis()+telnetrefreshanyscreen;           
            break;
          case ts_esc_raw:
              telnetclient.println("     00 01 02 03   04 05 06 07   08 09 0A 0B   0C 0D 0E 0F   10 11 12 13   14 15 16 17   18 19 1A 1B   1C 1D 1E 1F");
              for(uint16_t j = 0; j<=511; j++) {
                  if ((j % 32)==0) { 
                    telnetclient.printf("\r\n%03X: ",j); 
                  } else {
                    if ((j % 4)==0) { 
                      telnetclient.print("- "); 
                    }
                  } //mod32
                  telnetclient.printf("%02X ", escdata[j]);
              }
              telnetnextrefreshtimestamp=millis()+telnetrefreshrawarrayscreen;
            break;
          case ts_ble_raw:
              telnetclient.println("     00 01 02 03   04 05 06 07   08 09 0A 0B   0C 0D 0E 0F   10 11 12 13   14 15 16 17   18 19 1A 1B   1C 1D 1E 1F");
              for(uint16_t j = 0; j<=511; j++) {
                  if ((j % 32)==0) { 
                    telnetclient.printf("\r\n%03X: ",j); 
                  } else {
                    if ((j % 4)==0) { 
                      telnetclient.print("- "); 
                    }
                  } //mod32
                  telnetclient.printf("%02X ", bledata[j]);
              }
              telnetnextrefreshtimestamp=millis()+telnetrefreshrawarrayscreen;
            break;
          case ts_bms_raw:
              telnetclient.println("     00 01 02 03   04 05 06 07   08 09 0A 0B   0C 0D 0E 0F   10 11 12 13   14 15 16 17   18 19 1A 1B   1C 1D 1E 1F");
              for(uint16_t j = 0; j<=511; j++) {
                  if ((j % 32)==0) { 
                    telnetclient.printf("\r\n%03X: ",j); 
                  } else {
                    if ((j % 4)==0) { 
                      telnetclient.print("- "); 
                    }
                  } //mod32
                  telnetclient.printf("%02X ", bmsdata[j]);
              }
              telnetnextrefreshtimestamp=millis()+telnetrefreshrawarrayscreen;
            break;
          case ts_x1_raw:
              telnetclient.println("     00 01 02 03   04 05 06 07   08 09 0A 0B   0C 0D 0E 0F   10 11 12 13   14 15 16 17   18 19 1A 1B   1C 1D 1E 1F");
              for(uint16_t j = 0; j<=511; j++) {
                  if ((j % 32)==0) { 
                    telnetclient.printf("\r\n%03X: ",j); 
                  } else {
                    if ((j % 4)==0) { 
                      telnetclient.print("- "); 
                    }
                  } //mod32
                  telnetclient.printf("%02X ", x1data[j]);
              }
              telnetnextrefreshtimestamp=millis()+telnetrefreshrawarrayscreen;
            break;
          case ts_esc_array:
          case ts_ble_array:
          case ts_bms_array:
          case ts_x1_array:
              telnetclient.print("empty screen - not implemented");
              telnetnextrefreshtimestamp=millis()+telnetrefreshanyscreen;
              
            break;
        }
        yield();
    }
  //} //i 0 to maxclients
  //DebugSerial.printf("---TELNET--- %d\r\n",millis());
}

void handle_telnet() {
  timestamp_telnetstart=micros();
  boolean hc = false;
   switch(telnetstate) {
    case telnetoff:
      break;
    case telnetturnon:
        telnetserver.begin();
        telnetserver.setNoDelay(true);
        DebugSerial.print("### Telnet Debug Server @ ");
        if (wlanstate==wlanap) {
          DebugSerial.print(WiFi.softAPIP());  
        } else {
          DebugSerial.print(WiFi.localIP());  
        }
        DebugSerial.println(":36523");
#ifdef userawserver
        rawserver.begin();
        rawserver.setNoDelay(true);
        DebugSerial.print("### Telnet RAW Server @ ");
        if (wlanstate==wlanap) {
          DebugSerial.print(WiFi.softAPIP());  
        } else {
          DebugSerial.print(WiFi.localIP());  
        }
        DebugSerial.println(":36524");
#endif        
#ifdef usepacketserver
        packetserver.begin();
        packetserver.setNoDelay(true);
        DebugSerial.print("### Telnet PACKET Server @ ");
        if (wlanstate==wlanap) {
          DebugSerial.print(WiFi.softAPIP());  
        } else {
          DebugSerial.print(WiFi.localIP());  
        }
        DebugSerial.println(":36525");
#endif        

        

        telnetstate = telnetlistening;
        userconnecttimestamp = millis()+userconnecttimeout;  
      break;
    case telnetlistening: 
            if (telnetserver.hasClient()) {
              //for(i = 0; i < MAX_SRV_CLIENTS; i++){
                //find free/disconnected spot
                if (!telnetclient || !telnetclient.connected()){
                  if(telnetclient) telnetclient.stop();
                  telnetclient = telnetserver.available();
                  if (!telnetclient) DebugSerial.println("available broken");
                  DebugSerial.print("### Telnet New client: ");
                  DebugSerial.print(i); DebugSerial.print(' ');
                  DebugSerial.println(telnetclient.remoteIP());
                  telnetstate = telnetclientconnected;
                  #ifdef usestatusled
                    ledontime=250; ledofftime=0; ledcurrenttime = millis();
                  #endif
                  telnetnextrefreshtimestamp=millis()+telnetrefreshanyscreen;
                  break;
                }
            //}
            /*if (i >= MAX_SRV_CLIENTS) {
              //no free/disconnected spot so reject
              telnetserver.available().stop();
              DebugSerial.println("### Telnet rejected connection (max Clients)");
            }*/
          }
#ifdef userawserver
          if (rawserver.hasClient()) {
            //for(i = 0; i < MAX_SRV_CLIENTS; i++){
              //find free/disconnected spot
              if (!rawclient || !rawclient.connected()){
                if(rawclient) rawclient.stop();
                rawclient = rawserver.available();
                if (!rawclient) DebugSerial.println("available broken");
                DebugSerial.print("### Telnet RAW New client: ");
                DebugSerial.println(rawclient.remoteIP());
                telnetstate = telnetclientconnected;
                #ifdef usestatusled
                  ledontime=250; ledofftime=0; ledcurrenttime = millis();
                #endif
                //telnetnextrefreshtimestamp=millis()+telnetrefreshanyscreen;
                //break;
              }
            //}
          }
#endif
#ifdef usepacketserver
          if (packetserver.hasClient()) {
            //for(i = 0; i < MAX_SRV_CLIENTS; i++){
              //find free/disconnected spot
              if (!packetclient || !packetclient.connected()){
                if(packetclient) packetclient.stop();
                packetclient = packetserver.available();
                if (!packetclient) DebugSerial.println("available broken");
                DebugSerial.print("### Telnet PACKET New client: ");
                DebugSerial.println(packetclient.remoteIP());
                telnetstate = telnetclientconnected;
                #ifdef usestatusled
                  ledontime=250; ledofftime=0; ledcurrenttime = millis();
                #endif
                //telnetnextrefreshtimestamp=millis()+telnetrefreshanyscreen;
                //break;
              }
            //}
          }
#endif
          if (userconnecttimestamp<millis()) {
            telnetstate = telnetturnoff;
          }
      break;
    case telnetclientconnected: 
        //TODO check if clients are still connected... else start client connect timer and fall back to listening mode
        hc = false;
        //for(i = 0; i < MAX_SRV_CLIENTS; i++){
          if (telnetclient && telnetclient.connected()) { hc=true; }
        //} //i 0 to maxclients
#ifdef userawserver
        if (rawclient && rawclient.connected()) { hc=true; }
#endif
#ifdef usepacketserver
        if (packetclient && packetclient.connected()) { hc=true; }
#endif

        if (!hc) {
          //no more clients, restart listening timer....
          telnetstate = telnetturnon;
          DebugSerial.println("### Telnet lost all clients - restarting telnet");
          #ifdef usestatusled
            ledontime=500; ledofftime=500; ledcurrenttime = millis();
          #endif
        } else {
          //still has clients, update telnet stuff
            if (telnetnextrefreshtimestamp<millis()) {
                  telnet_refreshscreen();
            } //telnetrefreshtimer

        } //else !hc
        //check clients for data
        //for(i = 0; i < MAX_SRV_CLIENTS; i++){
          if (telnetclient && telnetclient.connected()){
            if(telnetclient.available()){
              //get data from the telnet client and push it to the UART
              uint8_t tcmd = telnetclient.read();
              DebugSerial.printf("Telnet Command: %02X\r\n",tcmd);
              switch (tcmd) {
                case 0x73: telnetscreen=ts_statistics; break; //s
                case 0x74: telnetscreen=ts_telemetrie; break; //t
                case 0x65: telnetscreen=ts_esc_raw; break; //e
                case 0x62: telnetscreen=ts_bms_raw; break; //b
                case 0x6E: telnetscreen=ts_ble_raw; break; //n
                case 0x78: telnetscreen=ts_x1_raw; break; //x
                case 0x45: telnetscreen=ts_esc_array; break; //E
                case 0x42: telnetscreen=ts_bms_array; break; //B
                case 0x4E: telnetscreen=ts_ble_array; break; //N
                case 0x58: telnetscreen=ts_x1_array; break;  //X
                case 0x72: reset_statistics(); break;  //r
              } //switch
              //while(serverClients[i].available()) M365DebugSerial.write(serverClients[i].read());
            } //serverclients available
          } //if connected
        //}  //for i
#ifdef userawserver        
        //TODO: Check RAW Client for Data and send to bus? immediately send or queue and send in next free timeslot?
#endif        
      break;
    case telnetdisconnectclients:
        if (telnetclient) telnetclient.stop();
        #ifdef userawserver
          if (rawclient) rawclient.stop();
        #endif        
        #ifdef userawserver
          if (packetclient) packetclient.stop();
        #endif
        telnetstate = telnetturnoff;
      break;
    case telnetturnoff:
        #ifdef ESP32
          telnetserver.end();
        #endif
        #if defined userawserver && defined ESP32
          rawserver.end();
        #endif      
        #if defined usepacketserver && defined ESP32
          packetserver.end();
        #endif
        wlanstate=wlanturnoff; 
        telnetstate = telnetoff;
      break;
   } //switch (telnetstate)
   duration_telnet = micros()-timestamp_telnetstart;
} //handle_telnet
#endif //usetelnetserver

#ifdef ESP32
void WiFiEvent(WiFiEvent_t event) {
    //DebugSerial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
      case SYSTEM_EVENT_WIFI_READY: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_WIFI_READY"); break; // < ESP32 WiFi ready
      case SYSTEM_EVENT_SCAN_DONE: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_SCAN_DONE"); break; // < ESP32 finish scanning AP
      case SYSTEM_EVENT_STA_START: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_STA_START"); break; // < ESP32 station start
      case SYSTEM_EVENT_STA_STOP: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_STA_STOP"); break; // < ESP32 station stop
      case SYSTEM_EVENT_STA_CONNECTED: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_STA_CONNECTED"); break; // < ESP32 station connected to AP
      case SYSTEM_EVENT_STA_DISCONNECTED: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_STA_DISCONNECTED"); break; // < ESP32 station disconnected from AP
      case SYSTEM_EVENT_STA_AUTHMODE_CHANGE: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_STA_AUTHMODE_CHANGE"); break; // < the auth mode of AP connected by ESP32 station changed
      case SYSTEM_EVENT_STA_GOT_IP: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_STA_GOT_IP"); break; // < ESP32 station got IP from connected AP
      case SYSTEM_EVENT_STA_LOST_IP: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_STA_LOST_IP"); break; // < ESP32 station lost IP and the IP is reset to 0
      case SYSTEM_EVENT_STA_WPS_ER_SUCCESS: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_STA_WPS_ER_SUCCESS"); break; // < ESP32 station wps succeeds in enrollee mode
      case SYSTEM_EVENT_STA_WPS_ER_FAILED: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_STA_WPS_ER_FAILED"); break; // < ESP32 station wps fails in enrollee mode
      case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_STA_WPS_ER_TIMEOUT"); break; // < ESP32 station wps timeout in enrollee mode
      case SYSTEM_EVENT_STA_WPS_ER_PIN: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_STA_WPS_ER_PIN"); break; // < ESP32 station wps pin code in enrollee mode
      case SYSTEM_EVENT_AP_START: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_AP_START"); break; // < ESP32 soft-AP start
      case SYSTEM_EVENT_AP_STOP: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_AP_STOP"); break; // < ESP32 soft-AP stop
      case SYSTEM_EVENT_AP_STACONNECTED: apnumclientsconnected++; DebugSerial.println("### WifiEvent: SYSTEM_EVENT_AP_STACONNECTED"); break; // < a station connected to ESP32 soft-AP
      case SYSTEM_EVENT_AP_STADISCONNECTED: apnumclientsconnected--; DebugSerial.println("### WifiEvent: SYSTEM_EVENT_AP_STADISCONNECTED"); break; // < a station disconnected from ESP32 soft-AP
      case SYSTEM_EVENT_AP_PROBEREQRECVED: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_AP_PROBEREQRECVED"); break; // < Receive probe request packet in soft-AP interface
      case SYSTEM_EVENT_GOT_IP6: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_GOT_IP6"); break; // < ESP32 station or ap or ethernet interface v6IP addr is preferred
      case SYSTEM_EVENT_ETH_START: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_ETH_START"); break; // < ESP32 ethernet start
      case SYSTEM_EVENT_ETH_STOP: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_ETH_STOP"); break; // < ESP32 ethernet stop
      case SYSTEM_EVENT_ETH_CONNECTED: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_ETH_CONNECTED"); break; // < ESP32 ethernet phy link up
      case SYSTEM_EVENT_ETH_DISCONNECTED: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_ETH_DISCONNECTED"); break; // < ESP32 ethernet phy link down
      case SYSTEM_EVENT_ETH_GOT_IP: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_ETH_GOT_IP"); break; // < ESP32 ethernet got IP from connected AP
      case SYSTEM_EVENT_MAX: DebugSerial.println("### WifiEvent: SYSTEM_EVENT_MAX"); break; //
    }
} //WiFiEvent
#endif

/*#ifdef ESP8266
void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  DebugSerial.println("### WifiEvent: Station connected");
  apnumclientsconnected++;
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
  DebugSerial.println("### WifiEvent: Station disconnected");
  apnumclientsconnected--;
}
#endif
*/

void handle_wlan() {
  if (wlanstate!=wlanstateold) {
    wlanstateold=wlanstate;
    newdata = true; //update display
    #ifdef debug_dump_states
      Serial.printf("### WLANSTATE %d -> %d\r\n",wlanstateold,wlanstate);
      print_states();
    #endif
  }

  switch(wlanstate) {
    case wlanoff: 
      break;
    case wlanturnapon:
        WiFi.disconnect(true);
        delay(500);
        WiFi.mode(WIFI_OFF);
        yield();
        WiFi.mode(WIFI_AP);
        yield();
        apnumclientsconnected=0;
        apnumclientsconnectedlast=0;
        #ifdef ESP32
          WiFi.onEvent(WiFiEvent);
        #endif
        #ifdef ESP8266
          stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
          stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);
        #endif
        WiFi.softAP(apssid, appassword);
        yield();
        DebugSerial.printf("### WLAN AP\r\nSSID: %s, AP IP address: ", apssid);  DebugSerial.println( WiFi.softAPIP());
#ifdef useoledxxx //POPUP
        display1.clearDisplay();
        display1.setFont();
        display1.println("AP Mode");
        display1.print( WiFi.softAPIP());
        display1.display();
#endif        
#ifdef usetelnetserver
        if (telnetstate==telnetoff) {
          telnetstate = telnetturnon;
        }
#endif        
        wlanconnecttimestamp = millis()+wlanapconnecttimeout;            
        wlanstate = wlanap;
        ArduinoOTA.begin();
        //start OTA
      break;
    case wlanturnstaon:
        DebugSerial.println("### Searching for known wlan");
        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        #ifdef staticip
          if (!WiFi.config(ip, gateway, subnet, dns)) {
            DebugSerial.println("### STA Failed to configure");
          }
        #endif
        currentssidindex = 0;
        #ifdef ESP32
          WiFi.onEvent(WiFiEvent); //not really needed in STA mode
          WiFi.setHostname("M365OLEDESP32");
        #endif
        #ifdef ESP8266
          WiFi.hostname("M365OLEDESP8266");
        #endif

        WiFi.begin(ssid1, password1);
        wlanconnecttimestamp = millis()+wlanconnecttimeout;
#ifdef useoledxxx //POPUP
        display1.clearDisplay();
        display1.setFont();
        display1.printf("W %d",currentssidindex);
        display1.display();  
#endif        
        DebugSerial.printf("###  WLan searching for ssidindex %d\r\n",currentssidindex);
        wlanstate = wlansearching;
        #ifdef usestatusled
          ledontime=50; ledofftime=450; ledcurrenttime = millis();
        #endif
        //start wlan connect timeout
      break;
    case wlansearching:
        if (WiFi.status() != WL_CONNECTED) {
          if (wlanconnecttimestamp<millis()) {
            //timeout, test next wlan
            currentssidindex++;
            if (currentssidindex<maxssids) {
              if (currentssidindex==1) { WiFi.begin(ssid2, password2); }
              if (currentssidindex==2) { WiFi.begin(ssid3, password3); }
              wlanconnecttimestamp = millis()+wlanconnecttimeout;
#ifdef useoledxxx //POPUP
              display1.clearDisplay();
              display1.setFont();
              display1.printf("W %d",currentssidindex);
              display1.display();
#endif              
              DebugSerial.printf("### WLan searching for ssidindex %d\r\n",currentssidindex);
            } else {
              DebugSerial.println("### WLAN search failed, starting Access Point");
              wlanstate = wlanturnapon;
            }
          }
        } else {
          wlanstate = wlanconnected;
          DebugSerial.print("### WLAN Connected to ");
          DebugSerial.print(WiFi.SSID());
          DebugSerial.print(", IP is ");
          DebugSerial.println(WiFi.localIP());
          #ifdef useoledxxx //POPUP
            display1.clearDisplay();
            display1.setFont();
            display1.print("WC ");
            display1.println(WiFi.SSID());
            display1.print(WiFi.localIP());
            display1.display();
          #endif          
          #ifdef usetelnetserver
            if (telnetstate==telnetoff) {
                  telnetstate = telnetturnon;
              }
          #endif            
          #ifdef usestatusled
            ledontime=500; ledofftime=500; ledcurrenttime = millis();
          #endif
          ArduinoOTA.begin();
        }
      break;
    case wlanconnected:
          if (WiFi.status() != WL_CONNECTED) {
              DebugSerial.println("### WiFi lost connection!");
#ifdef useoledxxx //POPUP
              display1.clearDisplay();
              display1.setFont();
              display1.println("W CONN LOST");
              display1.print(WiFi.SSID());
              display1.display();
#endif
#ifdef usetelnetserver
              if (telnetstate!=telnetoff) {
                  telnetstate = telnetdisconnectclients;
              }
#endif
             wlanstate = wlanturnoff;
          }
      break;
    case wlanap: //make 2 states - waiting for clients & has clients
        //without wifi events: apnumclientsconnected=WiFi.softAPgetStationNum();
        if (apnumclientsconnected!=apnumclientsconnectedlast) {
          //num of connect clients changed
          apnumclientsconnectedlast = apnumclientsconnected;
          DebugSerial.printf("### AP Clients connected changed: %d -> %d\r\n",apnumclientsconnectedlast, apnumclientsconnected);
#ifdef useoledxxx //POPUP
          display1.clearDisplay();
          display1.setFont();
          display1.printf("AP Clients %d",apnumclientsconnected);
          display1.display();
#endif          
          if (apnumclientsconnected==0) {
              //no one connnected, but there was someone connected.... restart timeout
              wlanconnecttimestamp = millis()+wlanapconnecttimeout;
          } //if (apnumclientsconnected==0) 
        } //if (apnumclientsconnected!=apnumclientsconnectedlast)
        yield();
        if (apnumclientsconnected==0) {
          if (wlanconnecttimestamp<millis()) {
            wlanstate=wlanturnoff; 
          } //if (wlanconnecttimestamp<millis())
        } //if (apnumclientsconnected==0)
      break;
    case wlanturnoff:
        #ifdef usemqtt
          if (client.connected) {
            client.disconnect();
          }        
        #endif
        #ifdef usetelnetserver
          if (telnetclient) telnetclient.stop();
        #endif
        #ifdef userawserver
          if (rawclient) rawclient.stop();
        #endif
        #ifdef usepacketserver
          if (packetclient) packetclient.stop();
        #endif
        #ifdef ESP32
          ArduinoOTA.end();
        #endif
        WiFi.softAPdisconnect();
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        DebugSerial.println("### WIFI OFF");
        #ifdef useoledxxx //POPUP
                display1.clearDisplay();
                display1.setFont();
                display1.print("WLAN OFF");
                display1.display();
        #endif
        wlanstate=wlanoff; 
        #ifdef usestatusled
          ledontime=0; ledofftime=500; ledcurrenttime = millis();
        #endif
        //STOP OTA    
      break;
  } //switch wlanstate
} //handle_wlan

#ifdef usestatusled
void handle_led() {
  if (ledontime==0) { 
    digitalWrite(led,LOW); 
  } else {
    if (ledofftime==0) { 
      digitalWrite(led,HIGH); 
    } else {
      if (millis()>ledcurrenttime) {
        if (digitalRead(led)) {
          digitalWrite(led,LOW);
          ledcurrenttime = millis()+ledontime;
        } else {
          digitalWrite(led,HIGH);
          ledcurrenttime = millis()+ledofftime;
        } //else digitalread
      } //if millis
    } //else ledoff
  } //else ledon
}//handle_led
#endif

#ifdef debug_dump_states //dump state machine states to Serial Port on change
  void print_states() {
    #ifdef usetelnetserver    
      if (telnetstate!=telnetstateold) {
        DebugSerial.printf("### TELNETSTATE %d -> %d\r\n",telnetstateold,telnetstate);
        telnetstateold=telnetstate;
      }
    #endif    
    if (m365receiverstate!=m365receiverstateold) {
      DebugSerial.printf("### M365RecState: %d -> %d\r\n",m365receiverstateold,m365receiverstate);
      m365receiverstateold=m365receiverstate;
    }
    if (m365packetstate!=m365packetstateold) {
      DebugSerial.printf("M365PacketState: %d -> %d\r\n",m365packetstateold,m365packetstate);
      m365packetstateold=m365packetstate;
    }    
  } //print_states
#endif

void oled_startscreen() {
  display1.setFont();
  display1.setTextSize(1);
  display1.setTextColor(WHITE);
  display1.clearDisplay();
  display1.setCursor(35,5);
  display1.print("ESP32 OLED");
  //display1.setCursor(64,20);
  //display1.print("for");
  display1.setCursor(35,25);
  display1.print("Xiaomi Mijia365");
  display1.setCursor(80,55);
  display1.print(swversion);
  display1.drawBitmap(0,0,  scooter, 64,64, 1);
}

void oled_switchscreens() {
  uint8_t oldscreen = screen;
  
  //Data/Bus Timeout
    if ((m365packettimestamp+m365packettimeout)<millis() & screen!=screen_timeout) {
      screen=screen_timeout;
      updatescreen=true;
      return;
    }
    if ((screen==screen_timeout) & ((m365packettimestamp+m365packettimeout)>millis())) {
      if (escparsed->speed>0) {
        screen=screen_drive;  
      } else {
        screen=screen_stop;  
      }
      updatescreen=true; 
    }
  

  //charging screens: 
    if (newdata & (escparsed->speed==0) & (bmsparsed->current<0) & (screen!=screen_charging)) { 
      screen=screen_charging; 
      //timeout_oled=millis()+oledchargestarttimeout;
      updatescreen=true;
    }
    if ((screen==screen_charging) & (abs((float)escparsed->speed/1000.0f)>2.0f)) { 
      if (abs((float)escparsed->speed/1000.0f)>0.9f) {
        screen=screen_drive;  
      } else {
        screen=screen_stop;  
      }
      updatescreen=true;
    }

  //switch between driving/stop screens:
    //maybe add a speed treshold like 0.3km/h to keep showing "stop" screen
    //if (newdata & (screen==screen_drive) & (escparsed->speed==0)) {
    if (newdata & (screen==screen_drive) & (abs((float)escparsed->speed/1000.0f)<0.5f)) {
    //if (newdata & ((x1parsed->mode==0)|(x1parsed->mode==2))) {
      screen=screen_stop;
      updatescreen=true;
    }
    //if (newdata & (screen==screen_stop) & (escparsed->speed>0)) {
    if (newdata & (screen==screen_stop) & (abs((float)escparsed->speed/1000.0f)>0.9f)) {
      
    //if (newdata & ((x1parsed->mode==1)|(x1parsed->mode==3))) {
      screen=screen_drive;
      updatescreen=true;
    }

  //switching of subscreens via throttle while we are in STOP mode
  if (newdata & (screen==screen_stop)) {
    uint8_t oldsubscreen = subscreen;
    if (bleparsed->throttle>=gasmin) {
      subscreen = ((bleparsed->throttle-gasmin) / stopwindowsize)+1;
      windowsubpos = (uint8_t)((float)((bleparsed->throttle-gasmin) % stopwindowsize)*(float)oledwidth/(float)stopwindowsize);
    } else {
      subscreen=0;
      windowsubpos=0;
    }
    if ((subscreen)>subscreens) { subscreen=subscreens; }
    if (subscreen!=oldsubscreen) { updatescreen = true; }
  }
  #if !defined useoled2
  //switching of subscreens via throttle while we are in CHARGE mode, only needed with one screen
  if (newdata & (screen==screen_charging)) {
    uint8_t oldsubscreen = subscreen;
    if (bleparsed->throttle>=gasmin) {
      subscreen = ((bleparsed->throttle-gasmin) / chargewindowsize)+1;
      windowsubpos = (uint8_t)((float)((bleparsed->throttle-gasmin) % chargewindowsize)*(float)oledwidth/(float)chargewindowsize);
    } else {
      subscreen=1;
      windowsubpos=0;
    }
    if ((subscreen)>subscreens) { subscreen=subscreens; }
    if (subscreen!=oldsubscreen) { updatescreen = true; }
  }
  #endif

  //update with new data, but honor refresh-rate
    if (newdata & (olednextrefreshtimestamp<millis())) { 
    //if ((olednextrefreshtimestamp<millis())) { 
      updatescreen=true;
      newdata=false; 
    }

  //update subscriptions if screen has been changed
    if (oldscreen!=screen) { 
      //DebugSerial.printf("---screen: %d\r\n",screen);
      subscribedrequests=rqsarray[screen];
    }
  //reset newdata flag if we consumed it
    if (updatescreen) { newdata=false; }
} //oled_switchscreens

#ifdef useoled1
  void oled1_update() {
    uint8_t line;
    lowest=10000;
    highest=0;
    timestamp_oled1draw=micros();
    display1.clearDisplay();
    
    if (screen==screen_drive) {
      #ifdef useoled2
        //speed  
          display1.setCursor(0,fontbigbaselinezero-5);
          display1.setFont(&fontbig);
          display1.printf("%04.1f", abs((float)escparsed->speed/1000.0f));
          display1.setFont();
          display1.setCursor(84,28);
          display1.println("km/h");
        //distance
          display1.setFont(&FreeSans18pt7b);
          display1.setCursor(0,63);
          //display1.printf("%6d", bmsparsed->remainingpercent);
          display1.printf("%04.1f",(float)escparsed->tripdistance/100.0f);
          display1.setFont();
          display1.setCursor(68,line8); 
          display1.print("km");
        /*//batt percent  
          display1.setFont(&FreeSans9pt7b); 
          display1.setCursor(86,dataoffset+baselineoffset*4+linespace*3-2); 
          display1.printf("%3d%%",bmsparsed->remainingpercent);
          */
          //remaining distance  
          display1.setFont(&FreeSans9pt7b); 
          display1.setCursor(84,63);
          display1.printf("%03.1f",(float)escparsed->remainingdistance/100.0f);
          display1.setFont();
          display1.setCursor(116,line8);
          display1.println("km");
      #else          
        //Speed
          display1.setCursor(0,fontbigbaselinezero-5);
          display1.setFont(&fontbig);
          display1.printf("%04.1f", abs((float)escparsed->speed/1000.0f));
          display1.setFont();
          display1.setCursor(84,28);
          display1.print("km/h");
        //Watt
          display1.setFont(&FreeSans18pt7b);
          display1.setCursor(0,63);
          display1.printf("%04.0f",((float)(bmsparsed->voltage/100.0f)*(float)bmsparsed->current/100.0f));
          display1.setFont();
          display1.setCursor(78,line8); 
          display1.print("W");
        //batt percent  
          display1.setFont(&FreeSans9pt7b); 
          //display1.setFont(&FreeSans18pt7b);
          //display1.setCursor(86,dataoffset+baselineoffset*4+linespace*3-2); 
          display1.setCursor(90,63);
          display1.printf("%02d",bmsparsed->remainingpercent);
          display1.setFont();
          display1.setCursor(122,line8); 
          display1.print("%");
      #endif
    }
    if (screen==screen_stop) {
          display1.setFont();
          display1.setCursor(0,0);
          //display1.drawFastHLine(0,8,windowsubpos,WHITE);
          switch (subscreen) {
            case 0: //Trip Info: Average Speed, Distance, Time, Average Speed 
                display1.print("TRIP Info");
                //display1.drawFastHLine(0,8,128,WHITE);
                display1.setFont(&FreeSans9pt7b); display1.setCursor(0,dataoffset+baselineoffset); display1.print("Avg:");
                display1.setFont(&FreeSansBold9pt7b); display1.setCursor(50,dataoffset+baselineoffset); display1.printf("%05.2f",(float)escparsed->averagespeed/1000.0f);
                display1.setCursor(100,dataoffset+baselineoffset); display1.setFont(); display1.print("km/h");
                display1.setFont(&FreeSans9pt7b); display1.setCursor(0,dataoffset+baselineoffset*2+linespace); display1.print("Dist:");
                display1.setFont(&FreeSansBold9pt7b); display1.setCursor(50,dataoffset+baselineoffset*2+linespace); display1.printf("%05.2f",(float)escparsed->tripdistance/100.0f);
                display1.setCursor(100,dataoffset+baselineoffset*2+linespace); display1.setFont(); display1.print("km");
                display1.setFont(&FreeSans9pt7b); display1.setCursor(0,dataoffset+baselineoffset*3+linespace*2); display1.print("Time:");
                //display1.setFont(&FreeSansBold9pt7b); display1.setCursor(50,dataoffset+baselineoffset*3+linespace*2); display1.printf("%05d",escparsed->triptime);
                display1.setFont(&FreeSansBold9pt7b); display1.setCursor(50,dataoffset+baselineoffset*3+linespace*2); display1.printf("%02d:%02d",escparsed->triptime/60,escparsed->triptime % 60);
                display1.setCursor(100,dataoffset+baselineoffset*3+linespace*2); display1.setFont(); display1.print("s");
                display1.setFont(&FreeSans9pt7b); display1.setCursor(0,dataoffset+baselineoffset*4+linespace*3); display1.print("Rem:");
                display1.setFont(&FreeSansBold9pt7b); display1.setCursor(50,dataoffset+baselineoffset*4+linespace*3); display1.printf("%05.2f",(float)escparsed->remainingdistance/100.0f);
                display1.setCursor(100,dataoffset+baselineoffset*4+linespace*3); display1.setFont(); display1.print("km");
              break;
            #if !defined useoled2
              case 1: //Single Screen  Temperatures - Frame temp 1 & 2, Batt Temp 1 & 2
//FIX THIS SCREEN -> UNITS & Alignment
                  display1.printf("TEMPERATURE     (%d/%d)",subscreen,subscreens);
                line = dataoffset+baselineoffset;
                  display1.setFont(&FreeSans9pt7b); display1.setCursor(0,line); display1.print("Batt1:");
                  display1.setFont(&FreeSansBold9pt7b); display1.setCursor(60,line); display1.printf("%4.1f",(float)bmsparsed->temperature[0]-20.0f);  
                  display1.setCursor(100,line); display1.setFont(); display1.print(" C"); display1.drawCircle(103,line-5,1,WHITE);
                line = dataoffset+baselineoffset*2+linespace;
                  display1.setFont(&FreeSans9pt7b); display1.setCursor(0,line); display1.print("Batt2:");
                  display1.setFont(&FreeSansBold9pt7b); display1.setCursor(60,line); display1.printf("%4.1f",(float)bmsparsed->temperature[1]-20.0f);  
                  display1.setCursor(100,line); display1.setFont(); display1.print(" C"); display1.drawCircle(103,line-5,1,WHITE);
                line = dataoffset+baselineoffset*3+linespace*2;
                  display1.setFont(&FreeSans9pt7b); display1.setCursor(0,line); display1.print("ESC:");
                  display1.setFont(&FreeSansBold9pt7b); display1.setCursor(60,line); display1.printf("%4.1f",(float)escparsed->frametemp2/10.0f);
                  display1.setCursor(100,line); display1.setFont(); display1.print(" C"); display1.drawCircle(103,line-5,1,WHITE);
                break;
              case 2: //Single Screen: Min/Max Values
            #else
              case 1: //Dual Screen: Min/Max Values
            #endif
                display1.printf("TRIP Min/Max    (%d/%d)\r\n\r\n",subscreen,subscreens);
                display1.printf("Speed : %5.1f %5.1f\r\n",(float)speed_min/1000.0f,(float)speed_max/1000.0f);
                display1.printf("Ampere: %5.1f %5.1f A\r\n",(float)current_min/100.0f,(float)current_max/100.0f);
                display1.printf("Watt  : %5d %5d W\r\n",watt_min,watt_max);
                display1.printf("BattT1: %3d.0 %3d.0 C\r\n",tb1_min-20,tb1_max-20); display1.drawCircle(117,42,1,WHITE);
                display1.printf("BattT2: %3d.0 %3d.0 C\r\n",tb2_min-20,tb2_max-20); display1.drawCircle(117,50,1,WHITE);
                display1.printf("ESC T : %5.1f %5.1f C\r\n",(float)te_min/10.0f,(float)te_max/10.0f); display1.drawCircle(117,58,1,WHITE);
              break;
            #if !defined useoled2
              case 3: //Single Screen Batt Info 1: Total Capacity, Remaining, Voltage, Percent
                display1.printf("BATTERY 1       (%d/%d)",subscreen,subscreens);
            #else
              case 2: //Dual Screen : Batt Info
                display1.printf("BATTERY STATUS  (%d/%d)",subscreen,subscreens);
            #endif
                line = dataoffset+baselineoffset;
                  display1.setFont(&FreeSans9pt7b); display1.setCursor(0,line); display1.print("Volt:");
                  display1.setFont(&FreeSansBold9pt7b); display1.setCursor(64,line); display1.printf("%5.2f",(float)bmsparsed->voltage/100.0f);
                  display1.setCursor(119,line); display1.setFont(); display1.print("V");
                line = dataoffset+baselineoffset*2+linespace;
                  display1.setFont(&FreeSans9pt7b); display1.setCursor(0,line); display1.print("Percent:");
                  display1.setFont(&FreeSansBold9pt7b); display1.setCursor(64,line); display1.printf("%4d%",bmsparsed->remainingpercent);
                  display1.setCursor(119,line); display1.setFont(); display1.print("%");
                line = dataoffset+baselineoffset*3+linespace*2;
                  display1.setFont(&FreeSans9pt7b); display1.setCursor(0,line); display1.print("Cycles:");
                  display1.setFont(&FreeSansBold9pt7b); display1.setCursor(64,line); display1.printf("%4d",bmsparsed->cycles);
                  display1.setCursor(119,line); display1.setFont(); display1.print("#");
                line = dataoffset+baselineoffset*4+linespace*3;
                  display1.setFont(&FreeSans9pt7b); display1.setCursor(0,line); display1.print("Charges:");
                  display1.setFont(&FreeSansBold9pt7b); display1.setCursor(64,line); display1.printf("%4d",bmsparsed->chargingtimes);
                  display1.setCursor(119,line); display1.setFont(); display1.print("#");
              break;
            #if !defined useoled2  
              case 4: //Single Screen Batt Info 2: Health/Cycles/Charge Num/Prod Date
                  display1.printf("BATTERY 2       (%d/%d)",subscreen,subscreens);
                  line = dataoffset+baselineoffset;
                    display1.setFont(&FreeSans9pt7b); display1.setCursor(0,line); display1.print("Health:");
                    display1.setFont(&FreeSansBold9pt7b); display1.setCursor(64,line); display1.printf("%4d",bmsparsed->health);
                    display1.setCursor(119,line); display1.setFont(); display1.print("%");
                  line = dataoffset+baselineoffset*2+linespace;
                    display1.setFont(&FreeSans9pt7b); display1.setCursor(0,line); display1.print("Cap:");
                    display1.setFont(&FreeSansBold9pt7b); display1.setCursor(56,line); display1.printf("%5d",bmsparsed->remainingcapacity);
                    display1.setCursor(108,line); display1.setFont(); display1.print("mAh");
                  line = dataoffset+baselineoffset*3+linespace*2;
                    display1.setFont(&FreeSans9pt7b); display1.setCursor(0,line); display1.print("Total:");
                    display1.setFont(&FreeSansBold9pt7b); display1.setCursor(56,line); display1.printf("%5d",bmsparsed->totalcapacity);
                    display1.setCursor(108,line); display1.setFont(); display1.print("mAh");
                  line = dataoffset+baselineoffset*4+linespace*3;
                    display1.setFont(&FreeSans9pt7b); display1.setCursor(0,line); display1.print("Temp:");
                    display1.setFont(&FreeSansBold9pt7b); display1.setCursor(64,line); display1.printf("%02d/%02d",bmsparsed->temperature[0]-20,bmsparsed->temperature[1]-20);
                    display1.setCursor(119,line); display1.setFont(); display1.print("C");
                    display1.drawCircle(115,line-5,1,WHITE);
                   break;
              case 5: //Single Screen Cell Voltages
            #else
              case 3: //dual screen Cell Voltages
            #endif
                display1.printf("CELL VOLTAGES   (%d/%d)\r\n",subscreen,subscreens);
                #if !defined useoled2
                  display1.printf("01: %5.3f ",(float)bmsparsed->Cell1Voltage/1000.0f);
                  display1.printf("02: %5.3f  ",(float)bmsparsed->Cell2Voltage/1000.0f);
                  display1.printf("03: %5.3f ",(float)bmsparsed->Cell3Voltage/1000.0f);
                  display1.printf("04: %5.3f  ",(float)bmsparsed->Cell4Voltage/1000.0f);
                  display1.printf("05: %5.3f ",(float)bmsparsed->Cell5Voltage/1000.0f);
                  display1.printf("06: %5.3f  ",(float)bmsparsed->Cell6Voltage/1000.0f);
                  display1.printf("07: %5.3f ",(float)bmsparsed->Cell7Voltage/1000.0f);
                  display1.printf("08: %5.3f  ",(float)bmsparsed->Cell8Voltage/1000.0f);
                  display1.printf("10: %5.3f  ",(float)bmsparsed->Cell10Voltage/1000.0f);
                  display1.printf("09: %5.3f ",(float)bmsparsed->Cell9Voltage/1000.0f);
                  #ifdef batt12s
                    display1.printf("11: %5.3f ",(float)bmsparsed->Cell11Voltage/1000.0f);
                    display1.printf("12: %5.3f  ",(float)bmsparsed->Cell12Voltage/1000.0f);
                  #endif
                  display1.printf("Max. Diff: %5.3f", (float)(highest-lowest)/1000.0f);
                #else
                  display1.setFont(&FreeSans9pt7b); 
                  display1.setCursor(0,21);
                  display1.printf("1:%5.3f ",(float)bmsparsed->Cell1Voltage/1000.0f);
                  display1.printf("2:%5.3f",(float)bmsparsed->Cell2Voltage/1000.0f);
                  display1.setCursor(0,35);
                  display1.printf("3:%5.3f ",(float)bmsparsed->Cell3Voltage/1000.0f);
                  display1.printf("4:%5.3f",(float)bmsparsed->Cell4Voltage/1000.0f);
                  display1.setCursor(0,49);
                  display1.printf("5:%5.3f ",(float)bmsparsed->Cell5Voltage/1000.0f);
                  display1.printf("6:%5.3f",(float)bmsparsed->Cell6Voltage/1000.0f);
                  display1.setCursor(0,63);
                  display1.printf("7:%5.3f ",(float)bmsparsed->Cell7Voltage/1000.0f);
                  display1.printf("8:%5.3f",(float)bmsparsed->Cell8Voltage/1000.0f);
                #endif  
              break;
            #if !defined useoled2
              case 6: //Single Screen Serials/Versions/Name
            #else
              case 4: //dual screen Serials/Versions/Name
            #endif
                display1.printf("Assets          (%d/%d)\r\n",subscreen,subscreens);
                //display1.setFont(&FreeSans9pt7b); display1.setCursor(0,dataoffset+baselineoffset); display1.print("BMS SN:");
                sprintf(tmp1,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c",bmsparsed->serial[0],bmsparsed->serial[1],bmsparsed->serial[2],bmsparsed->serial[3],bmsparsed->serial[4],bmsparsed->serial[5],bmsparsed->serial[6],bmsparsed->serial[7],bmsparsed->serial[8],bmsparsed->serial[9],bmsparsed->serial[10],bmsparsed->serial[11],bmsparsed->serial[12],bmsparsed->serial[13]);
                display1.printf("BMS   FW: %x.%x.%x\r\nSN: %s\r\n", bmsparsed->fwversion[1],(bmsparsed->fwversion[0]&0xf0)>>4,bmsparsed->fwversion[0]&0x0f,tmp1);
                //display1.setFont(&FreeSansBold9pt7b); display1.setCursor(64,dataoffset+baselineoffset); display1.printf("%s",tmp1);
                //display1.setFont(&FreeSans9pt7b); display1.setCursor(0,dataoffset+baselineoffset*2+linespace); 
                //display1.print("BMS FW:   ");
                //display1.setFont(&FreeSansBold9pt7b); display1.setCursor(64,dataoffset+baselineoffset*2+linespace); 
                //display1.printf("%x.%x.%x\r\n",bmsparsed->fwversion[1],(bmsparsed->fwversion[0]&0xf0)>>4,bmsparsed->fwversion[0]&0x0f);
                //display1.setFont(&FreeSans9pt7b); display1.setCursor(0,dataoffset+baselineoffset*3+linespace*2); 
                sprintf(tmp1,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c",escparsed->serial[0],escparsed->serial[1],escparsed->serial[2],escparsed->serial[3],escparsed->serial[4],escparsed->serial[5],escparsed->serial[6],escparsed->serial[7],escparsed->serial[8],escparsed->serial[9],escparsed->serial[10],escparsed->serial[11],escparsed->serial[12],escparsed->serial[13]);
                display1.printf("ESC   FW: %x.%x.%x\r\nSN: %s\r\n", escparsed->fwversion[1],(escparsed->fwversion[0]&0xf0)>>4,escparsed->fwversion[0]&0x0f,tmp1);
                //display1.setFont(&FreeSansBold9pt7b); display1.setCursor(64,dataoffset+baselineoffset*3+linespace*2); display1.printf("%s",tmp1);
                //display1.setFont(&FreeSans9pt7b); display1.setCursor(0,dataoffset+baselineoffset*4+linespace*3); 
                //display1.print("ESC FW:   ");
                //display1.setFont(&FreeSansBold9pt7b); display1.setCursor(64,dataoffset+baselineoffset*4+linespace*3); 
                //display1.printf("%x.%x.%x\r\n",escparsed->fwversion[1],(escparsed->fwversion[0]&0xf0)>>4,escparsed->fwversion[0]&0x0f);
                //display1.setFont(&FreeSans9pt7b); display1.setCursor(0,dataoffset+baselineoffset*4+linespace*3); 
                sprintf(tmp1,"%c%c%c%c%c%c",escparsed->pin[0],escparsed->pin[1],escparsed->pin[2],escparsed->pin[3],escparsed->pin[4],escparsed->pin[5]);
                display1.printf("Pin: %s\r\n", tmp1);
                display1.printf("Miles: %.2f km\r\n",(float)escparsed->totaldistance/1000.0f);
                display1.printf("Batt-Date: 20%02d-%02d-%02d",((bmsparsed->proddate)&0xFE00)>>9,((bmsparsed->proddate)&0x1E0)>>5,(bmsparsed->proddate)&0x1f);
                //display1.setFont(&FreeSansBold9pt7b); display1.setCursor(64,dataoffset+baselineoffset*3+linespace*2); 
                //display1.printf("%s",tmp1);
              break;
            #if !defined useoled2  
              case 7: //single screen ESP Status
                  display1.printf("ESP32 State     (%d/%d)\r\n\r\n",subscreen,subscreens);
                  display1.print("Firmware: "); display1.println(swversion);
                  display1.print("WLAN: ");
                  if (wlanstate==wlansearching) { display1.println("searching..."); }
                  if (wlanstate==wlanconnected) { display1.print("Connected\r\nSSID: "); display1.println(WiFi.SSID()); display1.println(WiFi.localIP()); }
                  if (wlanstate==wlanap) { display1.print("AP Mode\r\nSSID: "); display1.println(WiFi.softAPIP()); display1.println( WiFi.softAPIP());}
                  if (wlanstate==wlanoff) { display1.println("OFF"); }
                  display1.print("BT: OFF");
                break;
              case 8: //single screen config / menu
            #else
              case 5: //dual screen config/menu
            #endif
                display1.printf("CONFIG          (%d/%d)",subscreen, subscreens);
                display1.setFont(&FreeSans9pt7b); display1.setCursor(0,dataoffset+baselineoffset); display1.print("KERS:");
                display1.setFont(&FreeSansBold9pt7b); display1.setCursor(80,dataoffset+baselineoffset); display1.print("Weak");
                display1.setFont(&FreeSans9pt7b); display1.setCursor(0,dataoffset+baselineoffset*2+linespace+1); display1.print("Tail Light:");
                display1.setFont(&FreeSansBold9pt7b); display1.setCursor(80,dataoffset+baselineoffset*2+linespace+1); display1.print("  Off");
                display1.setFont(&FreeSans9pt7b); display1.setCursor(0,dataoffset+baselineoffset*3+(linespace+1)*2); display1.print("Cruise:");
                display1.setFont(&FreeSansBold9pt7b); display1.setCursor(80,dataoffset+baselineoffset*3+(linespace+1)*2); display1.print("   On");
                //display1.setFont(&FreeSans9pt7b); display1.setCursor(0,dataoffset+baselineoffset*4+linespace*3); display1.print("Volt:");
                //display1.setFont(&FreeSansBold9pt7b); display1.setCursor(64,dataoffset+baselineoffset*4+linespace*3); display1.print("42.65V");
              break;
          }
    }
    if (screen==screen_charging) {
          display1.clearDisplay();
          display1.setFont();
          display1.setCursor(0,0);
          display1.drawFastHLine(0,8,windowsubpos,WHITE);
          #if !defined useoled2
          switch (subscreen) {
            case 1: //Trip Info: Average Speed, Distance, Time, Average Speed 
                display1.printf("                (%d/2)",subscreen);
          #else
                //display1.print("Charging");
          #endif
            display1.setTextSize(1);
            display1.setTextColor(WHITE);
            display1.setFont(&FreeSans9pt7b); 
            display1.setCursor(10,baselineoffset);
            //display1.setCursor(40,10);
            //display1.setFont();
            display1.printf("Charging %02%%", bmsparsed->remainingpercent);
            //display1.printf("%02u%%", bmsparsed->remainingpercent);
            display1.drawRect(14,25,100,10,WHITE);
            display1.fillRect(14,26,bmsparsed->remainingpercent,8,WHITE);
            display1.setFont();
            display1.setCursor(14,45);
            display1.printf("%4.1f V   %4.1f A\r\n", (float)bmsparsed->voltage/100.0f,abs((float)bmsparsed->current/100.0f));
            display1.setCursor(14,56);
            display1.printf("%4.0f W  %5d mAh",((float)(bmsparsed->voltage/100.0f)*(float)bmsparsed->current/100.0f),bmsparsed->remainingcapacity);
          #if !defined useoled2
              break;
            case 2: //Cell Infos
                display1.printf("Charging        (%d/2)",subscreen);
                display1.printf("01: %5.3f ",(float)bmsparsed->Cell1Voltage/1000.0f);
                display1.printf("02: %5.3f  ",(float)bmsparsed->Cell2Voltage/1000.0f);
                display1.printf("03: %5.3f ",(float)bmsparsed->Cell3Voltage/1000.0f);
                display1.printf("04: %5.3f  ",(float)bmsparsed->Cell4Voltage/1000.0f);
                display1.printf("05: %5.3f ",(float)bmsparsed->Cell5Voltage/1000.0f);
                display1.printf("06: %5.3f  ",(float)bmsparsed->Cell6Voltage/1000.0f);
                display1.printf("07: %5.3f ",(float)bmsparsed->Cell7Voltage/1000.0f);
                display1.printf("08: %5.3f  ",(float)bmsparsed->Cell8Voltage/1000.0f);
                display1.printf("09: %5.3f ",(float)bmsparsed->Cell9Voltage/1000.0f);
                display1.printf("10: %5.3f  ",(float)bmsparsed->Cell10Voltage/1000.0f);
                #ifdef batt12s
                 display1.printf("11: %5.3f ",(float)bmsparsed->Cell11Voltage/1000.0f);
                 display1.printf("12: %5.3f  ",(float)bmsparsed->Cell12Voltage/1000.0f);
                #endif
                //display1.printf("Tot: %5.2fV D: %5.3fL: %5.3f H: %5.3f", (float)bmsparsed->voltage/100.0f,(float)(highest-lowest)/1000.0f,(float)(lowest)/1000.0f,(float)(highest)/1000.0f);
                #if !defined batt12s
                  display1.printf("L:  %5.3f H:  %5.3f\r\n", (float)(lowest)/1000.0f,(float)(highest)/1000.0f);
                #endif
                display1.printf("T:  %5.2f D:  %5.3f", (float)bmsparsed->voltage/100.0f,(float)(highest-lowest)/1000.0f);
              break;
          }
          #endif
    }
    //if (screen==screen_timeout) {
    if (screen==screen_error) {
      //https://mimod.ru/en_US/m365errorcodes/
        #if !defined(useoled2)
          display1.setFont(&fontsmall);
          display1.setCursor(30,20);
          display1.print("ERROR");
          display1.setCursor(50,40);
          display1.printf("%02d",escparsed->error);
          display1.setFont();
          display1.setCursor(0,56);
          switch (escparsed->error) {
            case 10: display1.print("BLE Communication"); break;
            case 11:
            case 12:
            case 13:
            case 28:
            case 29: display1.print("Shunt or FET"); break;
            case 14:
            case 15: display1.print("THROTTLE/BRAKE"); break;
            case 18: display1.print("MOTOR HALL SENSOR"); break;
            case 21: display1.print("BMS Communication"); break;
            case 22:
            case 23: display1.print("BMS Serialnumber"); break;
            case 24: display1.print("VOLTAGE WRONG"); break;
            case 27:
            case 39: display1.print("ESC Serial/Activation"); break;
            case 35:
            case 36: display1.print("BATT TEMP ALERT"); break;
            case 40: display1.print("ESC TEMP ALERT"); break;
            default: display1.print("unknown code^"); break;
          }
        #else
          display1.setFont(&fontsmall);
          display1.setCursor(50,20);
          display1.print("ERROR");
          display1.setCursor(70,45);
          display1.printf("%02d",escparsed->error);
          display1.drawBitmap(0,0,  scooter, 64,64, 1);
        #endif
    }
    if (screen==screen_timeout) {
      #ifdef useoled2
        oled_startscreen();

      #else
        display1.drawBitmap(0,0,  scooter, 64,64, 1);
        display1.setFont(&fontsmall);
        display1.setCursor(74,15);
        display1.print("NO");
        display1.setCursor(64,35);
        display1.print("DATA");
        display1.setFont();
        if (wlanstate==wlansearching) { display1.setCursor(64,42); display1.print("WLAN"); display1.setCursor(64,55); display1.print("searching"); }
        if (wlanstate==wlanconnected) { display1.setCursor(64,42); display1.print(WiFi.SSID()); display1.setCursor(34,57); display1.print(WiFi.localIP()); }
        if (wlanstate==wlanap) { display1.setCursor(64,42); display1.print(WiFi.softAPIP()); display1.setCursor(34,57); display1.print( WiFi.softAPIP()); }
        if (wlanstate==wlanoff) { display1.setCursor(64,56); display1.print("WLAN OFF"); }
      #endif
    }

    //Status ICONS (text for now) on right edge in drive/stop screen - LIGHT On/Off, Ecomode On/Off
      //if ((screen==screen_drive) | ((screen==screen_stop)&(subscreen==0))) {
        if (screen==screen_drive) {
      //LIGHT ON/OFF
        if (x1parsed->light==0x64) { 
          display1.setFont();
          display1.setCursor(122,line1);
          display1.print("L");
        }
      //NORMAL/ECO MODE
        display1.setFont();
        display1.setCursor(122,line2);
        if (x1parsed->mode<2) { 
          display1.print("N"); //normal mode
        } else {
          display1.print("E"); //eco mode
        }
      //WLAN STATUS
        display1.setFont();
        display1.setCursor(116,line3);
        if (wlanstate==wlansearching) { display1.print("WS"); }
        if (wlanstate==wlanconnected) { display1.print("WC"); }
        if (wlanstate==wlanap) { display1.print("WA"); }
      //Bluetooth Status
      //  display1.setFont();
      //  display1.setCursor(116,line4);
      //  if (blestate==blesearching) { display1.print("BS"); }
      //  if (blestate==bleconnected) { display1.print("BC"); }
      }

      //if wlan on
        //if telnet connected TC
        //if Wlanclient   WC
        //if wlanap       AP
    /*if (oled_blink) {
      display1.drawPixel(0,0, WHITE);
    } else {
      display1.drawPixel(0,0, BLACK);
    }*/
    duration_oled1draw = micros()-timestamp_oled1draw;
    timestamp_oled1i2c=micros();
    display1.display();
    duration_oled1i2c = micros()-timestamp_oled1i2c;
    #ifdef useoled2
      updatescreen2 = true;
    #endif
  } //oled1_update
#endif

#ifdef useoled2
  void oled2_update() {
    uint8_t line;
    timestamp_oled2draw=micros();
    display2.clearDisplay();
    if (screen==screen_drive) {
        //Watt
          display2.setCursor(0,fontbigbaselinezero-5);
          display2.setFont(&fontbig);
          display2.printf("%04.0f", ((float)(bmsparsed->voltage/100.0f)*(float)bmsparsed->current/100.0f));
          display2.setFont();
          display2.setCursor(104,28);
          display2.print("W");
        //Ampere
          display2.setFont(&FreeSans18pt7b);
          display2.setCursor(0,63);
          display2.printf("%02.0f",(float)bmsparsed->current/100.0f);
          display2.setFont();
          display2.setCursor(40,line8); 
          display2.print("A");
        //Volt
          display2.setFont(&FreeSans18pt7b);
          display2.setCursor(63,63);
          display2.printf("%02.0f",(float)bmsparsed->voltage/100.0f);
          display2.setFont();
          display2.setCursor(104,line8); 
          display2.print("V");
        //Batt Graph 
          display2.drawRect(117,0,8,4,WHITE);
          display2.fillRect(115,3,12,61,WHITE);
          //display2.fillRect(118,1+(uint8_t)((100.0f-(float)bmsparsed->remainingpercent)*0.62f),8,(uint8_t)((float)bmsparsed->remainingpercent*0.62f),WHITE);
          display2.fillRect(116,5,10,(uint8_t)((100.0f-(float)bmsparsed->remainingpercent)*0.59f),BLACK);
          display1.drawFastHLine(116,18,10,BLACK);
          display1.drawFastHLine(116,19,10,BLACK);
          display1.drawFastHLine(116,32,10,BLACK);
          display1.drawFastHLine(116,33,10,BLACK);
          display1.drawFastHLine(116,47,10,BLACK);
          display1.drawFastHLine(116,48,10,BLACK);
    }
    if (screen==screen_stop) {
          display2.setFont();
          display2.setCursor(0,0);
          switch (subscreen) {
            case 0: //Temperatures - we have 2 Battery Temperatures, but we only display the higher one - both can be seen on detail screens
                display2.setFont(&FreeSans9pt7b); display2.setCursor(0,baselineoffset); display2.print("Batt:");
                display2.setFont(&FreeSansBold9pt7b); display2.setCursor(60,baselineoffset); 
                if ((float)bmsparsed->temperature[0]>=(float)bmsparsed->temperature[1]) {
                  display2.printf("%4.1f",(float)bmsparsed->temperature[0]-20.0f);  
                } else {
                  display2.printf("%4.1f",(float)bmsparsed->temperature[1]-20.0f);  
                }
                display2.setCursor(100,baselineoffset); display2.setFont(); display2.print(" C"); display2.drawCircle(103,baselineoffset-5,1,WHITE);
                display2.setFont(&FreeSans9pt7b); display2.setCursor(0,baselineoffset*2+linespace); display2.print("ESC:");
                display2.setFont(&FreeSansBold9pt7b); display2.setCursor(60,baselineoffset*2+linespace); display2.printf("%4.1f",(float)escparsed->frametemp2/10.0f);
                display2.setCursor(100,baselineoffset*2+linespace); display2.setFont(); display2.print(" C"); display2.drawCircle(103,baselineoffset*2+linespace-5,1,WHITE);
                /*
                display2.setFont(&FreeSans9pt7b); display2.setCursor(0,baselineoffset*3+linespace*2); display2.print("Motor:");
                display2.setFont(&FreeSansBold9pt7b); display2.setCursor(60,baselineoffset*3+linespace*2); display2.print("00.0");
                display2.setCursor(100,baselineoffset*3+linespace*2); display2.setFont(); display2.print(" C");
                */
                display2.setFont(&FreeSans9pt7b); display2.setCursor(0,baselineoffset*3+linespace*2); display2.print("Volt:");
                display2.setFont(&FreeSansBold9pt7b); display2.setCursor(60,baselineoffset*3+linespace*2); display2.printf("%4.1f",(float)bmsparsed->voltage/100.0f);
                display2.setCursor(100,baselineoffset*3+linespace*2); display2.setFont(); display2.print(" V");

                display2.setFont(&FreeSans9pt7b); display2.setCursor(0,baselineoffset*4+linespace*3); display2.print("Batt:");
                display2.setFont(&FreeSansBold9pt7b); display2.setCursor(60,baselineoffset*4+linespace*3); display2.printf("%5d%",bmsparsed->remainingpercent);
                display2.setCursor(100,baselineoffset*4+linespace*3); display2.setFont(); display2.print(" %");

                
              break;
            case 1:
                display2.printf("FIX THIS SCREEN (%d/%d)\r\n\r\n",subscreen,subscreens);
                /*
                display2.printf("Speed: %04.1f/%04.1f\r\n",(float)speed_min/1000.0f,(float)speed_max/1000.0f);
                display2.printf("Current: %4.1f/%4.1fA\r\n",(float)current_min/100.0f,(float)current_max/100.0f);
                display2.printf("Watt: %4d/%4d\r\n",watt_min,watt_max);
                display2.printf("BattTmp1: %3d/%3d°C\r\n",tb1_min-20,tb1_max-20);
                display2.printf("BattTmp2: %3d/%3d°C\r\n",tb2_min-20,tb2_max-20);
                display2.printf("ESC Temp: %4.1f/%4.1f°C\r\n",(float)te_min/10.0f,(float)te_max/10.0f);
                */
              break;
            case 2:
                  line = baselineoffset;
                    display2.setFont(&FreeSans9pt7b); display2.setCursor(0,line); display2.print("Health:");
                    display2.setFont(&FreeSansBold9pt7b); display2.setCursor(64,line); display2.printf("%4d",bmsparsed->health);
                    display2.setCursor(119,line); display2.setFont(); display2.print("%");
                  line = baselineoffset*2+linespace;
                    display2.setFont(&FreeSans9pt7b); display2.setCursor(0,line); display2.print("Cap:");
                    display2.setFont(&FreeSansBold9pt7b); display2.setCursor(56,line); display2.printf("%5d",bmsparsed->remainingcapacity);
                    display2.setCursor(108,line); display2.setFont(); display2.print("mAh");
                  line = baselineoffset*3+linespace*2;
                    display2.setFont(&FreeSans9pt7b); display2.setCursor(0,line); display2.print("Total:");
                    display2.setFont(&FreeSansBold9pt7b); display2.setCursor(56,line); display2.printf("%5d",bmsparsed->totalcapacity);
                    display2.setCursor(108,line); display2.setFont(); display2.print("mAh");
                  line = baselineoffset*4+linespace*3;
                    display2.setFont(&FreeSans9pt7b); display2.setCursor(0,line); display2.print("Temp:");
                    display2.setFont(&FreeSansBold9pt7b); display2.setCursor(64,line); display2.printf("%02d/%02d",bmsparsed->temperature[0]-20,bmsparsed->temperature[1]-20);
                    display2.setCursor(119,line); display2.setFont(); display2.print("C");
                    display2.drawCircle(115,line-5,1,WHITE);
              break;
            case 3:
                display2.setFont(&FreeSans9pt7b); 
                display2.setCursor(0,12);
                display2.printf("9:%5.3f ",(float)bmsparsed->Cell9Voltage/1000.0f);
                display2.printf("X:%5.3f",(float)bmsparsed->Cell10Voltage/1000.0f);
                #ifdef batt12s
                  display2.setCursor(0,12+14);
                  display2.printf("1:%5.3f ",(float)bmsparsed->Cell11Voltage/1000.0f);
                  display2.printf("2:%5.3f",(float)bmsparsed->Cell12Voltage/1000.0f);
                #endif
                display2.setCursor(0,12+14+14);
                display2.printf("L:%5.3f H:%5.3f", (float)(lowest)/1000.0f,(float)(highest)/1000.0f);
                display2.setCursor(0,12+14+14+14);
                display2.printf("T:%5.2f D:%5.3f", (float)bmsparsed->voltage/100.0f,(float)(highest-lowest)/1000.0f);
              break;
            case 4:
                display2.print("ESP32 Status\r\nFirmware: "); display2.println(swversion);
                display2.print("WLAN: ");
                if (wlanstate==wlansearching) { display2.println("searching..."); }
                if (wlanstate==wlanconnected) { display2.print("Connected\r\nSSID: "); display2.println(WiFi.SSID()); display2.println(WiFi.localIP()); }
                if (wlanstate==wlanap) { display2.print("AP Mode\r\nSSID: "); display2.println(WiFi.softAPIP()); display2.println( WiFi.softAPIP()); }
                if (wlanstate==wlanoff) { display2.println("OFF"); }
                display2.print("BT: OFF");  
              break;
            case 5:
                display2.printf("CONFIG          (%d/%d)\r\n",subscreen,subscreens);
              break;
            default:
                display2.setFont();
                display2.setCursor(0,0);
                display2.print("STOP");
              break;
          }

    }
    if (screen==screen_charging) {
                display2.setFont();
                display2.setCursor(0,0);
                if (bmsparsed->Cell1Voltage>0) { lowest=_min(lowest,bmsparsed->Cell1Voltage); highest=_max(highest,bmsparsed->Cell1Voltage); }
                if (bmsparsed->Cell2Voltage>0) { lowest=_min(lowest,bmsparsed->Cell2Voltage); highest=_max(highest,bmsparsed->Cell2Voltage); }
                if (bmsparsed->Cell3Voltage>0) { lowest=_min(lowest,bmsparsed->Cell3Voltage); highest=_max(highest,bmsparsed->Cell3Voltage); }
                if (bmsparsed->Cell4Voltage>0) { lowest=_min(lowest,bmsparsed->Cell4Voltage); highest=_max(highest,bmsparsed->Cell4Voltage); }
                if (bmsparsed->Cell5Voltage>0) { lowest=_min(lowest,bmsparsed->Cell5Voltage); highest=_max(highest,bmsparsed->Cell5Voltage); }
                if (bmsparsed->Cell6Voltage>0) { lowest=_min(lowest,bmsparsed->Cell6Voltage); highest=_max(highest,bmsparsed->Cell6Voltage); }
                if (bmsparsed->Cell7Voltage>0) { lowest=_min(lowest,bmsparsed->Cell7Voltage); highest=_max(highest,bmsparsed->Cell7Voltage); }
                if (bmsparsed->Cell8Voltage>0) { lowest=_min(lowest,bmsparsed->Cell8Voltage); highest=_max(highest,bmsparsed->Cell8Voltage); }
                if (bmsparsed->Cell9Voltage>0) { lowest=_min(lowest,bmsparsed->Cell9Voltage); highest=_max(highest,bmsparsed->Cell9Voltage); }
                if (bmsparsed->Cell10Voltage>0) { lowest=_min(lowest,bmsparsed->Cell10Voltage); highest=_max(highest,bmsparsed->Cell10Voltage); }
                #ifdef batt12s
                  if (bmsparsed->Cell11Voltage>0) { lowest=_min(lowest,bmsparsed->Cell11Voltage); highest=_max(highest,bmsparsed->Cell11Voltage); }
                  if (bmsparsed->Cell12Voltage>0) { lowest=_min(lowest,bmsparsed->Cell12Voltage); highest=_max(highest,bmsparsed->Cell12Voltage); }
                #endif
                display2.printf("01: %5.3f ",(float)bmsparsed->Cell1Voltage/1000.0f);
                display2.printf("02: %5.3f  ",(float)bmsparsed->Cell2Voltage/1000.0f);
                display2.printf("03: %5.3f ",(float)bmsparsed->Cell3Voltage/1000.0f);
                display2.printf("04: %5.3f  ",(float)bmsparsed->Cell4Voltage/1000.0f);
                display2.printf("05: %5.3f ",(float)bmsparsed->Cell5Voltage/1000.0f);
                display2.printf("06: %5.3f  ",(float)bmsparsed->Cell6Voltage/1000.0f);
                display2.printf("07: %5.3f ",(float)bmsparsed->Cell7Voltage/1000.0f);
                display2.printf("08: %5.3f  ",(float)bmsparsed->Cell8Voltage/1000.0f);
                display2.printf("09: %5.3f ",(float)bmsparsed->Cell9Voltage/1000.0f);
                display2.printf("10: %5.3f  ",(float)bmsparsed->Cell10Voltage/1000.0f);
                #ifdef batt12s
                  display2.printf("11: %5.3f ",(float)bmsparsed->Cell11Voltage/1000.0f);
                  display2.printf("12: %5.3f  ",(float)bmsparsed->Cell12Voltage/1000.0f);
                #endif
                //display2.printf("Tot: %5.2fV D: %5.3fL: %5.3f H: %5.3f", (float)bmsparsed->voltage/100.0f,(float)(highest-lowest)/1000.0f,(float)(lowest)/1000.0f,(float)(highest)/1000.0f);
                display2.printf("L:  %5.3f H:  %5.3f\r\n", (float)(lowest)/1000.0f,(float)(highest)/1000.0f);
                display2.printf("T:  %5.2f D:  %5.3f", (float)bmsparsed->voltage/100.0f,(float)(highest-lowest)/1000.0f);
    }
    if (screen==screen_error) {
      display2.setFont(&fontsmall);
      display2.setCursor(0,20);
      switch (escparsed->error) {
        case 10: display2.print("BLE Communication"); break;
        case 11: display2.print("Mot Phase A Current"); break;
        case 12: display2.print("Mot Phase B Current"); break;
        case 13: display2.print("Mot Phase C Current"); break;
        case 14: display2.print("THROTTLE SENSOR"); break;
        case 15: display2.print("BRAKE SENSOR"); break;
        case 18: display2.print("MOTOR HALL SENSOR"); break;
        case 21: display2.print("BMS Communication"); break;
        case 22: display2.print("Bad BMS Password"); break;
        case 23: display2.print("BMS Serialnumber"); break;
        case 24: display2.print("VOLTAGE WRONG"); break;
        case 26: display2.print("EEPROM/FLASH CRC"); break;
        case 27: display2.print("Bad ESC Password"); break;
        case 28: display2.print("hS FET Error"); break;
        case 29: display2.print("lS FET Error"); break;
        case 31: display2.print("Program Error"); break;
        case 35: display2.print("ESC Serial"); break;
        case 36: display2.print("ESC Activation"); break;
        case 39: display2.print("BATT TEMP ALERT"); break;
        case 40: display2.print("ESC TEMP ALERT"); break;
        default: display2.print("unknown code^"); break;
      }
    }
    if (screen==screen_timeout) {  
      display2.setFont(&fontsmall);
      display2.setCursor(40,20);
      display2.print("DATA");
      display2.setCursor(20,40);
      display2.print("TIMEOUT");
      display2.setFont();
      if (wlanstate==wlansearching) { display2.setCursor(0,48); display2.print("WLAN searching..."); }
      if (wlanstate==wlanconnected) { display2.setCursor(0,48); display2.print("SSID: "); display2.print(WiFi.SSID()); display2.setCursor(0,57); display2.print("IP: "); display2.print(WiFi.localIP()); }
      if (wlanstate==wlanap) { display2.setCursor(0,48); display2.print("SSID: ");  display2.print(apssid); display2.setCursor(0,57); display2.print("IP: "); display2.print( WiFi.softAPIP()); }
      if (wlanstate==wlanoff) { display2.setCursor(40,57); display2.print("WLAN OFF"); }

    }

    duration_oled2draw = micros()-timestamp_oled2draw;
    timestamp_oled2i2c=micros();
    display2.display();
    duration_oled2i2c = micros()-timestamp_oled2i2c;
  } //oled2_update
#endif

#ifdef useoled1
void handle_oled() {
  timestamp_oledstart=micros();
  oled_switchscreens();
  //TEST TWEAK
  //screen=screen_charging;
  //screen=screen_timeout;
  //screen=screen_error; escparsed->error=12;
  //screen=screen_stop; subscreen=4;


  #ifdef useoled2 //workaround for slow i2c & 2 displays to avoid too long blocking of main code
    if (updatescreen2) {
      oled2_update();
      updatescreen2=false;
      //DebugSerial.printf("---OLED2----- %02d %d\r\n",screen, millis());
    }
  #endif
  if (updatescreen) {
    olednextrefreshtimestamp=millis()+oledrefreshanyscreen;
    oled1_update();
    //oled2_update();
    updatescreen=false;
    //DebugSerial.printf("---OLED1----- %02d %d\r\n",screen, millis());
  }
  duration_oled = micros()-timestamp_oledstart;
  oled_blink!=oled_blink;
}
#endif

void setup() {
  start_m365();
#ifdef useoled1
  #ifdef usei2c
    display1.begin(SSD1306_SWITCHCAPVCC, oled1_address, false, oled_sda, oled_scl);
  #else
    display1.begin(SSD1306_SWITCHCAPVCC);
  #endif
  display1.setRotation(OLED1_ROTATION); //upside down
  display1.setTextColor(WHITE);
  display1.setFont();
  display1.setTextSize(1);
  display1.clearDisplay();
  oled_startscreen();
  display1.display();
#endif
#ifdef useoled2
  #ifdef usei2c
    display2.begin(SSD1306_SWITCHCAPVCC, oled2_address, false,oled_sda, oled_scl);
  #else
    display2.begin(SSD1306_SWITCHCAPVCC);
  #endif
  display1.setRotation(0);
  oled_startscreen();
  display1.setRotation(OLED1_ROTATION); //upside down
  display2.setTextColor(WHITE);
  display2.setFont();
  display2.setTextSize(1);
  display2.display();
  //display2.startscrollright(1,64);
#endif
#if defined useoled1 || defined useoled2
  delay(2000);
#endif
#ifdef useoled1
  display1.clearDisplay();
  display1.display();
#endif
#ifdef useoled2
  display2.clearDisplay();
  display2.display();
#endif
  #ifdef usestatusled
    pinMode(led,OUTPUT);
    digitalWrite(led,LOW);
  #endif
  #ifdef ESP32
    DebugSerial.begin(115200);
    preferences.begin("my-app", false);
    unsigned int counter = preferences.getUInt("counter", 0);
    counter++;
    Serial.printf("Current counter value: %u\r\n", counter);
    preferences.putUInt("counter", counter);
    preferences.end();

  #endif
  #ifdef ESP8266
    DebugSerial.begin(115200);
    DebugSerial.setDebugOutput(true);
  #endif
  DebugSerial.println(swversion);
  wlanstate=wlanturnstaon;
  uint64_t cit;
  #ifdef ESP32
    cit = ESP.getEfuseMac();
    sprintf(chipid,"%02X%02X%02X",(uint8_t)(cit>>24),(uint8_t)(cit>>32),(uint8_t)(cit>>40));
    DebugSerial.println(chipid);
    sprintf(mac,"%02X%02X%02X%02X%02X%02X",(uint8_t)(cit),(uint8_t)(cit>>8),(uint8_t)(cit>>16),(uint8_t)(cit>>24),(uint8_t)(cit>>32),(uint8_t)(cit>>40));
  #elif defined(ESP8266)
    cit = ESP.getChipId();
    sprintf(chipid,"%02X%02X%02X",(uint8_t)(cit>>24),(uint8_t)(cit>>32),(uint8_t)(cit>>40));
    byte mc[6];
    WiFi.macAddress(mc);
    sprintf(mac, "%02X%02X%02X%02X%02X%02X", mc[0], mc[1], mc[2], mc[3], mc[4], mc[5]);
  #endif
  ArduinoOTA.onStart([]() {
    DebugSerial.println("OTA Start");
    #ifdef usemqtt
        if (client.connected()) {
          sprintf(tmp1, "n/%d/OTAStart", mqtt_clientID);
          client.publish(tmp1, "OTA Starting");
        }
    #endif
    #ifdef useoled1
        display1.clearDisplay();
        display1.display();
    #endif
    #ifdef useoled2
        display1.clearDisplay();
        oled_startscreen();
        display1.display();
        display2.clearDisplay();
        display2.display();
    #endif
  });
  ArduinoOTA.onEnd([]() {
    DebugSerial.println("\nOTA End");
    #ifdef usemqtt
        if (client.connected()) {
          sprintf(tmp1, "n/%d/OTAEND", mqtt_clientID);
          client.publish(tmp1, "OTA Done");
        }
    #endif
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DebugSerial.printf("OTA %02u%%\r\n", (progress / (total / 100)));
    #if (defined useoled1 && !defined useoled2)
        if ((progress / (total / 100))>lastprogress) {
          lastprogress = progress / (total / 100);
        }
        display1.clearDisplay();
        display1.setTextSize(1);
        display1.setTextColor(WHITE);
        display1.setCursor(0,10);
        display1.setFont();
        display1.print("  Updating  Firmware");
        display1.setCursor(54,50);
        display1.printf("%02u%%", lastprogress);
        display1.drawRect(14,25,100,10,WHITE);
        //display1.drawRect(15,33,14+(uint8_t)lastprogress,39,WHITE);
        display1.fillRect(14,26,(uint8_t)lastprogress,8,WHITE);
        display1.display();
    #endif
    #ifdef useoled2
        if ((progress / (total / 100))>lastprogress) {
          lastprogress = progress / (total / 100);
        }
        display2.clearDisplay();
        display2.setTextSize(1);
        display2.setTextColor(WHITE);
        display2.setCursor(0,10);
        display2.setFont();
        display2.print("  Updating  Firmware");
        display2.setCursor(54,50);
        display2.printf("%02u%%", lastprogress);
        display2.drawRect(14,25,100,10,WHITE);
        //display1.drawRect(15,33,14+(uint8_t)lastprogress,39,WHITE);
        display2.fillRect(14,26,(uint8_t)lastprogress,8,WHITE);
        display2.display();
    #endif
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if (error == OTA_AUTH_ERROR) sprintf(tmp1,"%s","Auth Failed");
    else if (error == OTA_BEGIN_ERROR) sprintf(tmp1,"%s","Begin Failed");
    else if (error == OTA_CONNECT_ERROR) sprintf(tmp1,"%s","Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) sprintf(tmp1,"%s","Receive Failed");
    else if (error == OTA_END_ERROR) sprintf(tmp1,"%s","End Failed");
    sprintf(tmp2, "OTA Error [%u]: %s", error, tmp1);
    DebugSerial.println(tmp2);
    #ifdef usemqtt
      if (client.connected()) {
        sprintf(tmp1, "n/%d/OTAError", mqtt_clientID);
        client.publish(tmp1, tmp2);
        yield(); //give wifi/mqtt a chance to send before reboot
      }
    #endif
    #ifdef useoled1
      display1.clearDisplay();
      display1.setTextSize(1);
      display1.setTextColor(WHITE);
      display1.setCursor(0,0);
      display1.setFont();
      display1.printf("OTA ERROR\r\nNr: %u\r\n%s", error,tmp1);
      display1.display();
      delay(2000); //give user a chance to read before reboot
    #endif    
  });
  ArduinoOTA.setPassword(OTApwd);
  sprintf(tmp2, "es-m365-%s",mac);
  ArduinoOTA.setHostname(tmp2);
} //setup

void loop() {
  timestamp_mainloopstart=micros();
  handle_wlan();
  #ifdef usetelnetserver
    handle_telnet();
  #endif  
  while (M365Serial.available()) { //decode anything we received as long as  there's data in the buffer
    m365_receiver(); 
    m365_handlepacket();
  }
  if (newdata) {
    m365_updatestats();
  }
  //m365_detectapp(); //detect if smartphone is connected and requests data, so we stay quiet
  #ifdef useoled1
    handle_oled();
  #endif
  ArduinoOTA.handle();
  #ifdef usestatusled
    handle_led();
  #endif
  #ifdef usemqtt
    client.loop();
  #endif
  #ifdef debug_dump_states
    print_states();
  #endif
  duration_mainloop=micros()-timestamp_mainloopstart;
}

