//Clean readme.md
//commit/push

//fix: display km/mile units (values are correct, just the display of the unit!)

//add option in menu for beeping on alert
//add alert / treshold for low battery -> make use of alertcounter_undervoltage

//auto-show ALERT Screen when stopped and alert condition set
//OR
//auto-popup errors for some seconds when we where driving and now stopped

//fix alertcounters -> should not count up if m365 value = 0 (nothing received for that value so far...)

//LOCKED AND SPEED? BLINK OLED?

//fix timing/crc issues
//add trip computer
//add mqtt logging
//add navigation code
//add flashprotetection function



//******************************************
//*************************************
//START OF File:
//current version working on esp32 ONLY
//esp8266 has no preferences class, i'm not using esp8266 anymore, if someone implements load/saveconfig methods for esp8266 i'm happy to merge your pull-request

//needs patch in esp32-arduino-core/esp32-hal-uart.c -> see comments in uart-section below and patch yourself or use https://github.com/smartinick/arduino-esp32
//needs patched Adafruit_SSD1306 Library (custom pins, higher clock speer) -> compare with base or clone from https://github.com/smartinick/Adafruit_SSD1306

/* known BUGS: 
 * - apssid/appassword not applied when switching from client-mode/search to AP
 * - does not receive/decode all data after doing OTA, reboot once and it works
 */

//please update config.h to reflect your hardwaresetup

#include "config.h"
#include "strings.h"

#define swversion "18.10.08"

//usually nothing must/should be changed below

#if (defined usei2c && defined useoled1)
    Adafruit_SSD1306 display1(oled_reset);
    Adafruit_SSD1306* displaydraw;
#endif
#if (defined usei2c && defined useoled2)
    Adafruit_SSD1306 display2(oled_reset);
#endif

#if (!defined usei2c && defined useoled1)
    //software spi Adafruit_SSD1306 display1(OLED_MOSI, OLED_CLK, OLED_DC, OLED1_RESET, OLED1_CS);
    //hardware spi with patched Adafruit_SSD1306 Library:
    Adafruit_SSD1306 display1(OLED_MISO, OLED_MOSI, OLED_CLK, OLED_DC, OLED1_RESET, OLED1_CS, 4000000UL);
    Adafruit_SSD1306* displaydraw;
#endif
#if (!defined usei2c && defined useoled2)
    //software spi Adafruit_SSD1306 display2(OLED_MOSI, OLED_CLK, OLED_DC, OLED1_RESET, OLED2_CS);
    //hardware spi
    Adafruit_SSD1306 display2(OLED_MISO, OLED_MOSI, OLED_CLK, OLED_DC, OLED2_RESET, OLED2_CS, 4000000UL);
#endif

#if (defined useoled1 || defined useoled2)
    #include <Adafruit_GFX.h>
    #include <Fonts/ARIALNB9pt7b.h> //Arial Narrow Bold Size 9 "33" = 15x13px
    #include <Fonts/ARIALN9pt7b.h> //Arial Narrow Size 9 "33" = 15x13px
    #include <Fonts/ariblk42pt7b.h> //Arial Black Size 42 "33" = 101x61px
    #include <Fonts/ARIALNB18pt7b.h> //Arial Narrow Bold Size 18 "33" = 30x25px


//old stuff, clean up:
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
    
    boolean oledreinit = false;
    #define oledreinitduration 30000
    unsigned long oledreinittimestamp = 0;

    unsigned long popuptimestamp = 0;
    boolean showpopup = false;
    #define popupalertduration 5000

    boolean showdialog = false;
    char diag_title[30];
    uint8_t *diag_value;
    uint8_t diag_min;
    uint8_t diag_max;

    //buffers for drawscreen stuff
    #define dsvalbuflen 10
    char val1buf[dsvalbuflen];
    char val2buf[dsvalbuflen];
    char val3buf[dsvalbuflen];
    char val4buf[dsvalbuflen];
    char popuptitle[30];
    char popuptext[100];

//move to screen.h START
  #define sp_lx 0
  #define sp_fontlabel &ARIALN9pt7b
  #define sp_dx 72 //64 for 4 ltter units, 72 for 3 letter units
  #define sp_dxnu 80
  #define sp_fontdata &ARIALNB9pt7b
  #define sp_ux 110 //104 for 4 letters, 112 for 3
  #define sp_fontunit

  #define menu_x 1
  #define menu_fontlabel &ARIALN9pt7b
  #define menu_fontlabelselected &ARIALNB9pt7b
  #define menu_fontdata &ARIALNB9pt7b

  #define popup_fontheader &ARIALNB9pt7b
  #define popup_header_x 13
  #define popup_header_y 22
  #define popup_fonttext &ARIALN9pt7b
  #define popup_text_x 13
  #define popup_text_y 42


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
  #define wlanapconnecttimeout 600000 //timeout in ap mode until client needs to connect / ap mode turns off
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
  uint16_t commands_sent=0;
  uint16_t requests_sent_housekeeper=0;

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
    uint16_t u8[1]; //offset 0x162-0x163
    uint8_t lockstate; //offset 0x164
    uint8_t u9; //offset 0x165
    uint16_t u10; //offset 0x166-0x167
    uint16_t battpercent; //offset 0x168-0x169
    int16_t speed; //0x16A-0x16B /1000 in km/h, negative value = backwards...
    uint16_t averagespeed; //0x16C-0x16D /1000 in km/h?
    uint32_t totaldistance; //0x16e-0x171 /1000 in km
    uint16_t tripdistance; //0x172-0x173
    uint16_t ontime2; //offset 0x174-0x175 power on time 2 in seconds
    uint16_t frametemp2; //0x176-0x177 /10 = temp in °C
    uint16_t u11[68]; //offset 0x178-0x200 
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

//M365 - command stuff
  #define commandlen 10
  uint8_t command[commandlen] = { 0x55,0xAA,0x04,0x20,0x03,0x01,0x02,0x03,0xB7,0xFF};
  #define command_cmd 5
  #define command_value1 6
  #define command_value2 7
  #define command_crcstart 2
  #define command_crc1 8
  #define command_crc2 9

  uint8_t sendcommand = 0;
  #define cmd_none 0
  #define cmd_kers_weak 1
  #define cmd_kers_medium 2
  #define cmd_kers_strong 3
  #define cmd_cruise_on 4
  #define cmd_cruise_off 5
  #define cmd_light_on 6
  #define cmd_light_off 7
  #define cmd_turnoff 8
  #define cmd_lock 9
  #define cmd_unlock 10
  //#define cmd_beep 11


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
    #define requestbmslen 9
    uint8_t request_bms[requestbmslen] = { 0x55,0xAA,0x03,0x22,0x01,0x10,0x3A,0xB7,0xFF};
    //uint8_t request_bms_serial[9] = { 0x55,0xAA,0x03,0x22,0x01,0x10,0x12,0xB7,0xFF};
    #define bms_request_offset 5
    #define bms_request_len 6
    #define bms_request_crcstart 2
    #define bms_request_crc1 7
    #define bms_request_crc2 8
  //packets for esc requests and offsets
    #define requestesclen 12
    uint8_t request_esc[requestesclen] = { 0x55,0xAA,0x06,0x20,0x61,0x10,0xAC,0x02,0x28,0x27,0xB7,0xFF};
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
    #define requestmax 9
    uint8_t requests[requestmax][3]= {
        { address_esc, 0xB0, 0x20}, //error, lockstate, battpercent,speed,averagespeed,totaldistance,tripdistance,ontime2,frametemp2
        { address_esc, 0x25, 0x02}, //remaining distance
        { address_esc, 0x3A, 0x0A}, //ontime, triptime, frametemp1
        { address_bms, 0x31, 0x0A}, //remaining cap & percent, current, voltage, temperature
        { address_bms, 0x40, 0x18}, //cell voltages (Cells 1-10 & 11-12 for 12S Batt/Custom BMS)
        { address_bms, 0x3B, 0x02}, //Battery Health
        { address_bms, 0x10, 0x22}, //serial,fwversion,totalcapacity,cycles,chargingtimes,productiondate
        { address_esc, 0x10, 0x16}, //serial,pin,fwversion
        { address_esc, 0x7B, 0x06}  //kers, cruisemode, taillight
    };

  //friendly names
    #define rq_esc_essentials 0x0001
    #define rq_esc_remdist 0x0002
    #define rq_esc_essentials2 0x0004
    #define rq_bms_essentials 0x0008
    #define rq_bms_cells 0x0010
    #define rq_bms_health 0x0020
    #define rq_bms_versioninfos 0x0040
    #define rq_esc_versioninfos 0x0080
    #define rq_esc_config 0x0100

  //mapping oled screens / requeired data
    #define numscreens 8
    uint16_t rqsarray[numscreens] = {
      rq_esc_essentials|rq_esc_remdist|rq_esc_essentials2|rq_bms_essentials|rq_bms_cells|rq_bms_health|rq_bms_versioninfos|rq_esc_versioninfos, //rq_esc_essentials|rq_bms_essentials|rq_esc_essentials2, //screen_stop
      //0xff, //screen_stop, TODO: Request infos like Serial/FW Version only _once_ (when entering subscreen)
      rq_esc_essentials|rq_bms_essentials, //screen_drive
      rq_esc_essentials, //screen_error
      rq_esc_essentials, //screen_timeout
      rq_esc_essentials|rq_bms_essentials|rq_bms_cells, //charging
      rq_esc_essentials|rq_esc_config, //configmenu
      rq_esc_essentials|rq_esc_essentials2|rq_bms_essentials, //alarm
      rq_esc_essentials //screen_locked
    };

  uint16_t subscribedrequests =  rqsarray[0];
  uint16_t housekeeperrequests = rq_bms_cells|rq_esc_essentials|rq_bms_essentials;
  uint8_t hkrequestindex=0;

//Screen Switching
  uint8_t screen = 0;
  #define screen_stop 0
  #define screen_drive 1
  #define screen_error 2
  #define screen_timeout 3
  #define screen_charging 4
  #define screen_configmenu 5
  #define screen_alarm 6
  #define screen_locked 7

  uint8_t subscreen = 0;
  uint8_t windowsubpos=0;


  
  #if !defined useoled2 //Single Screen Stopscreens
    #define stopsubscreens 8
    #define stopsubscreen_trip 0 //single/dual/same
    #define stopsubscreen_temp 1 //single
    #define stopsubscreen_minmax 2 //single/dual/same
    #define stopsubscreen_batt1 3 //single/dual/same
    #define stopsubscreen_batt2 4 //single
    #define stopsubscreen_cells 5 //single/dual/different
    #define stopsubscreen_assets 6 //single/dual/same
    #define stopsubscreen_espstate 7 //single
    #define stopsubscreen_alarms 8 //single/dual/same
  #else //Dual Screen Stopscreens
    #define stopsubscreens 5
    #define stopsubscreen_trip 0
    #define stopsubscreen_minmax 1 //single/dual/same
    #define stopsubscreen_batt1 2 //single/dual/same
    #define stopsubscreen_cells 3
    #define stopsubscreen_assets 4 //single/dual/same
    #define stopsubscreen_alarms 5 //single/dual/same
  #endif
  #define chargesubscreens 2
  #define throttlemindefault 40
  #define throttlemaxdefault 190
  uint8_t throttlemin = throttlemindefault;
  uint8_t throttlemax = throttlemaxdefault;
  uint8_t stopwindowsize = (uint8_t)((throttlemax-throttlemin)/stopsubscreens);
  uint8_t chargewindowsize = (uint8_t)((throttlemax-throttlemin)/chargesubscreens);

