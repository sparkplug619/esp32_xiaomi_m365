#include "wlan.h"

uint8_t currentssidindex = 0;
uint8_t apnumclientsconnected=0;
uint8_t apnumclientsconnectedlast=0;

uint8_t wlanstate = 0;
uint8_t wlanstateold = 0;
unsigned long wlanconnecttimestamp = 0;

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
        WiFi.onEvent(WiFiEvent);
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
        WiFi.onEvent(WiFiEvent); //not really needed in STA mode
        WiFi.setHostname("M365OLEDESP32");
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
        #ifdef usetelnetserver
          if (telnetclient) telnetclient.stop();
        #endif
        #ifdef userawserver
          if (rawclient) rawclient.stop();
        #endif
        #ifdef usepacketserver
          if (packetclient) packetclient.stop();
        #endif
        ArduinoOTA.end();
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

