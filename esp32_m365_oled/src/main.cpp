/*  ESP32 M365
 * see readme.md for details
 *
 * configuration adoptions should be done in definitions.h
 * secrets.h must be created (see readme.md)
 * boards.h can be created to store your hardware-setup locally and keep it when updates are rolling in (see readme.md)
 * language adoptions in strings.h - i'm happy to add new languages/translations from you ;)
 *
 * needs patch in esp32-arduino-core/esp32-hal-uart.c (112 byte hardware uart-receive buffer is to big for "realtime" communication)
 *  -> see comments in definitions.h / readme.md and patch yourself or use https://github.com/smartinick/arduino-esp32
 * for now this has to be done locally on your dev machine
 *
 * needs patched Adafruit_SSD1306 Library (patched fork see https://github.com/smartinick/Adafruit_SSD1306)
 * platformio.ini is configured to use this library - no user interaction needed
 *
 * known BUGS: 
 * - apssid/appassword not applied when switching from client-mode/search to AP
 * - does not receive/decode all data after doing OTA, reboot once and it works
 * - "beep" command does not work in current version
 */

#include "definitions.h"
#include "config.h"
#include "strings.h"
#include "wlan.h"
#ifdef headunit
  #include "m365headunit.h"
#else
  #include "m365client.h"
#endif
#ifdef usedisplay
  #include "display.h"
#endif
#ifdef usengcode
  #include "ngcode.h"
#endif
#ifdef usetelnetserver
  #include "telnet.h"
#endif

#ifdef usestatusled
  int ledontime = 0;
  int ledofftime = 10000;
  unsigned long ledcurrenttime = 100;
#endif

//generic temp  vars
  uint8_t i;
  char tmp1[200];
  char tmp2[200];

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
  unsigned long timestamp_showsplashscreen=0;

HardwareSerialPatched m365uart(2);

boolean pcflag = true;

#ifdef headunit
  M365Headunit *ptrm365bus = new M365Headunit();
#else
  M365Client *ptrm365bus = new M365Client();
#endif
#ifdef usedisplay
  M365Display *ptrm365d = new M365Display();
#endif

/*
//define
      #define throttlemindefault 40
      #define throttlemaxdefault 190
      uint8_t throttlemin = throttlemindefault;
      uint8_t throttlemax = throttlemaxdefault;


//setup:
  

//loop: if newdata from "x1"
  from updatestats:
    1st block
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
    2nd block:
      if (alert_lockedalarm)
          alertpopup((char*)"ALARM", (char*)"SCOOTER LOCKED!", popupalertduration);
      }
  



*/
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
    if (ptrm365bus->m365receiverstate!=ptrm365bus->m365receiverstateold) {
      DebugSerial.printf("### M365RecState: %d -> %d\r\n",ptrm365bus->m365receiverstateold,ptrm365bus->m365receiverstate);
      ptrm365bus->m365receiverstateold=ptrm365bus->m365receiverstate;
    }
    if (ptrm365bus->m365packetstate!=ptrm365bus->m365packetstateold) {
      DebugSerial.printf("M365PacketState: %d -> %d\r\n",ptrm365bus->m365packetstateold,ptrm365bus->m365packetstate);
      ptrm365bus->m365packetstateold=ptrm365bus->m365packetstate;
    }    
  } //print_states
#endif