//Config Menu
  #define cms_light 0 //tail ligth on/off //WORKING
  #define cms_cruise 1 //cruise mode on/off //WORKING
  #define cms_kers 2 //set kers //WORKING
  #define cms_ws 3 //set wheelsize //WORKING
  #define cms_unit 4 //kilometers or miles?
  #define cms_bc 5 //set number of cells (10/12s) //WORKING
  #define cms_buv 6 //set Battery undervoltage alarm
  #define cms_bac 7 //set Battery Alert CellVoltage Difference //WORKING
  #define cms_bat 8 //set Battery Alert Temperature //NOT IMPLEMENTED
  #define cms_eat 9 //set ESC Alert Temperature //NOT IMPLEMENTED
  #define cms_flashprotection 10 //activate flashprotection //NOT IMPLEMENTED
  #define cms_navigation 11 //activate ble/komoot navigaton display //NOT IMPLEMENTED
  #define cms_busmode 12 //busmode active/passive (request data from m365 or not...?) //WORKING
  #define cms_wifirestart 13 //restart wifi //WORKING
  #define cms_changelock 14 //WORKING
  #define cms_turnoff 15 //WORKING
  #define cms_exit 16 //exitmenu //WORKING
  #define configsubscreens 17
  uint8_t configwindowsize = (uint8_t)((throttlemax-throttlemin)/(configsubscreens-1));
#if !defined useoled2
  #define configlinesabove 1
  #define confignumlines 3
#else
  #define configlinesabove 1
  #define confignumlines 4
#endif

  uint8_t configstartindex = 0;
  uint8_t configendindex = 0;
  bool configchanged = false;
  uint8_t configcurrentitem = 0;


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

//timing & statistics
  unsigned long duration_mainloop=0;
  unsigned long timestamp_mainloopstart=0;
  unsigned long duration_oled=0;
  unsigned long timestamp_oledstart=0;
  unsigned long duration_oled1draw=0;
  unsigned long timestamp_oled1draw=0;
  unsigned long duration_oled1transfer=0;
  unsigned long timestamp_oled1transfer=0;
  unsigned long duration_oled2draw=0;
  unsigned long timestamp_oled2draw=0;
  unsigned long duration_oled2transfer=0;
  unsigned long timestamp_oled2transfer=0;
  unsigned long duration_telnet=0;
  unsigned long timestamp_telnetstart=0;

  uint16_t capacitychargestart = 0;

//helpers
  #define _max(a,b) ((a)>(b)?(a):(b))
  #define _min(a,b) ((a)<(b)?(a):(b))

//configmenu stuff
  Preferences preferences;
  uint8_t conf_wheelsize;
  uint8_t conf_battcells;
  uint8_t conf_alert_batt_celldiff;
  uint8_t conf_alert_batt_temp;
  uint8_t conf_alert_batt_voltage;
  uint8_t conf_alert_esc_temp;
  uint8_t conf_unit;
  bool conf_flashprotect;
  bool conf_espbusmode;
  #define wheelfact8km 1.0f
  #define wheelfact8miles  0.621371192f
  #define wheelfact10km    1.176470588f
  #define wheelfact10miles 0.731024932f

  float wheelfact = 0;

//housekeeper (manages alerts,...)
  uint8_t hkstate = 0;
  uint8_t hkstateold = 0;
  #define hkwaiting 0
  #define hkrequesting 1
  #define hkevaluating 2

  #define housekeepertimeout 3000 //duration between housekeeper-datarequests
  unsigned long housekeepertimestamp = housekeepertimeout;

//alerts
  bool alert_escerror = false;
  bool alert_cellvoltage = false;
  bool alert_bmstemp = false;
  bool alert_esctemp = false;
  bool alert_undervoltage = false;
  uint16_t alertcounter_escerror = 0;
  uint16_t alertcounter_cellvoltage = 0;
  uint16_t alertcounter_bmstemp = 0;
  uint16_t alertcounter_esctemp = 0;
  uint16_t alertcounter_undervoltage = 0;

//buttonhandling
  uint8_t brakebuttonstate = 0;  //1 = short, 2 = long press
  boolean brakebuttonpressed = false; //helper for key-detection
  #define buttonbrakepressed1 60 //treshold for "button pressed"
  #define buttonbrakeshortpressedduration 50 //millis needed for short press
  #define buttonbrakelongpressedduration 500 //millis needed for long press
  unsigned long buttonbrakepressedtimestamp = 0;

  uint8_t throttlebuttonstate = 0;  //1 = short, 2 = long press
  boolean throttlebuttonpressed = false; //helper for key-detection
  #define buttonthrottlepressed1 150 //treshold for "button pressed"
  #define buttonthrottleshortpressedduration 50 //millis needed for short press
  #define buttonthrottlelongpressedduration 500 //millis needed for long press
  unsigned long buttonthrottlepressedtimestamp = 0;

  uint8_t bothbuttonstate = 0;  //1 = short, 2 = long press
  boolean bothbuttonpressed = false; //helper for key-detection
  #define buttonbothshortpressedduration 50 //millis needed for short press
  #define buttonbothlongpressedduration 500 //millis needed for long press
  unsigned long buttonbothpressedtimestamp = 0;

//and finally.... *tada* the start of code.... :D

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

void m365_updatebattstats() {
  //Cell Voltages
    highest = 0;
    lowest =10000;
    if (conf_battcells>=1 & bmsparsed->Cell1Voltage>0) { lowest=_min(lowest,bmsparsed->Cell1Voltage); highest=_max(highest,bmsparsed->Cell1Voltage); }
    if (conf_battcells>=2 & bmsparsed->Cell2Voltage>0) { lowest=_min(lowest,bmsparsed->Cell2Voltage); highest=_max(highest,bmsparsed->Cell2Voltage); }
    if (conf_battcells>=3 & bmsparsed->Cell3Voltage>0) { lowest=_min(lowest,bmsparsed->Cell3Voltage); highest=_max(highest,bmsparsed->Cell3Voltage); }
    if (conf_battcells>=4 & bmsparsed->Cell4Voltage>0) { lowest=_min(lowest,bmsparsed->Cell4Voltage); highest=_max(highest,bmsparsed->Cell4Voltage); }
    if (conf_battcells>=5 & bmsparsed->Cell5Voltage>0) { lowest=_min(lowest,bmsparsed->Cell5Voltage); highest=_max(highest,bmsparsed->Cell5Voltage); }
    if (conf_battcells>=6 & bmsparsed->Cell6Voltage>0) { lowest=_min(lowest,bmsparsed->Cell6Voltage); highest=_max(highest,bmsparsed->Cell6Voltage); }
    if (conf_battcells>=7 & bmsparsed->Cell7Voltage>0) { lowest=_min(lowest,bmsparsed->Cell7Voltage); highest=_max(highest,bmsparsed->Cell7Voltage); }
    if (conf_battcells>=8 & bmsparsed->Cell8Voltage>0) { lowest=_min(lowest,bmsparsed->Cell8Voltage); highest=_max(highest,bmsparsed->Cell8Voltage); }
    if (conf_battcells>=9 & bmsparsed->Cell9Voltage>0) { lowest=_min(lowest,bmsparsed->Cell9Voltage); highest=_max(highest,bmsparsed->Cell9Voltage); }
    if (conf_battcells>=10 & bmsparsed->Cell10Voltage>0) { lowest=_min(lowest,bmsparsed->Cell10Voltage); highest=_max(highest,bmsparsed->Cell10Voltage); }
    if (conf_battcells>=11 & bmsparsed->Cell11Voltage>0) { lowest=_min(lowest,bmsparsed->Cell11Voltage); highest=_max(highest,bmsparsed->Cell11Voltage); }
    if (conf_battcells>=12 & bmsparsed->Cell12Voltage>0) { lowest=_min(lowest,bmsparsed->Cell12Voltage); highest=_max(highest,bmsparsed->Cell12Voltage); }
}

void m365_updatestats() {
  //automatic gas learning:
    if (bleparsed->throttle!=0) {
      if (throttlemin>bleparsed->throttle) { 
        throttlemin=bleparsed->throttle; 
        stopwindowsize = (uint8_t)((throttlemax-throttlemin)/stopsubscreens);
        chargewindowsize = (uint8_t)((throttlemax-throttlemin)/chargesubscreens);
        configwindowsize = (uint8_t)((throttlemax-throttlemin)/(configsubscreens-1));
      }
      if (throttlemax<bleparsed->throttle) { 
        throttlemax=bleparsed->throttle;
        stopwindowsize = (uint8_t)((throttlemax-throttlemin)/stopsubscreens);
        chargewindowsize = (uint8_t)((throttlemax-throttlemin)/chargesubscreens);
        configwindowsize = (uint8_t)((throttlemax-throttlemin)/(configsubscreens-1));
      }

    }
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
  m365_updatebattstats();
} //m365_updatestats

void handle_housekeeper() {
  if (hkstate!=hkstateold) {
    hkstateold=hkstate;
    #ifdef debug_dump_states
      Serial.printf("### HKSTATE %d -> %d\r\n",hkstateold,hkstate);
      print_states();
    #endif
  }
  switch(hkstate) {
    case hkwaiting: //check timeout, if over request data
        if (millis()>housekeepertimestamp) {
          hkstate = hkrequesting;
          hkrequestindex = 0; //start new hk requestcycle
          requests_sent_housekeeper++;
          //set requestor stuff
        }
      break;
    case hkrequesting: //waiting until we received data...
    break;
  case hkevaluating: //evaluate alarms,... rearm timeout
    //check error register
      if (escparsed->error!=0) {
        if (!alert_escerror) {
          alertcounter_escerror++;
          alert_escerror = true;
          sprintf(tmp1,"%d (eror screen!)",escparsed->error);
          popup((char*)"ESC ERROR", tmp1, popupalertduration);

        }
      }
    //batt low voltage alarm:
      if ((uint8_t)((float)bmsparsed->voltage/100.0f) < conf_alert_batt_voltage) {
        if (!alert_undervoltage) {
          alertcounter_undervoltage++;
          alert_undervoltage=true;
          sprintf(tmp1,"%f < %d",(float)bmsparsed->voltage/100.0f,conf_alert_batt_voltage);
          popup((char*)"LOW BATTERY", tmp1, popupalertduration);
        }
      }
    //cell voltage difference alarm:
      m365_updatebattstats();
      if ((highest-lowest) >= conf_alert_batt_celldiff*10) {
        if (!alert_cellvoltage) {
          alertcounter_cellvoltage++;
          alert_cellvoltage = true;
          sprintf(tmp1,"%d > %d",(highest-lowest),conf_alert_batt_celldiff*10);
          popup((char*)"CELL ALERT", tmp1, popupalertduration);
          //popup((char*)"CELL ALERT", (char*)"Volt Difference", 5000);
        }
      } else {
        alert_cellvoltage = false;
      }
    //bms temp alarm
      if (((bmsparsed->temperature[1]-20) >=conf_alert_batt_temp) | ((bmsparsed->temperature[0]-20) >= conf_alert_batt_temp)) {
        if (!alert_bmstemp) {
          alertcounter_bmstemp++;
          alert_bmstemp = true;
          sprintf(tmp1,"%d / %d > %d",(bmsparsed->temperature[0]-20),(bmsparsed->temperature[1]-20), conf_alert_batt_temp);
          popup((char*)"BATT TEMP", tmp1, popupalertduration);
          //popup((char*)"BATT TEMP", (char*)"Temp over limit!", 5000);
        }
      } else {
        alert_bmstemp = false;
      }
    //esc temp alarm
      if (((float)escparsed->frametemp2/10.0f) >= (float)conf_alert_esc_temp) {
        if (!alert_esctemp) {
          alertcounter_esctemp++;
          alert_esctemp = true;
          sprintf(tmp1,"%f > %d",(float)escparsed->frametemp2/10.0f, conf_alert_esc_temp);
          popup((char*)"ESC TEMP", tmp1, popupalertduration);
        }
      } else {
        alert_esctemp = false;
      }
    //last step: rearm
      housekeepertimestamp = millis() + housekeepertimeout;
      hkstate = hkwaiting;
      //popup((char*)"hk title", (char*)"hk text", 1000);
    break;
  } //switch hkstate
} //handle_housekeeper


