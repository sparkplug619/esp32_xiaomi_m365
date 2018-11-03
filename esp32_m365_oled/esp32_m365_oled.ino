/*  ESP32 M365 OLED Display
 * NG Branch
 * see readme.md for details
 *
 * configuration adoptions should be done in definitions.h
 * language adoptions in strings.h (i'm happy to add your strings_<language>.h to my repo)
 *
 * needs patch in esp32-arduino-core/esp32-hal-uart.c (112 byte hardware uart-receive buffer is to big for "realtime" communication)
 *  -> see comments in definitions.h / readme.md and patch yourself or use https://github.com/smartinick/arduino-esp32
 * needs patched Adafruit_SSD1306 Library (custom pins, higher clock speer)
 *  -> compare with base or clone from https://github.com/smartinick/Adafruit_SSD1306
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
#include "m365.h"
#include "display.h"

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
  DebugSerial.begin(115200);
  loadconfig();
  DebugSerial.println(swversion);
  wlanstate=wlanturnstaon;
  uint64_t cit;
  cit = ESP.getEfuseMac();
  sprintf(chipid,"%02X%02X%02X",(uint8_t)(cit>>24),(uint8_t)(cit>>32),(uint8_t)(cit>>40));
  DebugSerial.println(chipid);
  sprintf(mac,"%02X%02X%02X%02X%02X%02X",(uint8_t)(cit),(uint8_t)(cit>>8),(uint8_t)(cit>>16),(uint8_t)(cit>>24),(uint8_t)(cit>>32),(uint8_t)(cit>>40));
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