void printconfiguration() {
  DebugSerial.println("Using ESP32 Configuration:");
  #ifdef usengfeatuart
    DebugSerial.printf("# UART Mode: IO/1/2: %d/%d/%d\r\n",M365SerialGPIO, M365SerialSpareRX, M365SerialSpareTX);
  #else
    DebugSerial.printf("# UART Mode: RX/TX: %d/%d\r\n",UART2RX, UART2TX);
  #endif
  #ifdef usedisplay
    DebugSerial.println("# Display Configuration"); 
      #ifdef LANGUAGE_EN
        DebugSerial.println("#  Language: English");
      #endif
      #ifdef LANGUAGE_FR
        DebugSerial.println("#  Language: French");
      #endif
      #ifdef LANGUAGE_DE
        DebugSerial.println("#  Language: German");
      #endif
      #ifdef usei2c
        DebugSerial.printf("#  Mode: I2C, SDA: %d, SCL: %d, Reset: %d, doReset: %d, Clock: %lu\r\n", oled_sda, oled_scl,oled_reset, oled_doreset, oled_clock);
        #ifdef useoled1
          DebugSerial.printf("#  display1: Address: 0x%02x, Rotation: %d\r\n", oled1_address, OLED1_ROTATION);
        #endif
        #ifdef useoled2
          DebugSerial.printf("#  display2: Address: 0x%02x, Rotation: %d\r\n", oled2_address, OLED2_ROTATION);
        #endif
      #else
        DebugSerial.printf("#  Mode: SPI, MiSo: %d, MoSi: %d, Clk: %d, DC: %d, Clock: %lu\r\n", OLED_MISO,OLED_MOSI,OLED_CLK,OLED_DC,oled_clock);
        #ifdef useoled1
          DebugSerial.printf("#  display1: CS: %d, Rst: %d, Rotation: %d\r\n", OLED1_CS, OLED1_RESET, OLED1_ROTATION);
        #endif
        #ifdef useoled2
          DebugSerial.printf("#  display2: CS: %d, Rst: %d, Rotation: %d\r\n", OLED2_CS, OLED2_RESET, OLED2_ROTATION);
        #endif
      #endif
    DebugSerial.printf("#  Display Core %d, Prio %d\r\n",core_display,prio_display);
  #else
    DebugSerial.println("# Display disabled"); 
  #endif    
  #ifdef headunit
    DebugSerial.println("# Controller Config:");
    DebugSerial.printf("#  UART Core %d, Prio %d\r\n",core_uart,prio_uart);
    DebugSerial.printf("#  Main Core %d, Prio %d\r\n",xPortGetCoreID(), uxTaskPriorityGet(NULL));
    DebugSerial.printf("#  WiFi Core %d, Prio %d\r\n",wificore, wifiprio);
    DebugSerial.printf("#  ADC GPIO: Gas: %d, Throttle %d\r\n", sensor_brake, sensor_throttle);
    DebugSerial.printf("#  ADC Min: %d, Max %d\r\n", adcmin, adcmax);
    DebugSerial.printf("#  ESC Min: %d, Max %d\r\n", sensormin, sensormax);
    #ifdef debuggpio1
      DebugSerial.printf("#  Debug GPIO1: %d, GPIO2: %d\r\n", debuggpio1, debuggpio2);
    #endif
  #else
    DebugSerial.println("# Controller disabled");
  #endif
  #ifdef debuguarttiming
    DebugSerial.printf("# UART Timing Debug Pin: %d\r\n", M365debugtx);
  #else
    DebugSerial.println("# UART Timing Debug disabled");
  #endif
  #ifdef usetelnetserver
    DebugSerial.printf("# TelnetServer @ TCP Port %d\r\n", telnet_port_statistics);
  #endif
  #ifdef usepacketserver
    DebugSerial.printf("# PacketServer @ TCP Port %d\r\n", telnet_port_packet);
  #endif
  #ifdef userawserver
    DebugSerial.printf("# RAWServer @ TCP Port %d\r\n", telnet_port_raw);
  #endif
  DebugSerial.println("\r\nUsing M365 Configuration:");
  printconfig();
  DebugSerial.println("");
  DebugSerial.println("\r\nDebug NVS Config:");
  printnvsconfig();
  DebugSerial.println("");


}

void setup() {
  loadconfig();
  DebugSerial.begin(115200);
  m365uart.begin(115200,SERIAL_8N1, UART2RX, UART2TX);
  #ifdef usedisplay
    ptrm365d->begin(ptrm365bus);
  #endif
  #ifdef usengcode
    init_ng();
  #else
    DebugSerial.printf("\r\n\r\n\r\nESP32 M365 %s\r\n",swversion);
  #endif
  #ifdef usedisplay
    ptrm365bus->begin(&m365uart, ptrm365d); //start_m365();
  #else
    ptrm365bus->begin(&m365uart); //start_m365();
  #endif
  #ifdef usestatusled
    pinMode(led,OUTPUT);
    digitalWrite(led,LOW);
  #endif
  //printconfiguration();
  if (conf_espwifionstart) {
    wlanstate=wlanturnstaon;
  }
  pcflag = true;
} //setup


void loop() {
  timestamp_mainloopstart=micros();
  handle_wlan();
  if (((wificore!=255) & (pcflag)) | !conf_espwifionstart) {
    printconfiguration();
    pcflag = false;
  }
  #ifdef usetelnetserver
    handle_telnet();
  #endif
  #ifdef usengcode
    loop_ng1();
  #endif
  ptrm365bus->loop();
  #ifdef usedisplay
    //ptrm365d->loop();
  #endif
  #ifdef usestatusled
    handle_led();
  #endif
  #ifdef debug_dump_states
    print_states();
  #endif
  duration_mainloop=micros()-timestamp_mainloopstart;
}
