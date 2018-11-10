#ifndef WLAN_h
#define WLAN_h

#include "definitions.h"
#include "strings.h"
#include "m365.h"

#include <WiFi.h>
#include <WiFiUdp.h>
#include <Update.h> //needed for arduinoOTA on esp32
#include <ArduinoOTA.h>

extern uint8_t currentssidindex;
extern uint8_t apnumclientsconnected;
extern uint8_t apnumclientsconnectedlast;

extern uint8_t wlanstate;
extern uint8_t wlanstateold;

#define wlanoff 0
#define wlanturnapon 1
#define wlanturnstaon 2
#define wlansearching 3
#define wlanconnected 4
#define wlanap 5
#define wlanturnoff 6 


extern unsigned long wlanconnecttimestamp;

void WiFiEvent(WiFiEvent_t event);
void handle_wlan(void);

#endif