void m365_sendcommand(uint8_t cvalue, uint8_t cparam1, uint8_t cparam2) {
  command[command_cmd] = cvalue;
  command[command_value1] = cparam1;
  command[command_value2] = cparam2;
  crccalc = 0x00;
  for(uint8_t i=command_crcstart;i<command_crc1;i++) {
    crccalc=crccalc + command[i];
  }
  crccalc = crccalc ^ 0xffff;
  command[command_crc1]=(uint8_t)(crccalc&0xff);
  command[command_crc2]=(uint8_t)((crccalc&0xff00)>>8);
  M365Serial.write((unsigned char*)&command,commandlen);
  commands_sent++;
} //m365_sendcommand

void m365_sendrequest(uint8_t radr, uint8_t roffset, uint8_t rlen) {
    if (radr==address_bms) {
    request_bms[bms_request_offset] = roffset;
    request_bms[bms_request_len] = rlen;
    crccalc = 0x00;
    for(uint8_t i=bms_request_crcstart;i<bms_request_crc1;i++) {
      crccalc=crccalc + request_bms[i];
    }
    crccalc = crccalc ^ 0xffff;
    request_bms[bms_request_crc1]=(uint8_t)(crccalc&0xff);
    request_bms[bms_request_crc2]=(uint8_t)((crccalc&0xff00)>>8);
    M365Serial.write((unsigned char*)&request_bms,requestbmslen);
    requests_sent_bms++;
  } //if address address_bms
  if (radr==address_esc) {
    request_esc[esc_request_offset] = roffset;
    request_esc[esc_request_len] = rlen;
    request_esc[esc_request_throttle] = bleparsed->throttle;
    request_esc[esc_request_brake] = bleparsed->brake;
    crccalc = 0x00;
    for(uint8_t i=esc_request_crcstart;i<esc_request_crc1;i++) {
      crccalc=crccalc + request_esc[i];
    }
    crccalc = crccalc ^ 0xffff;
    request_esc[esc_request_crc1]=(uint8_t)(crccalc&0xff);
    request_esc[esc_request_crc2]=(uint8_t)((crccalc&0xff00)>>8);
    M365Serial.write((unsigned char*)&request_esc,requestesclen);
    requests_sent_esc++;
  } //if address address_esc
} //m365_sendrequest

void m365_handlerequests() {
  uint8_t startindex;
  //1st prio: send comands?
  if (sendcommand != cmd_none) {
      switch(sendcommand) {
          case cmd_kers_weak: m365_sendcommand(0x7b,0,0); break;
          case cmd_kers_medium: m365_sendcommand(0x7b,1,0); break;
          case cmd_kers_strong: m365_sendcommand(0x7b,2,0); break;
          case cmd_cruise_on: m365_sendcommand(0x7c,1,0); break;
          case cmd_cruise_off: m365_sendcommand(0x7c,0,0); break;
          case cmd_light_on: m365_sendcommand(0x7d,2,0); break;
          case cmd_light_off: m365_sendcommand(0x7d,0,0); break;
          case cmd_turnoff: m365_sendcommand(0x79,01,0); break;
          case cmd_lock: m365_sendcommand(0x70,01,0); break;
          case cmd_unlock: m365_sendcommand(0x71,01,0); break;
        } //switch sendcommand
    sendcommand = cmd_none; //reset  
  } else {
    //2nd prio: housekeeper todos?
        if (hkstate==hkrequesting) {
          //find next subscribed index
          startindex = hkrequestindex;
          while (!(housekeeperrequests&(1<<hkrequestindex)))  {
            //DebugSerial.printf("HK skipping RQ index %d\r\n",hkrequestindex+1);
            hkrequestindex++;
            if (hkrequestindex==requestmax) {
                //DebugSerial.println("HK rollover in rqloop1");
                hkrequestindex=0;
                hkstate=hkevaluating; //we are done with one request-cycle, evaluate... 
            }
            if (hkrequestindex==startindex) { break; }
          }
        //request data for current index if we are interested in
        if (housekeeperrequests&(1<<hkrequestindex)) {
          //DebugSerial.printf("HK requesting RQ index %d\r\n",hkrequestindex+1);
          m365_sendrequest(requests[hkrequestindex][0], requests[hkrequestindex][1], requests[hkrequestindex][2]);
        } /*else {
          DebugSerial.printf("HK skipping RQ index %d\r\n",hkrequestindex+1);
        }*/
        //prepare next requestindex for next call
        hkrequestindex++;
        if (hkrequestindex==requestmax) { 
          //DebugSerial.println("HK rollover in rqloop2");
          hkrequestindex=0; 
          hkstate=hkevaluating; //we are done with one request-cycle, evaluate... 
        }
        //DebugSerial.printf("---HKREQUEST-- %d\r\n",millis());
      } else {
      //3rd prio - request other data for display
      //find next subscribed index
        startindex = requestindex;
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
        m365_sendrequest(requests[requestindex][0], requests[requestindex][1], requests[requestindex][2]);
      } /*else {
        DebugSerial.printf("skipping RQ index %d\r\n",requestindex+1);
      }*/
      //prepare next requestindex for next call
      requestindex++;
      if (requestindex==requestmax) { 
        //DebugSerial.println("rollover in rqloop2");
        requestindex=0; 
        duration_requestcycle=millis()-timestamp_requeststart;
        timestamp_requeststart=millis();
      }
      //DebugSerial.printf("---REQUEST-- %d\r\n",millis());
    } //else if hkstate
  } //else if sendcommand
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
        if (sbuf[i_address]==0x20 && sbuf[i_hz]==0x65 && sbuf[i_offset]==0x00 && conf_espbusmode) {
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
                telnetclient.printf(" Highest: %1.3f Lowest: %1.3f Difference: %1.3f\r\n",(float)highest/1000.0f, (float)lowest/1000.0f, (float)(highest-lowest)/1000.0f); 
                sprintf(tmp1,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c",escparsed->serial[0],escparsed->serial[1],escparsed->serial[2],escparsed->serial[3],escparsed->serial[4],escparsed->serial[5],escparsed->serial[6],escparsed->serial[7],escparsed->serial[8],escparsed->serial[9],escparsed->serial[10],escparsed->serial[11],escparsed->serial[12],escparsed->serial[13]);
                telnetclient.printf("\r\n\r\nESC Serial: %s Version: %x.%x.%x\r\n", tmp1,escparsed->fwversion[1],(escparsed->fwversion[0]&0xf0)>>4,escparsed->fwversion[0]&0x0f);
                telnetclient.printf(" Distance Total: %.2f km Trip: %.2f km Remaining %.2f km\r\n", (float)escparsed->totaldistance/1000.0f,(float)escparsed->tripdistance/100.0f,(float)escparsed->remainingdistance/100.0f);
                telnetclient.printf(" Power On Time1: %d s, Power On Time1: %d s, Trip Time: %d s\r\n",escparsed->ontime1,escparsed->ontime2,escparsed->triptime);
                telnetclient.printf(" FrameTemp1: %05d FrameTemp2: %.1f °C\r\n", (float)escparsed->frametemp1/10.0f, (float)escparsed->frametemp2/10.0f);
                telnetclient.printf(" Speed: %.2f km/h Avg: %.2f km/h\r\n", (float)escparsed->speed/1000.0f, (float)escparsed->averagespeed/1000.0f);
                telnetclient.printf(" Batt Percent: %3d%%\r\n",escparsed->battpercent);
                telnetclient.printf(" Ecomode: %05d Kers: %05d Cruisemode: %05d Taillight: %05d\r\n", escparsed->ecomode, escparsed->kers, escparsed->cruisemode, escparsed->taillight);
                sprintf(tmp1,"%c%c%c%c%c%c",escparsed->pin[0],escparsed->pin[1],escparsed->pin[2],escparsed->pin[3],escparsed->pin[4],escparsed->pin[5]);
                telnetclient.printf(" Pin: %s\r\n Error %05d Lockstate %02d\r\n",tmp1,escparsed->error, escparsed->lockstate);
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
              telnetclient.printf("Commands Sent: %05d   HK-Cycles: %05d\r\n", commands_sent, requests_sent_housekeeper);
              telnetclient.printf("Packets Received:\r\n  ESC: %05d   BMS: %05d   BLE: %05d   X1: %05d   unhandled: %05d\r\n", packets_rec_esc,packets_rec_bms,packets_rec_ble,packets_rec_x1,packets_rec_unhandled);
              telnetclient.printf("  CRC OK: %05d   CRC FAIL: %05d\r\n   Total: %05d\r\n",packets_crcok,packets_crcfail,packets_rec);
              telnetclient.printf("\r\nALARM States:\r\n  Cellvoltage Differnce (Is: %d mV, Alert: %d0 mV): %s\r\n",highest-lowest, conf_alert_batt_celldiff, alert_cellvoltage ? "true" : "false");
              telnetclient.printf("  BMS Temperature (Is: %d °C %d °C, Alert: %d °C): %s\r\n",bmsparsed->temperature[1]-20, bmsparsed->temperature[0]-20,conf_alert_batt_temp, alert_bmstemp ? "true" : "false");
              telnetclient.printf("  ESC Temperarture (Is: %3.1f °C, Alert: %d °C): %s\r\n",(float)escparsed->frametemp2/10.0f, conf_alert_esc_temp, alert_esctemp ? "true" : "false");
              telnetclient.printf("  ALARM Counters: Cellvoltage: %d, BMS Temperature: %d, ESC Temperature %d\r\n", alertcounter_cellvoltage, alertcounter_bmstemp, alertcounter_esctemp);

/*                    m365_updatebattstats();
      if ((highest-lowest)*100 >= conf_alert_batt_celldiff) {
        alert_cellvoltage = true;
      } else {
        alert_cellvoltage = false;
      }
    //bms temp alarm
      if (((bmsparsed->temperature[1]-20) >=conf_alert_batt_temp) | ((bmsparsed->temperature[2]-20) >= conf_alert_batt_temp)) {
        alert_bmstemp = true;
      } else {
        alert_bmstemp = false;
      }
    //esc temp alarm
      if (((float)escparsed->frametemp2/10.0f) >= (float)conf_alert_esc_temp) {
        alert_esctemp = true;
      } else {
        alert_esctemp = false;
      }
    //last step: rearm
      housekeepertimestamp = millis() + housekeepertimeout;
      hkstate = hkwaiting;
    break;
              bool alert_cellvoltage = false;
  bool alert_bmstemp = false;
  bool alert_esctemp = false;
*/
              telnetclient.printf("\r\nTimers:\r\n  Main:   %04.3f ms\r\n  Telnet: %04.3f ms\r\n",(float)duration_mainloop/1000.0f, (float)duration_telnet/1000.0f);
              telnetclient.printf("  OLED Main: %04.3f ms\r\n",(float)duration_oled/1000.0f);
              telnetclient.printf("  OLED1 Draw: %04.3f ms\r\n  OLED1 Transfer:  %04.3f ms\r\n",(float)duration_oled1draw/1000.0f,(float)duration_oled1transfer/1000.0f);
              telnetclient.printf("  OLED2 Draw: %04.3f ms\r\n  OLED2 Transfer:  %04.3f ms\r\n",(float)duration_oled2draw/1000.0f,(float)duration_oled2transfer/1000.0f);
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
        //delay(500);
        //WiFi.mode(WIFI_OFF);
        //yield();
        WiFi.mode(WIFI_AP);
        delay(100);
        WiFi.softAP(apssid, appassword);
        yield();
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


void applyconfig() {
  //TODO - add conf_unit

  if (conf_wheelsize==0 & conf_unit==0) { wheelfact = wheelfact8km; } //8" and kilometers
  if (conf_wheelsize==0 & conf_unit==1) { wheelfact = wheelfact8miles; } //8" and miles
  if (conf_wheelsize==1 & conf_unit==0) { wheelfact = wheelfact10km; } //10" and kilometers
  if (conf_wheelsize==1 & conf_unit==1) { wheelfact = wheelfact10miles; } //10" and miles
}

void loadconfig() {
  preferences.begin("espm365config", true); //open in readonly mode
  conf_wheelsize = preferences.getUChar("wheelsize", 0); //0=8.5", 1=10"
  conf_battcells = preferences.getUChar("battcells", 10); //10 = 10S, 12 = 12S
  conf_alert_batt_celldiff = preferences.getUChar("alertcelldiff", 5); //50mV difference -> alert
  conf_alert_batt_voltage = preferences.getUChar("alertlowvoltage", 30); //default 30V 
  conf_alert_batt_temp = preferences.getUChar("alertbatttemp", 50); //50° Celsius -> alert
  conf_alert_esc_temp = preferences.getUChar("alertesctemp", 50);  //50° Celsius -> alert
  conf_flashprotect = preferences.getBool("flashprotect",false);
  conf_espbusmode = preferences.getBool("busmode",true); //false = ESP does not request data, true = ESP requests data
  conf_unit = preferences.getUChar("unit",0); //unit: 0 = kilmeters, 1 = miles
  preferences.end();
  applyconfig();
}

void saveconfig() {
  preferences.begin("espm365config", false); //open in rw-mode
  preferences.clear(); //remove all values
  preferences.putUChar("wheelsize", conf_wheelsize); //0=8.5", 1=10"
  preferences.putUChar("battcells", conf_battcells); //10 = 10S, 12 = 12S
  preferences.putUChar("alertcelldiff", conf_alert_batt_celldiff); //50mV difference -> alert
  preferences.putUChar("alertlowvoltage", conf_alert_batt_voltage); //30V default
  preferences.putUChar("alertbatttemp", conf_alert_batt_temp); //50° Celsius -> alert
  preferences.putUChar("alertesctemp", conf_alert_esc_temp);  //50° Celsius -> alert
  preferences.putBool("flashprotect",conf_flashprotect);
  preferences.putBool("busmode",conf_espbusmode); //false = ESP does not request data, true = ESP requests data
  preferences.putUChar("unit",conf_unit); //unit: 0 = kilmeters, 1 = miles
  preferences.end();
}

void handle_configmenuactions() {
    switch(subscreen) {
      case cms_light:
          switch(escparsed->taillight) {
            case 0: sendcommand = cmd_light_on; break;
            case 2: sendcommand = cmd_light_off; break;
          }
        break;
      case cms_cruise:
          switch(escparsed->cruisemode) {
            case 0: sendcommand = cmd_cruise_on; break;
            case 1: sendcommand = cmd_cruise_off; break;
          }
        break;
      case cms_kers:
          switch(escparsed->kers) {
            case 0: sendcommand = cmd_kers_medium; break;
            case 1: sendcommand = cmd_kers_strong; break;
            case 2: sendcommand = cmd_kers_weak; break;
          }
        break;
      case cms_ws: 
          if (conf_wheelsize==0) { 
              conf_wheelsize=1; 
            } else {
              conf_wheelsize=0; 
            }
            configchanged = true;
        break;
      case cms_unit: 
          if (conf_unit==0) { 
              conf_unit=1; 
            } else {
              conf_unit=0; 
            }
            configchanged = true;
        break;
      case cms_buv:
            dialog_edit_int8(FPSTR(s_setvolt),&conf_alert_batt_voltage,10,40);
            configchanged = true;
          break;
      case cms_bc: 
          if (conf_battcells==10) { 
            conf_battcells=12; 
          } else {
            conf_battcells=10;
          }
          configchanged = true;
        break;
      case cms_bac:
          switch (conf_alert_batt_celldiff) {
            case 1: conf_alert_batt_celldiff=3; break;
            case 3: conf_alert_batt_celldiff=5; break;
            case 5: conf_alert_batt_celldiff=10; break;
            case 10: conf_alert_batt_celldiff=20; break;
            case 20: conf_alert_batt_celldiff=50; break;
            case 50: conf_alert_batt_celldiff=1; break;
            default: conf_alert_batt_celldiff=5; break;
          }
          configchanged = true;
        break;
      case cms_bat:
          dialog_edit_int8(FPSTR(s_settemp),&conf_alert_batt_temp,10,100);
          configchanged = true;
        break;
      case cms_eat:
          dialog_edit_int8(FPSTR(s_settemp),&conf_alert_esc_temp,10,100);
          configchanged = true;
        break;
      case cms_flashprotection: 
          conf_flashprotect = !conf_flashprotect;
          configchanged = true;
        break;
      /*case cms_navigation: display1.print("Start Navigation Mode");
        break;*/
      case cms_busmode: 
          conf_espbusmode = !conf_espbusmode;
          configchanged = true;
        break;
      case cms_wifirestart:
          wlanstate = wlanturnstaon;
          screen = screen_stop;
          subscreen = 0;
          if (configchanged) { saveconfig(); applyconfig(); }
        break;
      case cms_changelock:
          if (escparsed->lockstate==0x00) { 
            sendcommand = cmd_lock;
          } else {
            sendcommand = cmd_unlock;
            screen = screen_stop;
            subscreen = 0;
          }
          screen = screen_stop;
          subscreen = 0;
          if (configchanged) { saveconfig(); applyconfig(); }
        break;
      case cms_turnoff:
          sendcommand = cmd_turnoff;
          if (configchanged) { saveconfig(); applyconfig(); }
        break;
      case cms_exit:
          screen = screen_stop;
          subscreen = 0;
          if (configchanged) { saveconfig(); applyconfig(); }
        break;
    } //switch curline
} //handle_configmenuactions


//screendrawing start MOVE TO SCREEN.C START


void drawscreen_startscreen() {
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

void drawscreen_header(const char *h, uint8_t scrnum, uint8_t scrtotal) {
  //blestruct* bleparsed = (blestruct*)bledata;
  displaydraw->setFont();
  displaydraw->setCursor(0,0);
  if (scrtotal!=0) {
    displaydraw->printf("%-15.15s (%d/%d)\r\n",FPSTR(h),scrnum,scrtotal);
  } else {
    displaydraw->printf("%-21.21s\r\n",FPSTR(h));
  }
}

void drawscreen_data(bool headline, uint8_t lines, bool showunits,
               const char *l1, char *v1, const char *u1,
               const char *l2, char* v2, const char *u2,
               const char *l3, char* v3, const char *u3,
               const char *l4, char* v4, const char *u4) {
  uint8_t i;
  uint8_t line;
  for (i=0;i<lines;i++) {
      switch(i) {
        case 0:
            if (headline & lines==4) { line = dataoffset+baselineoffset; }
            if (!headline & lines==4) { line = baselineoffset; }
            if (headline & lines==3) { line = dataoffset+baselineoffset+5;}
            if (!headline & lines==3) { line = baselineoffset; }
            displaydraw->setFont(sp_fontlabel); displaydraw->setCursor(sp_lx,line); displaydraw->print(FPSTR(l1));
            if (showunits) {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dx,line); displaydraw->printf(v1);
              displaydraw->setCursor(sp_ux,line); displaydraw->setFont(sp_fontunit); displaydraw->print(FPSTR(u1));
            } else {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dxnu,line); displaydraw->printf(v1);
            }
          break;
        case 1:
            if (headline & lines==4) { line = dataoffset+baselineoffset*2+linespace; }
            if (!headline & lines==4) { line = baselineoffset*2+linespace+2;}
            if (headline & lines==3) { line = dataoffset+baselineoffset*2+10; }
            if (!headline & lines==3) { line = baselineoffset*2+5+4; }
            displaydraw->setFont(sp_fontlabel); displaydraw->setCursor(sp_lx,line); displaydraw->print(FPSTR(l2));
            if (showunits) {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dx,line); displaydraw->printf(v2);
              displaydraw->setCursor(sp_ux,line); displaydraw->setFont(sp_fontunit); displaydraw->print(FPSTR(u2));
            } else {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dxnu,line); displaydraw->printf(v2);
            }
          break;
        case 2:
            if (headline & lines==4) { line = dataoffset+baselineoffset*3+linespace*2; }
            if (!headline & lines==4) { line = baselineoffset*3+linespace*2+5;}
            if (headline & lines==3) { line = dataoffset+baselineoffset*3+15; }
            if (!headline & lines==3) { line = baselineoffset*3+10+8; }
            displaydraw->setFont(sp_fontlabel); displaydraw->setCursor(sp_lx,line); displaydraw->print(FPSTR(l3));
            if (showunits) {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dx,line); displaydraw->printf(v3);
              displaydraw->setCursor(sp_ux,line); displaydraw->setFont(sp_fontunit); displaydraw->print(FPSTR(u3));
            } else {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dxnu,line); displaydraw->printf(v3);
            }
          break;
        case 3:
            if (headline & lines==4) { line = dataoffset+baselineoffset*4+linespace*3; }
            if (!headline & lines==4) { line = baselineoffset*4+linespace*3+8;}
            displaydraw->setFont(sp_fontlabel); displaydraw->setCursor(sp_lx,line); displaydraw->print(FPSTR(l4));
            if (showunits) {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dx,line); displaydraw->printf(v4);
              displaydraw->setCursor(sp_ux,line); displaydraw->setFont(sp_fontunit); displaydraw->print(FPSTR(u4));
            } else {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dxnu,line); displaydraw->printf(v4);
            }
          break;
      } //Switch i
  } //for i
} //drawscreen


