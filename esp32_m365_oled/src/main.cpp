/*  ESP32 M365 OLED Display
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
#include "display.h"
#include "config.h"
#include "strings.h"
#include "wlan.h"
#include "m365.h"

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

  uint8_t i;
  char tmp1[200];
  char tmp2[200];
  uint16_t tmpi;
  char chipid[7];
  char mac[12];
  unsigned int lastprogress=0;

  const char *apssid = ap_ssid;
  const char *appassword= ap_password;


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


void setup() {
    init_displays();
  #ifdef usengcode
    init_ng();
  #else
    start_m365();
    DebugSerial.begin(115200);
    loadconfig();
    DebugSerial.println(swversion);
    #ifdef usestatusled
      pinMode(led,OUTPUT);
      digitalWrite(led,LOW);
    #endif
  #endif
  wlanstate=wlanturnstaon;
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
  #ifdef debug_dump_states
    print_states();
  #endif
  duration_mainloop=micros()-timestamp_mainloopstart;
}
