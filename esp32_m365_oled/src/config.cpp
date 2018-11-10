#include "config.h"

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
bool conf_beeponalert;

void applyconfig() {
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
  conf_beeponalert = preferences.getBool("alertbeep",true); //beep if a error occured?
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
  preferences.putBool("alertbeep",conf_beeponalert); //beep if a error occured? 
  preferences.putUChar("unit",conf_unit); //unit: 0 = kilmeters, 1 = miles
  preferences.end();
}