void popup (char *_popuptitle, char *_popuptext, uint16_t duration) {
  sprintf(popuptitle, "%s", _popuptitle);
  sprintf(popuptext, "%s", _popuptext);
  showpopup = true;
  popuptimestamp = millis()+duration;
}

void drawscreen_popup() {
  //singlescreen version:
  display1.fillRect(4,4,119,55,BLACK);
  display1.drawRect(6,6,115,51,WHITE); //cheap frame
  display1.drawRect(7,7,113,49,WHITE); //cheap frame
  display1.setFont(popup_fontheader); display1.setCursor(popup_header_x, popup_header_y); 
  display1.print(popuptitle);
  display1.setFont(popup_fonttext); display1.setCursor(popup_text_x, popup_text_y); 
  display1.print(popuptext);
}

void dialog_edit_int8(const char *_dialogtitle, uint8_t *_value, uint8_t _minvalue, uint8_t _maxvalue) {
  sprintf(diag_title, "%s", _dialogtitle);
  diag_value = _value;
  diag_min = _minvalue;
  diag_max = _maxvalue;
  showdialog = true;
}

void drawscreen_dialog() {
  //singlescreen version:
  display1.fillRect(4,4,119,55,BLACK);
  display1.drawRect(6,6,115,51,WHITE); //cheap frame
  display1.drawRect(7,7,113,49,WHITE); //cheap frame
  display1.setFont(popup_fontheader); display1.setCursor(popup_header_x, popup_header_y); 
  display1.print(diag_title);
  display1.setFont(popup_fonttext); display1.setCursor(popup_text_x, popup_text_y); 
  display1.printf("%d",*diag_value);
}

void drawscreen_screenconfigsingle() {
  uint8_t line = baselineoffset;
    display1.setFont(menu_fontlabel); display1.setCursor(menu_x,line); display1.print("conf entry 1");
  line = baselineoffset*2+linespace+1;
    display1.setFont(menu_fontlabelselected); display1.setCursor(menu_x,line); display1.print("conf entry selecte");
  line = baselineoffset*3+linespace*2+3;
    display1.setFont(menu_fontlabel); display1.setCursor(menu_x,line); display1.print("conf entry 2");
  
  display1.drawRect(120,0,8,64,BLACK);
  display1.drawRect(0,baselineoffset*2+linespace-12,122,16,WHITE);
  //display1.drawFastVLine(125,0,line+2,WHITE);  
  display1.drawFastVLine(126,0,line+2,WHITE);
  display1.drawRect(125,15,3,10,WHITE);

  display1.drawFastHLine(0,baselineoffset*3+linespace*2+3+4,128,WHITE);
  line = baselineoffset*4+linespace*3+8;
  display1.setFont(menu_fontdata); display1.setCursor(menu_x,63); display1.print("current value");
}


//screendrawing start MOVE TO SCREEN.C END
//screendrawing stop
void oled_switchscreens() {
  uint8_t oldscreen = screen;
  
  //1st. prio: Data/Bus Timeout
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

  //2nd prio - locked?
  if (escparsed->lockstate==0x02 & screen!=screen_configmenu) {
    screen=screen_locked;
  }
  
  //"button" handling
    if (newdata & (bleparsed->brake>=buttonbrakepressed1) & !brakebuttonpressed) {
      buttonbrakepressedtimestamp = millis();
      brakebuttonpressed=true;
      brakebuttonstate=0;
    }
    if (newdata & (bleparsed->brake<buttonbrakepressed1) & brakebuttonpressed) {
      if (millis()>buttonbrakepressedtimestamp+buttonbrakelongpressedduration) { 
        brakebuttonstate=2;
      } else {
        if (millis()>buttonbrakepressedtimestamp+buttonbrakeshortpressedduration) { 
          brakebuttonstate=1;
        }
      }
      if (brakebuttonstate!=0) { brakebuttonpressed=false; }
    }

    if (newdata & (bleparsed->throttle>=buttonthrottlepressed1) & !throttlebuttonpressed) {
      buttonthrottlepressedtimestamp = millis();
      throttlebuttonpressed=true;
      throttlebuttonstate=0;
    }
    if (newdata & (bleparsed->throttle<buttonthrottlepressed1) & throttlebuttonpressed) {
      if (millis()>buttonthrottlepressedtimestamp+buttonthrottlelongpressedduration) { 
        throttlebuttonstate=2;
      } else {
        if (millis()>buttonthrottlepressedtimestamp+buttonthrottleshortpressedduration) { 
          throttlebuttonstate=1;
        }
      }
      if (throttlebuttonstate!=0) { throttlebuttonpressed=false; }
    }

    if (newdata & (bleparsed->brake>=buttonbrakepressed1) & (bleparsed->throttle>=buttonthrottlepressed1) & !bothbuttonpressed & screen!=screen_configmenu) {
      buttonbothpressedtimestamp = millis();
      bothbuttonpressed=true;
      bothbuttonstate=0;
    }
    if (newdata & (bleparsed->brake<buttonbrakepressed1) & (bleparsed->throttle<buttonthrottlepressed1) & bothbuttonpressed) {
      if (millis()>buttonbothpressedtimestamp+buttonbothlongpressedduration) { 
        bothbuttonstate=2;
      } else {
        if (millis()>buttonbothpressedtimestamp+buttonbothshortpressedduration) { 
          bothbuttonstate=1;
        }
      }
      if (bothbuttonstate!=0) { bothbuttonpressed=false; }
    }
  
  //execute commands from configscreen
    if (screen==screen_configmenu) {
      if (showdialog) {
        //1st prio: handle value-change dialogs
        updatescreen = true;
        *diag_value = map(bleparsed->throttle,throttlemin,throttlemax,diag_min,diag_max);
        if (brakebuttonstate!=0) { //brake pressed, exit dialog?
          showdialog = false;
        }
      } else {
        //2nd prio: handle brake-button actions
        //if (screen==screen_configmenu & brakebuttonstate!=0 & !showdialog) {
        if (brakebuttonstate!=0) {  
          handle_configmenuactions();
          updatescreen = true;
        }
      }
    }

  //switch to configmenu
    //if (brakebuttonstate!=0 & throttlebuttonstate!=0 & escparsed->speed==0) {
    if (bothbuttonstate!=0 & escparsed->speed==0 & screen!=screen_configmenu) {
      screen = screen_configmenu;
      subscreen = 0;
      configchanged = false;
      updatescreen = true;
      popup((char*)"  MENU", (char*)"release throttle!", 1000);
    }
/*
  //exit from configmenu via long-brake-press
    if (brakebuttonstate==2 & screen==screen_configmenu) {
    //if (brakebuttonstate==1 & screen==screen_stop & subscreen == stopsubscreens) {
      if (configchanged) { saveconfig(); }
      screen = screen_stop;
      subscreen = 0;
      updatescreen = true;
    }
    */
  //configmenu navigaton via gas:
  if (newdata & (screen==screen_configmenu) & !showdialog) {
    uint8_t oldsubscreen = subscreen;
    if (bleparsed->throttle>throttlemin+5) {
      subscreen = ((bleparsed->throttle-throttlemin) / configwindowsize)+1;
      subscreen=_min(subscreen,configsubscreens-1);
      windowsubpos = (uint8_t)((float)((bleparsed->throttle-throttlemin) % configwindowsize)*(float)oledwidth/(float)configwindowsize);
    } else {
      subscreen=0;
      windowsubpos=0;
    }
    if ((subscreen)>configsubscreens) { subscreen=configsubscreens; }
    if (subscreen!=oldsubscreen) { 
      configstartindex = _max(subscreen - configlinesabove,0);
      configendindex = _min(configstartindex + confignumlines-1,configsubscreens-1);
      if ((configendindex-confignumlines+1)<configstartindex) {
        configstartindex = configendindex - confignumlines+1;
      }
      updatescreen = true; 
    }
  }
  //charging screens: 
    if (newdata & (escparsed->speed==0) & (bmsparsed->current<0) & (screen!=screen_charging)) { 
      if  (screen!=screen_locked) {
        //only display if not locked...
        screen=screen_charging; 
      }
      //timeout_oled=millis()+oledchargestarttimeout;
      capacitychargestart = bmsparsed->remainingcapacity;
      updatescreen=true;
    }
    if (newdata & (screen==screen_charging) & (abs((float)escparsed->speed/1000.0f)>2.0f)) { 
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
    if (bleparsed->throttle>throttlemin+5) {
      subscreen = ((bleparsed->throttle-throttlemin) / stopwindowsize)+1;
      windowsubpos = (uint8_t)((float)((bleparsed->throttle-throttlemin) % stopwindowsize)*(float)oledwidth/(float)stopwindowsize);
    } else {
      subscreen=0;
      windowsubpos=0;
    }
    if ((subscreen)>stopsubscreens) { subscreen=stopsubscreens; }
    if (subscreen!=oldsubscreen) { updatescreen = true; }
  }
  
  #if !defined useoled2
  //switching of subscreens via throttle while we are in CHARGE mode, only needed with one screen
  if (newdata & (screen==screen_charging)) {
    uint8_t oldsubscreen = subscreen;
    if (bleparsed->throttle>=throttlemin) {
      subscreen = ((bleparsed->throttle-throttlemin) / chargewindowsize)+1;
      windowsubpos = (uint8_t)((float)((bleparsed->throttle-throttlemin) % chargewindowsize)*(float)oledwidth/(float)chargewindowsize);
    } else {
      subscreen=1;
      windowsubpos=0;
    }
    if ((subscreen)>chargesubscreens) { subscreen=chargesubscreens; }
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
  
  //DebugSerial.printf("### Br: %03d Thr: %03d B: %01d T: %01d BT: %01d Scr: %01d\r\n",escparsed->brake, escparsed->throttle, brakebuttonstate, throttlebuttonstate, bothbuttonstate, screen);
  //reset buttonstate
    brakebuttonstate=0;
    throttlebuttonstate=0;
    bothbuttonstate=0;

  //popup timeout handling
  if ((showpopup) & (millis() > popuptimestamp)) {
    showpopup = false;
  }
} //oled_switchscreens

void cm_printKey(uint8_t entryid) {
          switch(entryid) {
            case cms_light: display1.print(FPSTR(menu_light));
              break;
            case cms_cruise: 
                display1.print(FPSTR(menu_cruise));
              break;
            case cms_kers: 
                display1.print(FPSTR(menu_kers));
              break;
            case cms_ws: display1.print(FPSTR(menu_wheelsize));
              break;
            case cms_unit: display1.print(FPSTR(menu_unit));
              break;
            case cms_buv: display1.print(FPSTR(menu_battalertlowvoltage));
              break;
            case cms_bc: display1.print(FPSTR(menu_battcells));
              break;
            case cms_bac: display1.print(FPSTR(menu_battalertcell));
              break;
            case cms_bat: display1.print(FPSTR(menu_battalerttemp));
              break;
            case cms_eat: display1.print(FPSTR(menu_escalerttemp));
              break;
            case cms_flashprotection: display1.print("***shprot");
              break;
            case cms_navigation: display1.print("***igation");
              break;
            case cms_busmode: display1.print(FPSTR(menu_espbusmode));
              break;
            case cms_wifirestart: display1.print(FPSTR(menu_espwifirestart));
              break;
            case cms_changelock: 
                  if (escparsed->lockstate==0x00) {
                    display1.print(FPSTR(menu_m365lock));
                  } else {
                    display1.print(FPSTR(menu_m365unlock));
                  }
              break;
            case cms_turnoff: display1.print(FPSTR(menu_m365turnoff));
              break;
            case cms_exit: display1.print(FPSTR(menu_exit));
              break;
          } //switch curline
}

void cm_printValue(uint8_t entryid) {
  #if!defined useoled2
    displaydraw->setCursor(5,dataoffset+baselineoffset + 3*(baselineoffset+linespace));
    displaydraw->drawFastHLine(0,dataoffset+baselineoffset + 2*(baselineoffset+linespace)-linespace-linespace,128,WHITE);
  #endif
  #ifdef useoled2
    displaydraw->setCursor(5,31-baselineoffset/2);
  #endif
          switch(entryid) {
            case cms_light:
                  switch(escparsed->taillight) {
                    case 0: displaydraw->print(FPSTR(menu_off)); break;
                    case 2: displaydraw->print(FPSTR(menu_on)); break;
                  }
              break;
            case cms_cruise: 
                  switch(escparsed->cruisemode) {
                    case 0: displaydraw->print(FPSTR(menu_off)); break;
                    case 1: displaydraw->print(FPSTR(menu_on)); break;
                  }
              break;
            case cms_kers: 
                  switch(escparsed->kers) {
                    case 0: displaydraw->print(FPSTR(menu_weak)); break;
                    case 1: displaydraw->print(FPSTR(menu_medium)); break;
                    case 2: displaydraw->print(FPSTR(menu_strong)); break;
                  }
              break;
            case cms_ws: 
                if (conf_wheelsize==0) { displaydraw->print("8.5\""); }
                if (conf_wheelsize==1) { displaydraw->print("10\""); }
                if (conf_wheelsize>1) { displaydraw->print("???"); }
              break;
            case cms_unit: 
                if (conf_unit==0) { displaydraw->print(FPSTR(menu_km)); }
                if (conf_unit==1) { displaydraw->print(FPSTR(menu_miles)); }
              break;
            case cms_bc:
                displaydraw->printf("%d", conf_battcells);
              break;
            case cms_buv: 
                displaydraw->printf("%d V", conf_alert_batt_voltage);
              break;
            case cms_bac:
                displaydraw->printf("%d0 mV", conf_alert_batt_celldiff);
              break;
            case cms_bat:
                  displaydraw->printf("%d C", conf_alert_batt_temp);
              break;
            case cms_eat:
                    displaydraw->printf("%d C", conf_alert_esc_temp);
              break;
            //case cms_flashprotection: displaydraw->print("no value");
            //case cms_navigation: displaydraw->print("no value");
            case cms_busmode:
                if (conf_espbusmode) { 
                  displaydraw->print(FPSTR(menu_active));
                } else { 
                  displaydraw->print(FPSTR(menu_passive)); 
                }
              break;
            //case cms_wifirestart: displaydraw->print("no value");
            //case cms_changelock:  displaydraw->print("no value");
            //case cms_turnoff:  displaydraw->print("no value");
            //case cms_exit:  displaydraw->print("no value");
          } //switch curline
}

#ifdef useoled1
  void oled1_update() {
    uint8_t line;
    uint8_t lineitem;
    timestamp_oled1draw=micros();

    if (oledreinit) {
      oled1init();
      #if !defined useoled2
        oledreinit=false;
      #endif
    }
    display1.clearDisplay();
    displaydraw = &display1;
    
    if (screen==screen_drive) {
        display1.setFont(&ariblk42pt7b);
        display1.setCursor(0,62);
        display1.printf("%02d", uint8_t(abs((float)escparsed->speed/1000.0f*wheelfact)));
        display1.setFont(&ARIALNB18pt7b);
        display1.setCursor(108,28);
        display1.printf("%01d", uint16_t((float)escparsed->speed/100.0f*wheelfact) %10); 
        display1.setFont(&ARIALNB9pt7b);
        if (x1parsed->light==0x64) { 
          display1.setCursor(118,45);
          display1.print("L");
        }
        display1.setCursor(118,63);
        if (x1parsed->mode<2) { 
          //display1.print("N"); //normal mode
        } else {
          display1.print("E"); //eco mode
        }
    }
    if (screen==screen_stop) {
          display1.setFont();
          display1.setCursor(0,0);
          switch (subscreen) {
            case stopsubscreen_trip: //Trip Info: Average Speed, Distance, Time, Average Speed 
                drawscreen_header(FPSTR(headline_tripinfo),0,0);
                sprintf(val1buf,"%05.2f",(float)escparsed->averagespeed/1000.0f*wheelfact);
                sprintf(val2buf,"%05.2f",(float)escparsed->tripdistance/100.0f*wheelfact);
                sprintf(val3buf,"%02d:%02d",escparsed->triptime/60,escparsed->triptime % 60);
                sprintf(val4buf,"%05.2f",(float)escparsed->remainingdistance/100.0f);
                  drawscreen_data(true, 4, true,
                      FPSTR(label_averageshort),&val1buf[0],FPSTR(unit_speed),
                      FPSTR(label_distanceshort),&val2buf[0],FPSTR(unit_distance),
                      FPSTR(label_time),&val3buf[0],FPSTR(label_seconds),
                      FPSTR(label_remainingshort),&val4buf[0],FPSTR(unit_distance));
              break;
              #if !defined useoled2
                case stopsubscreen_temp: //Single
                    drawscreen_header(FPSTR(headline_temperature),subscreen,stopsubscreens);
                    sprintf(val1buf,"%4.1f",(float)bmsparsed->temperature[0]-20.0f);
                    sprintf(val2buf,"%4.1f",(float)bmsparsed->temperature[1]-20.0f);
                    sprintf(val3buf,(float)escparsed->frametemp2/10.0f);
                    drawscreen_data(true, 3, true,
                      FPSTR(label_batt1),&val1buf[0],FPSTR(unit_temp),
                      FPSTR(label_batt2),&val2buf[0],FPSTR(unit_temp),
                      FPSTR(label_esc),&val3buf[0],FPSTR(unit_temp),
                      NULL,NULL,NULL);
                  break;
              #endif
              case stopsubscreen_minmax: //Single/Dual/Same
              //fix this screen - single and dual version...
                display1.printf("TRIP Min/Max    (%d/%d)\r\n\r\n",subscreen,stopsubscreens);
                display1.printf("Speed : %5.1f %5.1f\r\n",(float)speed_min/1000.0f,(float)speed_max/1000.0f);
                display1.printf("Ampere: %5.1f %5.1f A\r\n",(float)current_min/100.0f,(float)current_max/100.0f);
                display1.printf("Watt  : %5d %5d W\r\n",watt_min,watt_max);
                display1.printf("BattT1: %3d.0 %3d.0 C\r\n",tb1_min-20,tb1_max-20); display1.drawCircle(117,42,1,WHITE);
                display1.printf("BattT2: %3d.0 %3d.0 C\r\n",tb2_min-20,tb2_max-20); display1.drawCircle(117,50,1,WHITE);
                display1.printf("ESC T : %5.1f %5.1f C\r\n",(float)te_min/10.0f,(float)te_max/10.0f); display1.drawCircle(117,58,1,WHITE);
              break;
              case stopsubscreen_batt1: //Single/Dual/Same
                #if !defined useoled2
                  drawscreen_header(FPSTR(headline_battery1),subscreen,stopsubscreens);
                #else
                  drawscreen_header(FPSTR(headline_batterystatus),subscreen,stopsubscreens);
                #endif
                sprintf(val1buf,"%5.2f",(float)bmsparsed->voltage/100.0f);
                sprintf(val2buf,"%4d%",bmsparsed->remainingpercent);
                sprintf(val3buf,"%4d",bmsparsed->cycles);
                sprintf(val4buf,"%4d",bmsparsed->chargingtimes);
                drawscreen_data(true, 4, true,
                    FPSTR(label_volt),&val1buf[0],FPSTR(unit_volt),
                    FPSTR(label_percent),&val2buf[0],FPSTR(unit_percent),
                    FPSTR(label_cycles),&val3buf[0],FPSTR(unit_count),
                    FPSTR(label_charges),&val4buf[0],FPSTR(unit_count));
              break;
              #if !defined useoled2  
                case stopsubscreen_batt2: //Single Screen Batt Info 2: Health/Cycles/Charge Num/Prod Date
                    drawscreen_header(FPSTR(headline_battery2),subscreen,stopsubscreens);
                    sprintf(val1buf,"%4d",bmsparsed->health);
                    sprintf(val2buf,"%5d",bmsparsed->remainingcapacity);
                    sprintf(val3buf,"%5d",bmsparsed->totalcapacity);
                    sprintf(val4buf,"%02d/%02d",bmsparsed->temperature[0]-20,bmsparsed->temperature[1]-20);
                    drawscreen_data(true, 4, true,
                        FPSTR(label_health),&val1buf[0],FPSTR(unit_percent),
                        FPSTR(label_capacityshort),&val2buf[0],FPSTR(unit_mah),
                        FPSTR(label_totalcapacityshort),&val3buf[0],FPSTR(unit_mah),
                        FPSTR(label_tempshort),&val4buf[0],FPSTR(unit_temp));
              #endif
              case stopsubscreen_cells: //single/dual/different
                drawscreen_header(FPSTR(headline_cellvolt),subscreen,stopsubscreens);
                #if !defined useoled2
                  if (conf_battcells>=1) { display1.printf("01: %5.03f ",(float)bmsparsed->Cell1Voltage/1000.0f); }
                  if (conf_battcells>=2) { display1.printf("02: %5.03f  ",(float)bmsparsed->Cell2Voltage/1000.0f); }
                  if (conf_battcells>=3) { display1.printf("03: %5.03f ",(float)bmsparsed->Cell3Voltage/1000.0f); }
                  if (conf_battcells>=4) { display1.printf("04: %5.03f  ",(float)bmsparsed->Cell4Voltage/1000.0f); }
                  if (conf_battcells>=5) { display1.printf("05: %5.03f ",(float)bmsparsed->Cell5Voltage/1000.0f); }
                  if (conf_battcells>=6) { display1.printf("06: %5.03f  ",(float)bmsparsed->Cell6Voltage/1000.0f); }
                  if (conf_battcells>=7) { display1.printf("07: %5.03f ",(float)bmsparsed->Cell7Voltage/1000.0f); }
                  if (conf_battcells>=8) { display1.printf("08: %5.03f  ",(float)bmsparsed->Cell8Voltage/1000.0f); }
                  if (conf_battcells>=9) { display1.printf("09: %5.03f  ",(float)bmsparsed->Cell9Voltage/1000.0f); }
                  if (conf_battcells>=10) { display1.printf("10: %5.03f ",(float)bmsparsed->Cell10Voltage/1000.0f); }
                  if (conf_battcells>=11) { display1.printf("11: %5.03f ",(float)bmsparsed->Cell11Voltage/1000.0f); }
                  if (conf_battcells>=12) { display1.printf("12: %5.03f  ",(float)bmsparsed->Cell12Voltage/1000.0f); }
                  display1.printf("%s %5.03f", FPSTR(label_maxdiff), (float)(highest-lowest)/1000.0f);
                #else
                  display1.setFont(&ARIALN9pt7b); 
                  display1.setCursor(0,21);
                  if (conf_battcells>=1) { display1.printf(" 1: %5.03f ",(float)bmsparsed->Cell1Voltage/1000.0f); }
                  if (conf_battcells>=2) { display1.printf(" 2: %5.03f",(float)bmsparsed->Cell2Voltage/1000.0f); }
                  display1.setCursor(0,35);
                  if (conf_battcells>=3) { display1.printf(" 3: %5.03f ",(float)bmsparsed->Cell3Voltage/1000.0f); }
                  if (conf_battcells>=4) { display1.printf(" 4: %5.03f",(float)bmsparsed->Cell4Voltage/1000.0f); }
                  display1.setCursor(0,49);
                  if (conf_battcells>=5) { display1.printf(" 5: %5.03f ",(float)bmsparsed->Cell5Voltage/1000.0f); }
                  if (conf_battcells>=6) { display1.printf(" 6: %5.03f",(float)bmsparsed->Cell6Voltage/1000.0f); }
                  display1.setCursor(0,63);
                  if (conf_battcells>=7) { display1.printf(" 7: %5.03f ",(float)bmsparsed->Cell7Voltage/1000.0f); }
                  if (conf_battcells>=8) { display1.printf(" 8: %5.03f",(float)bmsparsed->Cell8Voltage/1000.0f); }
                #endif  
              break;
              case stopsubscreen_assets: //Single/Dual/Same
                drawscreen_header(FPSTR(headline_assests),subscreen,stopsubscreens);
                sprintf(tmp1,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c",bmsparsed->serial[0],bmsparsed->serial[1],bmsparsed->serial[2],bmsparsed->serial[3],bmsparsed->serial[4],bmsparsed->serial[5],bmsparsed->serial[6],bmsparsed->serial[7],bmsparsed->serial[8],bmsparsed->serial[9],bmsparsed->serial[10],bmsparsed->serial[11],bmsparsed->serial[12],bmsparsed->serial[13]);
                display1.printf(FPSTR(s_bmsfw), bmsparsed->fwversion[1],(bmsparsed->fwversion[0]&0xf0)>>4,bmsparsed->fwversion[0]&0x0f,tmp1);
                sprintf(tmp1,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c",escparsed->serial[0],escparsed->serial[1],escparsed->serial[2],escparsed->serial[3],escparsed->serial[4],escparsed->serial[5],escparsed->serial[6],escparsed->serial[7],escparsed->serial[8],escparsed->serial[9],escparsed->serial[10],escparsed->serial[11],escparsed->serial[12],escparsed->serial[13]);
                display1.printf(FPSTR(s_escfw), escparsed->fwversion[1],(escparsed->fwversion[0]&0xf0)>>4,escparsed->fwversion[0]&0x0f,tmp1);
                sprintf(tmp1,"%c%c%c%c%c%c",escparsed->pin[0],escparsed->pin[1],escparsed->pin[2],escparsed->pin[3],escparsed->pin[4],escparsed->pin[5]);
                display1.printf(FPSTR(s_pin), tmp1);
                display1.printf(FPSTR(s_miles),(float)escparsed->totaldistance/1000.0f);
                display1.printf(FPSTR(s_battdate),((bmsparsed->proddate)&0xFE00)>>9,((bmsparsed->proddate)&0x1E0)>>5,(bmsparsed->proddate)&0x1f);
              break;
              #if !defined useoled2  
                case stopsubscreen_espstate: //single
                    drawscreen_header(FPSTR(headline_espstate),subscreen,stopsubscreens);
                    display1.print(FPSTR(s_firmware)); display1.println(swversion);
                    display1.print(FPSTR(s_wlan));
                    if (wlanstate==wlansearching) { display1.println(FPSTR(s_wlansearch)); }
                    if (wlanstate==wlanconnected) { display1.print(FPSTR(s_wlancon)); display1.println(WiFi.SSID()); display1.println(WiFi.localIP()); }
                    if (wlanstate==wlanap) { display1.print(FPSTR(s_wlanap)); display1.println(WiFi.softAPIP()); display1.println( WiFi.softAPIP());}
                    if (wlanstate==wlanoff) { display1.println(FPSTR(s_off)); }
                    display1.print(FPSTR(s_bleoff));
                  break;
              #endif
              case stopsubscreen_alarms: //single/dual/same
                drawscreen_header(FPSTR(headline_alerts),subscreen,stopsubscreens);
                sprintf(val1buf,"%03d",alertcounter_bmstemp);
                sprintf(val2buf,"%03d",alertcounter_esctemp);
                sprintf(val3buf,"%03d",alertcounter_cellvoltage);
                sprintf(val4buf,"%03d",alertcounter_undervoltage);
                drawscreen_data(true, 4, false,
                    FPSTR(label_bmstemp),&val1buf[0],FPSTR(unit_percent),
                    FPSTR(label_esctemp),&val2buf[0],FPSTR(unit_mah),
                    FPSTR(label_cellvoltage),&val3buf[0],FPSTR(unit_mah),
                    FPSTR(label_undervoltage),&val4buf[0],FPSTR(unit_temp));
              break;
          }
    }
    if (screen == screen_configmenu) {
      
      lineitem=0;
      for (uint8_t curline=configstartindex;curline<=configendindex;curline++) {
        if (curline==subscreen) {
          display1.setFont(sp_fontdata);
          #if !defined useoled2
            cm_printValue(curline);
          #endif
          #ifdef useoled2
            configcurrentitem=curline;
          #endif
        } else {
          display1.setFont(sp_fontlabel);
        }
        //#if !defined useoled2
          display1.setCursor(5,baselineoffset + lineitem*(baselineoffset+linespace));
        //#else
          display1.setCursor(5,baselineoffset + lineitem*(baselineoffset+linespace));
        //#endif
        cm_printKey(curline);
        lineitem++;
      } //curline loop
    } //screen == screen_configmenu
    if (screen==screen_charging) {
          display1.clearDisplay();
          display1.setFont();
          display1.setCursor(0,0);
          display1.drawFastHLine(0,8,windowsubpos,WHITE);
          #if !defined useoled2
          switch (subscreen) {
            case 1: //Trip Info: Average Speed, Distance, Time, Average Speed 
                drawscreen_header(FPSTR(s_blank),subscreen,2);
          #endif
            display1.setTextSize(1);
            display1.setTextColor(WHITE);
            display1.setFont(&ARIALN9pt7b); 
            display1.setCursor(2,baselineoffset);
            display1.print(FPSTR(s_charging));
            display1.printf(" %2d%%", bmsparsed->remainingpercent);
            display1.drawRect(14,23,100,10,WHITE);
            display1.fillRect(14,24,bmsparsed->remainingpercent,8,WHITE);
            display1.setFont();
            display1.setCursor(14,39);
            display1.printf("%4.1f V   %4.1f A\r\n", (float)bmsparsed->voltage/100.0f,abs((float)bmsparsed->current/100.0f));
            display1.setCursor(14,48);
            display1.printf("%4.0f W  %5d mAh",abs(((float)(bmsparsed->voltage/100.0f)*(float)bmsparsed->current/100.0f)),bmsparsed->remainingcapacity);
            display1.setCursor(14,57);
            display1.print(FPSTR(s_charged));
            display1.printf(" %5d mAh",bmsparsed->remainingcapacity-capacitychargestart);
          #if !defined useoled2
              break;
            case 2: //Cell Infos
                drawscreen_header(FPSTR(headline_charging),subscreen,2);
                if (conf_battcells>=1) { display1.printf("01: %5.03f ",(float)bmsparsed->Cell1Voltage/1000.0f); }
                if (conf_battcells>=2) { display1.printf("02: %5.03f  ",(float)bmsparsed->Cell2Voltage/1000.0f); }
                if (conf_battcells>=3) { display1.printf("03: %5.03f ",(float)bmsparsed->Cell3Voltage/1000.0f); }
                if (conf_battcells>=4) { display1.printf("04: %5.03f  ",(float)bmsparsed->Cell4Voltage/1000.0f); }
                if (conf_battcells>=5) { display1.printf("05: %5.03f ",(float)bmsparsed->Cell5Voltage/1000.0f); }
                if (conf_battcells>=6) { display1.printf("06: %5.03f  ",(float)bmsparsed->Cell6Voltage/1000.0f); }
                if (conf_battcells>=7) { display1.printf("07: %5.03f ",(float)bmsparsed->Cell7Voltage/1000.0f); }
                if (conf_battcells>=8) { display1.printf("08: %5.03f  ",(float)bmsparsed->Cell8Voltage/1000.0f); }
                if (conf_battcells>=9) { display1.printf("09: %5.03f ",(float)bmsparsed->Cell9Voltage/1000.0f); }
                if (conf_battcells>=10) { display1.printf("10: %5.03f  ",(float)bmsparsed->Cell10Voltage/1000.0f); }
                if (conf_battcells>=11) { display1.printf("11: %5.03f ",(float)bmsparsed->Cell11Voltage/1000.0f); }
                if (conf_battcells>=12) { display1.printf("12: %5.03f  ",(float)bmsparsed->Cell12Voltage/1000.0f); }
                if (conf_battcells<11) { display1.printf("L:  %5.03f H:  %5.03f\r\n", (float)(lowest)/1000.0f,(float)(highest)/1000.0f); }
                display1.printf("T:  %5.02f D:  %5.03f", (float)bmsparsed->voltage/100.0f,(float)(highest-lowest)/1000.0f);
              break;
          }
          #endif
    }
    //if (screen==screen_timeout) {
    if (screen==screen_error) {
        #if !defined(useoled2)
          display1.setFont(&ARIALN9pt7b);
          display1.setCursor(30,20);
          display1.print(FPSTR(error_error));
          display1.setCursor(50,40);
          display1.printf("%02d",escparsed->error);
          display1.setFont();
          display1.setCursor(0,56);
          switch (escparsed->error) {
            case 10: display1.print(FPSTR(error_10)); break;
            case 11: display1.print(FPSTR(error_11)); break;
            case 12: display1.print(FPSTR(error_12)); break;
            case 13: display1.print(FPSTR(error_13)); break;
            case 14: display1.print(FPSTR(error_14)); break;
            case 15: display1.print(FPSTR(error_15)); break;
            case 18: display1.print(FPSTR(error_18)); break;
            case 21: display1.print(FPSTR(error_21)); break;
            case 22: display1.print(FPSTR(error_22)); break;
            case 23: display1.print(FPSTR(error_23)); break;
            case 24: display1.print(FPSTR(error_24)); break;
            case 26: display1.print(FPSTR(error_26)); break;
            case 27: display1.print(FPSTR(error_27)); break;
            case 28: display1.print(FPSTR(error_28)); break;
            case 29: display1.print(FPSTR(error_29)); break;
            case 31: display1.print(FPSTR(error_31)); break;
            case 35: display1.print(FPSTR(error_35)); break;
            case 36: display1.print(FPSTR(error_36)); break;
            case 39: display1.print(FPSTR(error_39)); break;
            case 40: display1.print(FPSTR(error_40)); break;
            default: display1.print(FPSTR(error_other)); break;
          }
        #else
          display1.setFont(&ARIALN9pt7b);
          display1.setCursor(50,20);
          display1.print(FPSTR(error_error));
          display1.setCursor(70,45);
          display1.printf("%02d",escparsed->error);
          display1.drawBitmap(0,0,  scooter, 64,64, 1);
        #endif
    }
    if (screen==screen_timeout) {
      #ifdef useoled2
        drawscreen_startscreen();
      #else
        display1.drawBitmap(0,0,  scooter, 64,64, 1);
        display1.setFont(&ARIALN9pt7b);
        display1.setCursor(74,15);
        display1.print(FPSTR(s_timeout1));
        display1.setCursor(64,35);
        display1.print(FPSTR(s_timeout2));
        display1.setFont();
        if (wlanstate==wlansearching) { display1.setCursor(64,42); display1.print(FPSTR(s_wlan)); display1.setCursor(64,55); display1.print(FPSTR(s_wlansearch)); }
        if (wlanstate==wlanconnected) { display1.setCursor(64,42); display1.print(WiFi.SSID()); display1.setCursor(34,57); display1.print(WiFi.localIP()); }
        if (wlanstate==wlanap) { display1.setCursor(64,42); display1.print(WiFi.softAPIP()); display1.setCursor(34,57); display1.print( WiFi.softAPIP()); }
        if (wlanstate==wlanoff) { display1.setCursor(64,56); display1.print(FPSTR(s_wlanoff)); }
      #endif
    }
    if (screen==screen_locked) {
      //display1.drawBitmap(0,0,  scooter, 64,64, 1);  
      display1.setFont();
      display1.setCursor(39,31);
      display1.print(FPSTR(s_locked));
    }

      if ((screen==screen_stop)&(subscreen==0)) {
        //Lockstate
          //if (escparsed->lockstate==0x02) { 
          //  display1.setFont();
          //  display1.setCursor(88,line1);
          //  display1.print(FPSTR(s_locked));
          //} else {
        
            //LIGHT ON/OFF
              if (x1parsed->light==0x64) { 
                display1.setFont();
                display1.setCursor(120,line1);
                display1.print("L");
              }
            //NORMAL/ECO MODE
              display1.setFont();
              display1.setCursor(112,line1);
              if (x1parsed->mode<2) { 
                display1.print("N"); //normal mode
              } else {
                display1.print("E"); //eco mode
              }
            //WLAN STATUS
              display1.setFont();
              display1.setCursor(96,line1);
              if (wlanstate==wlansearching) { display1.print(FPSTR(s_wlansearchshort)); }
              if (wlanstate==wlanconnected) { display1.print(FPSTR(s_wlanconshort)); }
              if (wlanstate==wlanap) { display1.print(FPSTR(s_wlanapshort)); }
            }
          //} //else lockstate

    if (showpopup) {
      drawscreen_popup();
    }
//    #if !defined useoled2
      if (showdialog) {
        drawscreen_dialog();
      }
  //  #endif
    duration_oled1draw = micros()-timestamp_oled1draw;
    timestamp_oled1transfer=micros();
    display1.display();
    duration_oled1transfer = micros()-timestamp_oled1transfer;
    #ifdef useoled2
      updatescreen2 = true;
    #endif
  } //oled1_update
#endif

#ifdef useoled2
  void oled2_update() {
    uint8_t line;
    timestamp_oled2draw=micros();
    if (oledreinit) {
      oled2init();
      oledreinit=false;
    }
    display2.clearDisplay();
    displaydraw = &display2;
    if (screen==screen_drive) {
        displaydraw->setFont(&ARIALNB18pt7b); displaydraw->setCursor(0,31); displaydraw->printf("%3d", int16_t((float)(bmsparsed->voltage/100.0f)*(float)bmsparsed->current/100.0f));
        displaydraw->setFont(&ARIALN9pt7b); displaydraw->setCursor(50,31); displaydraw->print(FPSTR(unit_power));
        displaydraw->setFont(&ARIALNB18pt7b); displaydraw->setCursor(65,31); displaydraw->printf("%3d",bmsparsed->remainingpercent);
        displaydraw->setFont(&ARIALN9pt7b); displaydraw->setCursor(116,31); displaydraw->print(FPSTR(unit_percent));
        displaydraw->setFont(&ARIALNB18pt7b); displaydraw->setCursor(0,63); displaydraw->printf("%2d",uint8_t((float)bmsparsed->voltage/100.0f));
        displaydraw->setFont(&ARIALNB9pt7b); displaydraw->setCursor(34,63); displaydraw->printf(".%1d",uint8_t((float)bmsparsed->voltage/10.0f) %10);
        displaydraw->setFont(&ARIALN9pt7b); displaydraw->setCursor(50,63); displaydraw->print(FPSTR(unit_volt));
        displaydraw->setFont(&ARIALNB18pt7b); displaydraw->setCursor(65,63); displaydraw->printf("%2d",int16_t((float)bmsparsed->current/100.0f));
        displaydraw->setFont(&ARIALNB9pt7b); displaydraw->setCursor(100,63); displaydraw->printf(".%1d",int16_t((float)bmsparsed->current/10.0f) %10);
        displaydraw->setFont(&ARIALN9pt7b); displaydraw->setCursor(116,63); displaydraw->print(FPSTR(unit_current));
    }
    if (screen==screen_stop) {
          display2.setFont();
          display2.setCursor(0,0);
          switch (subscreen) {
            case stopsubscreen_trip: //Temperatures - we have 2 Battery Temperatures, but we only display the higher one - both can be seen on detail screens
              sprintf(val1buf,"%4d",bmsparsed->health);
              if ((float)bmsparsed->temperature[0]>=(float)bmsparsed->temperature[1]) {
                  sprintf(val1buf,"%4.1f",(float)bmsparsed->temperature[0]-20.0f);  
                } else {
                  sprintf(val1buf,"%4.1f",(float)bmsparsed->temperature[1]-20.0f);  
                }
              sprintf(val2buf,"%4.1f",(float)escparsed->frametemp2/10.0f);
              sprintf(val3buf,"%4.1f",(float)bmsparsed->voltage/100.0f);
              sprintf(val4buf,"%5d",bmsparsed->remainingpercent);
              drawscreen_data(false, 4, true,
                FPSTR(label_bmstemp),&val1buf[0],FPSTR(unit_temp),
                FPSTR(label_esctemp),&val2buf[0],FPSTR(unit_temp),
                FPSTR(label_volt),&val3buf[0],FPSTR(unit_volt),
                FPSTR(label_percent),&val4buf[0],FPSTR(unit_percent));
              break;
            case stopsubscreen_minmax:
                display2.printf("FIX THIS SCREEN (%d/%d)\r\n\r\n",subscreen,stopsubscreens);
              break;
            case stopsubscreen_batt1:
                sprintf(val1buf,"%4d",bmsparsed->health);
                sprintf(val2buf,"%5d",bmsparsed->remainingcapacity);
                sprintf(val3buf,"%5d",bmsparsed->totalcapacity);
                sprintf(val4buf,"%02d/%02d",bmsparsed->temperature[0]-20,bmsparsed->temperature[1]-20);
                drawscreen_data(true, 4, true,
                    FPSTR(label_health),&val1buf[0],FPSTR(unit_percent),
                    FPSTR(label_capacityshort),&val2buf[0],FPSTR(unit_mah),
                    FPSTR(label_totalcapacityshort),&val3buf[0],FPSTR(unit_mah),
                    FPSTR(label_tempshort),&val4buf[0],FPSTR(unit_temp));
              break;
            case stopsubscreen_cells:
                display2.setFont(&ARIALN9pt7b); 
                display2.setCursor(0,12);
                if (conf_battcells>=9) { display2.printf(" 9: %5.03f ",(float)bmsparsed->Cell9Voltage/1000.0f); }
                if (conf_battcells>=10) { display2.printf("10: %5.03f",(float)bmsparsed->Cell10Voltage/1000.0f); }
                display2.setCursor(0,12+14);
                if (conf_battcells>=11) { display2.printf("11: %5.03f ",(float)bmsparsed->Cell11Voltage/1000.0f); }
                if (conf_battcells>=12) { display2.printf("12: %5.03f",(float)bmsparsed->Cell12Voltage/1000.0f); }
                display2.setCursor(0,12+14+14);
                display2.printf("Lo: %5.03f Hi: %5.03f", (float)(lowest)/1000.0f,(float)(highest)/1000.0f);
                display2.setCursor(0,12+14+14+14);
                display2.printf("T : %5.02f D : %5.03f", (float)bmsparsed->voltage/100.0f,(float)(highest-lowest)/1000.0f);
              break;
            case stopsubscreen_assets:
                display2.print(FPSTR(headline_espstate));
                display2.print(FPSTR(s_firmware)); 
                display2.println(swversion);
                display2.print(FPSTR(s_wlan));
                if (wlanstate==wlansearching) { display2.println(FPSTR(s_wlansearch)); }
                if (wlanstate==wlanconnected) { display2.print(FPSTR(s_wlancon)); display2.println(WiFi.SSID()); display2.println(WiFi.localIP()); }
                if (wlanstate==wlanap) { display2.print(FPSTR(s_wlanap)); display2.println(WiFi.softAPIP()); display2.println( WiFi.softAPIP()); }
                if (wlanstate==wlanoff) { display2.println(FPSTR(s_off)); }
                display2.print(FPSTR(s_bleoff));  
              break;
            case stopsubscreen_alarms:
                drawscreen_header(FPSTR(headline_alerts),subscreen,stopsubscreens);
                sprintf(val1buf,"%03d",alertcounter_escerror);
                drawscreen_data(true, 1, false,
                    FPSTR(label_escerrorcounter),&val1buf[0],FPSTR(unit_percent),
                    NULL,NULL,NULL,
                    NULL,NULL,NULL,
                    NULL,NULL,NULL);
              break;
            default:
                display2.setFont();
                display2.setCursor(0,0);
                display2.print("OLED2 STOP DEFAULT");
              break;
          }
    }
    if (screen == screen_configmenu) {
        display2.setFont(sp_fontdata);
        cm_printValue(configcurrentitem);
    } //screen == screen_configmenu
    if (screen==screen_charging) {
                display2.setFont();
                display2.setCursor(0,0);
                if (conf_battcells>=1) { display2.printf("01: %5.03f ",(float)bmsparsed->Cell1Voltage/1000.0f); }
                if (conf_battcells>=2) { display2.printf("02: %5.03f  ",(float)bmsparsed->Cell2Voltage/1000.0f); }
                if (conf_battcells>=3) { display2.printf("03: %5.03f ",(float)bmsparsed->Cell3Voltage/1000.0f); }
                if (conf_battcells>=4) { display2.printf("04: %5.03f  ",(float)bmsparsed->Cell4Voltage/1000.0f); }
                if (conf_battcells>=5) { display2.printf("05: %5.03f ",(float)bmsparsed->Cell5Voltage/1000.0f); }
                if (conf_battcells>=6) { display2.printf("06: %5.03f  ",(float)bmsparsed->Cell6Voltage/1000.0f); }
                if (conf_battcells>=7) { display2.printf("07: %5.03f ",(float)bmsparsed->Cell7Voltage/1000.0f); }
                if (conf_battcells>=8) { display2.printf("08: %5.03f  ",(float)bmsparsed->Cell8Voltage/1000.0f); }
                if (conf_battcells>=9) { display2.printf("09: %5.03f ",(float)bmsparsed->Cell9Voltage/1000.0f); }
                if (conf_battcells>=10) { display2.printf("10: %5.03f  ",(float)bmsparsed->Cell10Voltage/1000.0f); }
                if (conf_battcells>=11) { display2.printf("11: %5.03f ",(float)bmsparsed->Cell11Voltage/1000.0f); }
                if (conf_battcells>=12) { display2.printf("12: %5.03f  ",(float)bmsparsed->Cell12Voltage/1000.0f); }
                display2.printf("L:  %5.03f H:  %5.03f\r\n", (float)(lowest)/1000.0f,(float)(highest)/1000.0f);
                display2.printf("T:  %5.02f D:  %5.03f", (float)bmsparsed->voltage/100.0f,(float)(highest-lowest)/1000.0f);
    }
    if (screen==screen_error) {
      display2.setFont(&ARIALN9pt7b);
      display2.setCursor(0,20);
      switch (escparsed->error) {
            case 10: display2.print(FPSTR(error_10)); break;
            case 11: display2.print(FPSTR(error_11)); break;
            case 12: display2.print(FPSTR(error_12)); break;
            case 13: display2.print(FPSTR(error_13)); break;
            case 14: display2.print(FPSTR(error_14)); break;
            case 15: display2.print(FPSTR(error_15)); break;
            case 18: display2.print(FPSTR(error_18)); break;
            case 21: display2.print(FPSTR(error_21)); break;
            case 22: display2.print(FPSTR(error_22)); break;
            case 23: display2.print(FPSTR(error_23)); break;
            case 24: display2.print(FPSTR(error_24)); break;
            case 26: display2.print(FPSTR(error_26)); break;
            case 27: display2.print(FPSTR(error_27)); break;
            case 28: display2.print(FPSTR(error_28)); break;
            case 29: display2.print(FPSTR(error_29)); break;
            case 31: display2.print(FPSTR(error_31)); break;
            case 35: display2.print(FPSTR(error_35)); break;
            case 36: display2.print(FPSTR(error_36)); break;
            case 39: display2.print(FPSTR(error_39)); break;
            case 40: display2.print(FPSTR(error_40)); break;
            default: display2.print(FPSTR(error_other)); break;
      }
    }
    if (screen==screen_timeout) {  
      display2.setFont(&ARIALN9pt7b);
      display2.setCursor(40,20);
      display2.print(FPSTR(s_timeout2));
      display2.setCursor(20,40);
      display2.print(FPSTR(s_timeout3));
      display2.setFont();
      if (wlanstate==wlansearching) { display2.setCursor(0,48); display2.print(FPSTR(s_wlan)); display2.print(FPSTR(s_wlansearch)); }
      if (wlanstate==wlanconnected) { display2.setCursor(0,48); display2.print("SSID: "); display2.print(WiFi.SSID()); display2.setCursor(0,57); display2.print("IP: "); display2.print(WiFi.localIP()); }
      if (wlanstate==wlanap) { display2.setCursor(0,48); display2.print("SSID: ");  display2.print(apssid); display2.setCursor(0,57); display2.print("IP: "); display2.print( WiFi.softAPIP()); }
      if (wlanstate==wlanoff) { display2.setCursor(40,57); display2.print(FPSTR(s_wlanoff)); }

    }
    if (screen==screen_locked) {  
      display2.setFont();
      display2.setCursor(39,31);
      display2.print(FPSTR(s_locked));
    }

    duration_oled2draw = micros()-timestamp_oled2draw;
    timestamp_oled2transfer=micros();
    display2.display();
    duration_oled2transfer = micros()-timestamp_oled2transfer;
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
  if (millis() > oledreinittimestamp) {
      oledreinit = true;
      oledreinittimestamp = millis() + oledreinitduration;
  }

  #ifdef useoled2 //workaround for slow i2c & 2 displays to avoid too long blocking of main code
    if (updatescreen2) {
      oled2_update();
      updatescreen2=false;
      //DebugSerial.printf("---OLED2----- %02d %d\r\n",screen, millis());
    } else {
  #endif
      if (updatescreen) {
        olednextrefreshtimestamp=millis()+oledrefreshanyscreen;
        oled1_update();
        //oled2_update();
        updatescreen=false;
        //DebugSerial.printf("---OLED1----- %02d %d\r\n",screen, millis());
      }
  #ifdef useoled2
    }
  #endif
  duration_oled = micros()-timestamp_oledstart;
  oled_blink!=oled_blink;
}
#endif

#ifdef useoled1
void oled1init() {
  #ifdef usei2c
    display1.begin(SSD1306_SWITCHCAPVCC, oled1_address, false, oled_sda, oled_scl, 800000UL);
  #else
    display1.begin(SSD1306_SWITCHCAPVCC);
  #endif
  display1.setRotation(OLED1_ROTATION);
  display1.setTextColor(WHITE);
  display1.setFont();
  display1.setTextSize(1);
  display1.clearDisplay();
}
#endif

#ifdef useoled2
void oled2init() {
  #ifdef usei2c
    display2.begin(SSD1306_SWITCHCAPVCC, oled2_address, false,oled_sda, oled_scl, 800000UL);
  #else
    display2.begin(SSD1306_SWITCHCAPVCC);
  #endif
  display2.setRotation(OLED2_ROTATION);
  display2.setTextColor(WHITE);
  display2.setFont();
  display2.setTextSize(1);
  display2.clearDisplay();
}
#endif

void setup() {
  start_m365();
#ifdef useoled1
  oled1init();
  drawscreen_startscreen();
  display1.display();
#endif
#ifdef useoled2
  oled2init();
  display1.setRotation(0);
  drawscreen_startscreen();
  display1.setRotation(OLED1_ROTATION); //upside down
  display2.display();
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
    /*preferences.begin("my-app", false);
    unsigned int counter = preferences.getUInt("counter", 0);
    counter++;
    Serial.printf("Current counter value: %u\r\n", counter);
    preferences.putUInt("counter", counter);
    preferences.end();
*/
    loadconfig();
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
    #if !defined useoled2
        oled1init();
        display1.clearDisplay();
        display1.display();
    #endif
    #ifdef useoled2
        oled1init();
        oled2init();
        display1.clearDisplay();
        drawscreen_startscreen();
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
    handle_housekeeper();
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
