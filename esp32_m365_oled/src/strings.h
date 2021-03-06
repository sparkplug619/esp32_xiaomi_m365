#ifndef STRINGS_h
#define STRINGS_h

#include "definitions.h"

#ifdef LANGUAGE_EN
	static const char headline_test[] PROGMEM = "SCREEN HEADLINE";
	static const char headline_tripinfo[] PROGMEM = "TRIP Info";
	static const char headline_temperature[] PROGMEM = "TEMPERATURE";
	static const char headline_battery1[] PROGMEM = "BATTERY 1"; //headline for single screen
	static const char headline_battery2[] PROGMEM = "BATTERY 2"; //headline for single screen
	static const char headline_batterystatus[] PROGMEM = "BATTERY STATUS"; //headline for dual screen
	static const char headline_cellvolt[] PROGMEM = "CELL VOLTAGES";
	static const char headline_assests[] PROGMEM = "Assets";
	static const char headline_espstate[] PROGMEM = "ESP State";
	static const char headline_alerts[] PROGMEM = "ALARM COUNTERS";
	static const char headline_charging[] PROGMEM = "CHARGING";
	static const char headline_configmenu[] PROGMEM = "CONFIG MENU";

	static const char unit_speed_km[] PROGMEM = "kmh";
	static const char unit_speed_miles[] PROGMEM = "mph";
	static const char unit_distance_km[] PROGMEM = "km";
	static const char unit_distance_miles[] PROGMEM = "mi";
	static const char unit_time[] PROGMEM = "s";
	static const char unit_volt[] PROGMEM = "V";
	static const char unit_current[] PROGMEM = "A";
	static const char unit_power[] PROGMEM = "W";
	static const char unit_temp[] PROGMEM = "C";
	static const char unit_percent[] PROGMEM = "%";
	static const char unit_count[] PROGMEM = "#";
	static const char unit_mah[] PROGMEM = "mAh";

	static const char label_volt[] PROGMEM = "Volt:";
	static const char label_percent[] PROGMEM = "Percent:";
	static const char label_current[] PROGMEM = "Current:";
	static const char label_temp[] PROGMEM = "Temperature:";
	static const char label_tempshort[] PROGMEM = "Temp:";
	static const char label_averageshort[] PROGMEM = "Avg:";
	static const char label_distanceshort[] PROGMEM = "Dist:";
	static const char label_time[] PROGMEM = "Time:";
	static const char label_seconds[] PROGMEM = "s";
	static const char label_remainingshort[] PROGMEM = "Rem:";
	static const char label_batt1[] PROGMEM = "Batt 1:";
	static const char label_batt2[] PROGMEM = "Batt 2:";
	static const char label_esc[] PROGMEM = "ESC:";
	static const char label_cycles[] PROGMEM = "Cycles:";
	static const char label_charges[] PROGMEM = "Charges:";
	static const char label_health[] PROGMEM = "Health:";
	static const char label_capacityshort[] PROGMEM = "Cap:";
	static const char label_totalcapacityshort[] PROGMEM = "Total:";
	static const char label_maxdiff[] PROGMEM = "Max. Diff:"; //Cell Voltage Screen, max Difference between cells
	static const char label_bmstemp[] PROGMEM = "BMS Tmp:"; //alert screen
	static const char label_esctemp[] PROGMEM = "ESC Tmp:"; //alert screen
	static const char label_cellvoltage[] PROGMEM = "Cell Diff:"; //alert screen
	static const char label_undervoltage[] PROGMEM = "Low Volt:"; //alert screen
	static const char label_escerrorcounter[] PROGMEM = "ESC Err:"; //alert screen
	static const char label_alertcounter[] PROGMEM = "Lock-Alert:"; //alert screen

	static const char menu_light[] PROGMEM = "Tail Light";
	static const char menu_cruise[] PROGMEM = "Cruise Control";
	static const char menu_kers[] PROGMEM = "KERS";
	static const char menu_wheelsize[] PROGMEM = "Wheelsize";
	static const char menu_unit[] PROGMEM = "Unit";
	static const char menu_battcells[] PROGMEM = "Battery Cells";
	static const char menu_battalertcell[] PROGMEM = "Cell Alert";
	static const char menu_battalertlowvoltage[] PROGMEM = "Batt Low Volt";
	static const char menu_battalerttemp[] PROGMEM = "Batt Temp Alert";
	static const char menu_escalerttemp[] PROGMEM = "ESC Temp Alert";
	static const char menu_espbusmode[] PROGMEM = "UART Busmode";
	static const char menu_espwifirestart[] PROGMEM = "Restart Wifi";
	static const char menu_espwifionstart[] PROGMEM = "Wifi on Start";
	
	static const char menu_m365lock[] PROGMEM = "M365 LOCK";
	static const char menu_m365unlock[] PROGMEM = "M365 UNLOCK";
	static const char menu_m365turnoff[] PROGMEM = "M365 Turn OFF";
	static const char menu_exit[] PROGMEM = "Exit";
	static const char menu_on[] PROGMEM = "ON";
	static const char menu_off[] PROGMEM = "OFF";
	static const char menu_weak[] PROGMEM = "weak";
	static const char menu_medium[] PROGMEM = "medium";
	static const char menu_strong[] PROGMEM = "strong";
	static const char menu_km[] PROGMEM = "km";
	static const char menu_miles[] PROGMEM = "miles";
	static const char menu_active[] PROGMEM = "active";
	static const char menu_passive[] PROGMEM = "passive";
	static const char menu_beeponalert[] PROGMEM = "Beep on Alert";
	static const char menu_scooter[] PROGMEM = "Scooter";
	static const char menu_alerts[] PROGMEM = "Alerts";
	static const char menu_esp[] PROGMEM = "ESP";

	static const char error_error[] PROGMEM = "ERROR";
	static const char error_10[] PROGMEM = "BLE Communication";
	static const char error_11[] PROGMEM = "Mot Phase A Current";
	static const char error_12[] PROGMEM = "Mot Phase B Current";
	static const char error_13[] PROGMEM = "Mot Phase C Current";
	static const char error_14[] PROGMEM = "THROTTLE SENSOR";
	static const char error_15[] PROGMEM = "BRAKE SENSOR";
	static const char error_18[] PROGMEM = "MOTOR HALL SENSOR";
	static const char error_21[] PROGMEM = "BMS Communication";
	static const char error_22[] PROGMEM = "Bad BMS Password";
	static const char error_23[] PROGMEM = "BMS Serialnumber";
	static const char error_24[] PROGMEM = "VOLTAGE WRONG";
	static const char error_26[] PROGMEM = "EEPROM/FLASH CRC";
	static const char error_27[] PROGMEM = "Bad ESC Password";
	static const char error_28[] PROGMEM = "hS FET Error";
	static const char error_29[] PROGMEM = "lS FET Error";
	static const char error_31[] PROGMEM = "Program Error";
	static const char error_35[] PROGMEM = "ESC Serial";
	static const char error_36[] PROGMEM = "ESC Activation";
	static const char error_39[] PROGMEM = "BATT TEMP ALERT";
	static const char error_40[] PROGMEM = "ESC TEMP ALERT";
	static const char error_other[] PROGMEM = "unknown code";

	static const char s_firmware[] PROGMEM = "Firmware: ";
	static const char s_wlan[] PROGMEM = "WLAN: ";
	static const char s_wlanoff[] PROGMEM = "WLAN OFF";
	static const char s_wlansearch[] PROGMEM = "searching...";
	static const char s_wlancon[] PROGMEM = "Connected\r\nSSID: ";
	static const char s_wlanap[] PROGMEM = "AP Mode\r\nSSID: ";
	static const char s_wlansearchshort[] PROGMEM = "WS";
	static const char s_wlanconshort[] PROGMEM = "WC";
	static const char s_wlanapshort[] PROGMEM = "WA";

	static const char s_off[] PROGMEM = "OFF";
	static const char s_bleoff[] PROGMEM = "BT: OFF";
	static const char s_bmsfw[] PROGMEM = "BMS   FW: %x.%x.%x\r\nSN: %s\r\n";
	static const char s_escfw[] PROGMEM = "ESC   FW: %x.%x.%x\r\nSN: %s\r\n";
	static const char s_pin[] PROGMEM = "Pin: %s\r\n";
	static const char s_miles[] PROGMEM = "Tot. Dist.: %.2f km\r\n";
	static const char s_battdate[] PROGMEM = "Batt-Date: 20%02d-%02d-%02d";
	static const char s_locked[] PROGMEM = "LOCKED";
	static const char s_alert[] PROGMEM = "ALERT";
	static const char s_timeout1[] PROGMEM = "NO";
	static const char s_timeout2[] PROGMEM = "DATA";
	static const char s_timeout3[] PROGMEM = "TIMEOUT";
	static const char s_blank[] PROGMEM = " ";
	static const char s_charging[] PROGMEM = "Charging";
	static const char s_charged[] PROGMEM = "charged";
	static const char s_settemp[] PROGMEM = "set Temp:";
	static const char s_setvolt[] PROGMEM = "set Volt:";
#endif //en

//french translation kindly provided by Technoo' Loggie (Telegram Handle @TechnooLoggie)
#ifdef LANGUAGE_FR
	static const char headline_test[] PROGMEM = "TITRE D'ECRAN";
	static const char headline_tripinfo[] PROGMEM = "INFO GENERALES";
	static const char headline_temperature[] PROGMEM = "TEMPERATURE";
	static const char headline_battery1[] PROGMEM = "BATTERIE 1"; //headline for single screen
	static const char headline_battery2[] PROGMEM = "BATTERIE 2"; //headline for single screen
	static const char headline_batterystatus[] PROGMEM = "BATTERIE STATUT"; //headline for dual screen
	static const char headline_cellvolt[] PROGMEM = "CELL VOLTAGES";
	static const char headline_assests[] PROGMEM = "RECAPITULATIF";
	static const char headline_espstate[] PROGMEM = "ESP32";
	static const char headline_alerts[] PROGMEM = "NBRE D'ALARME";
	static const char headline_charging[] PROGMEM = "CHARGEMENT";
	static const char headline_configmenu[] PROGMEM = "CONFIF MENU";

	static const char unit_speed_km[] PROGMEM = "kmh";
	static const char unit_speed_miles[] PROGMEM = "mph";
	static const char unit_distance_km[] PROGMEM = "km";
	static const char unit_distance_miles[] PROGMEM = "mi";
	static const char unit_time[] PROGMEM = "s";
	static const char unit_volt[] PROGMEM = "V";
	static const char unit_current[] PROGMEM = "A";
	static const char unit_power[] PROGMEM = "W";
	static const char unit_temp[] PROGMEM = "C";
	static const char unit_percent[] PROGMEM = "%";
	static const char unit_count[] PROGMEM = "#";
	static const char unit_mah[] PROGMEM = "mAh";

	static const char label_volt[] PROGMEM = "Volt:";
	static const char label_percent[] PROGMEM = "Pourcent:";
	static const char label_current[] PROGMEM = "Courant:";
	static const char label_temp[] PROGMEM = "Temperature:";
	static const char label_tempshort[] PROGMEM = "Temp:";
	static const char label_averageshort[] PROGMEM = "Moy:";
	static const char label_distanceshort[] PROGMEM = "Dist:";
	static const char label_time[] PROGMEM = "Temps:";
	static const char label_seconds[] PROGMEM = "s";
	static const char label_remainingshort[] PROGMEM = "Restant:";
	static const char label_batt1[] PROGMEM = "Batt 1:";
	static const char label_batt2[] PROGMEM = "Batt 2:";
	static const char label_esc[] PROGMEM = "ESC:";
	static const char label_cycles[] PROGMEM = "Cycles:";
	static const char label_charges[] PROGMEM = "Charges:";
	static const char label_health[] PROGMEM = "Vie:";
	static const char label_capacityshort[] PROGMEM = "Cap:";
	static const char label_totalcapacityshort[] PROGMEM = "Total:";
	static const char label_maxdiff[] PROGMEM = "Max. Diff:"; //Cell Voltage Screen, max Difference between cells
	static const char label_bmstemp[] PROGMEM = "BMS Tmp:"; //alert screen
	static const char label_esctemp[] PROGMEM = "ESC Tmp:"; //alert screen
	static const char label_cellvoltage[] PROGMEM = "Cell Diff:"; //alert screen
	static const char label_undervoltage[] PROGMEM = "Tension Basse"; //alert screen
	static const char label_escerrorcounter[] PROGMEM = "ESC Err:"; //alert screen
	static const char label_alertcounter[] PROGMEM = "Lock-Alert:"; //alert screen TODO TRANSLATE

	static const char menu_light[] PROGMEM = "Lumiere avant";
	static const char menu_cruise[] PROGMEM = "Regulat vitesse";
	static const char menu_kers[] PROGMEM = "KERS";
	static const char menu_wheelsize[] PROGMEM = "Taille de Roue";
	static const char menu_unit[] PROGMEM = "Unite";
	static const char menu_battcells[] PROGMEM = "Batterie Cell";
	static const char menu_battalertcell[] PROGMEM = "Cell Alert";
	static const char menu_battalertlowvoltage[] PROGMEM = "Batt Volt Bas";
	static const char menu_battalerttemp[] PROGMEM = "Batt Alert Temp";
	static const char menu_escalerttemp[] PROGMEM = "ESC Alert Temp";
	static const char menu_espbusmode[] PROGMEM = "ESP Busmode";
	static const char menu_espwifirestart[] PROGMEM = "Demarrage Wifi";
	static const char menu_espwifionstart[] PROGMEM = "Wifi on Start";
	static const char menu_m365lock[] PROGMEM = "VEROUILLE";
	static const char menu_m365unlock[] PROGMEM = "DEVEROUILLE";
	static const char menu_m365turnoff[] PROGMEM = "Eteindre";
	static const char menu_exit[] PROGMEM = "Sortir";
	static const char menu_on[] PROGMEM = "ON";
	static const char menu_off[] PROGMEM = "OFF";
	static const char menu_weak[] PROGMEM = "faible";
	static const char menu_medium[] PROGMEM = "moyen";
	static const char menu_strong[] PROGMEM = "fort";
	static const char menu_km[] PROGMEM = "km";
	static const char menu_miles[] PROGMEM = "miles";
	static const char menu_active[] PROGMEM = "actif";
	static const char menu_passive[] PROGMEM = "passif";
	static const char menu_beeponalert[] PROGMEM = "Bip sur Alert";
	static const char menu_scooter[] PROGMEM = "Scooter";
	static const char menu_alerts[] PROGMEM = "Alerts";
	static const char menu_esp[] PROGMEM = "ESP";

	static const char error_error[] PROGMEM = "ERREUR";
	static const char error_10[] PROGMEM = "BLE Communication";
	static const char error_11[] PROGMEM = "Mot Phase A Courant";
	static const char error_12[] PROGMEM = "Mot Phase B Courant";
	static const char error_13[] PROGMEM = "Mot Phase C Courant";
	static const char error_14[] PROGMEM = "ACCELERATEUR SENSOR";
	static const char error_15[] PROGMEM = "FREIN SENSOR";
	static const char error_18[] PROGMEM = "MOTEUR HALL SENSOR";
	static const char error_21[] PROGMEM = "BMS Communication";
	static const char error_22[] PROGMEM = "Mauvais BMS Password";
	static const char error_23[] PROGMEM = "BMS Serialnumber";
	static const char error_24[] PROGMEM = "MAUVAISE TENSION";
	static const char error_26[] PROGMEM = "EEPROM/FLASH CRC";
	static const char error_27[] PROGMEM = "Mauvais ESC Password";
	static const char error_28[] PROGMEM = "hS FET Erreur";
	static const char error_29[] PROGMEM = "lS FET Erreur";
	static const char error_31[] PROGMEM = "Program Erreur";
	static const char error_35[] PROGMEM = "ESC Serial";
	static const char error_36[] PROGMEM = "ESC Activation";
	static const char error_39[] PROGMEM = "BATT ALERT TEMP";
	static const char error_40[] PROGMEM = "ESC ALERT TEMP";
	static const char error_other[] PROGMEM = "code non connu";

	static const char s_firmware[] PROGMEM = "Firmware: ";
	static const char s_wlan[] PROGMEM = "Wifi: ";
	static const char s_wlanoff[] PROGMEM = "Wifi OFF";
	static const char s_wlansearch[] PROGMEM = "recherche...";
	static const char s_wlancon[] PROGMEM = "Connecte\r\nSSID: ";
	static const char s_wlanap[] PROGMEM = "AP Mode\r\nSSID: ";
	static const char s_wlansearchshort[] PROGMEM = "WS";
	static const char s_wlanconshort[] PROGMEM = "WC";
	static const char s_wlanapshort[] PROGMEM = "WA";

	static const char s_off[] PROGMEM = "OFF";
	static const char s_bleoff[] PROGMEM = "BT: OFF";
	static const char s_bmsfw[] PROGMEM = "BMS   FW: %x.%x.%x\r\nSN: %s\r\n";
	static const char s_escfw[] PROGMEM = "ESC   FW: %x.%x.%x\r\nSN: %s\r\n";
	static const char s_pin[] PROGMEM = "Pin: %s\r\n";
	static const char s_miles[] PROGMEM = "Tot. Dist.: %.2f km\r\n";
	static const char s_battdate[] PROGMEM = "Batt-Date: 20%02d-%02d-%02d";
	static const char s_locked[] PROGMEM = "VEROUILLE";
	static const char s_alert[] PROGMEM = "ALARM"; //TODO TRANSLATE
	static const char s_timeout1[] PROGMEM = "NO";
	static const char s_timeout2[] PROGMEM = "DONNEES";
	static const char s_timeout3[] PROGMEM = "TIMEOUT";
	static const char s_blank[] PROGMEM = " ";
	static const char s_charging[] PROGMEM = "Chargement";
	static const char s_charged[] PROGMEM = "charge";
	static const char s_settemp[] PROGMEM = "entrer Temp:";
	static const char s_setvolt[] PROGMEM = "entrer Volt:";
#endif

#ifdef LANGUAGE_DE
	static const char headline_test[] PROGMEM = "SCREEN HEADLINE";
	static const char headline_tripinfo[] PROGMEM = "TRIP Info";
	static const char headline_temperature[] PROGMEM = "TEMPERATUR";
	static const char headline_battery1[] PROGMEM = "BATTERIE 1"; //headline for single screen
	static const char headline_battery2[] PROGMEM = "BATTERIE 2"; //headline for single screen
	static const char headline_batterystatus[] PROGMEM = "BATTERIE STATUS"; //headline for dual screen
	static const char headline_cellvolt[] PROGMEM = "ZELL SPANNUNG";
	static const char headline_assests[] PROGMEM = "M365 INFOS";
	static const char headline_espstate[] PROGMEM = "ESP Status";
	static const char headline_alerts[] PROGMEM = "ALARME";
	static const char headline_charging[] PROGMEM = "LADE AKKU";
	static const char headline_configmenu[] PROGMEM = "EINSTELLUNGEN";

	static const char unit_speed_km[] PROGMEM = "kmh";
	static const char unit_speed_miles[] PROGMEM = "mph";
	static const char unit_distance_km[] PROGMEM = "km";
	static const char unit_distance_miles[] PROGMEM = "mi";
	static const char unit_time[] PROGMEM = "s";
	static const char unit_volt[] PROGMEM = "V";
	static const char unit_current[] PROGMEM = "A";
	static const char unit_power[] PROGMEM = "W";
	static const char unit_temp[] PROGMEM = "C";
	static const char unit_percent[] PROGMEM = "%";
	static const char unit_count[] PROGMEM = "#";
	static const char unit_mah[] PROGMEM = "mAh";

	static const char label_volt[] PROGMEM = "Spannung:";
	static const char label_percent[] PROGMEM = "Prozent:";
	static const char label_current[] PROGMEM = "Strom:";
	static const char label_temp[] PROGMEM = "Temperatur:";
	static const char label_tempshort[] PROGMEM = "Temp:";
	static const char label_averageshort[] PROGMEM = "Schnitt:";
	static const char label_distanceshort[] PROGMEM = "Strecke:";
	static const char label_time[] PROGMEM = "Zeit:";
	static const char label_seconds[] PROGMEM = "s";
	static const char label_remainingshort[] PROGMEM = "Reichw.:";
	static const char label_batt1[] PROGMEM = "Batt 1:";
	static const char label_batt2[] PROGMEM = "Batt 2:";
	static const char label_esc[] PROGMEM = "ESC:";
	static const char label_cycles[] PROGMEM = "Zyklen:";
	static const char label_charges[] PROGMEM = "Ladungen:";
	static const char label_health[] PROGMEM = "Status:";
	static const char label_capacityshort[] PROGMEM = "Kapazit.:";
	static const char label_totalcapacityshort[] PROGMEM = "Gesamt:";
	static const char label_maxdiff[] PROGMEM = "Max. Diff:"; //Cell Voltage Screen, max Difference between cells
	static const char label_bmstemp[] PROGMEM = "BMS Tmp:"; //alert screen
	static const char label_esctemp[] PROGMEM = "ESC Tmp:"; //alert screen
	static const char label_cellvoltage[] PROGMEM = "Zell Diff:"; //alert screen
	static const char label_undervoltage[] PROGMEM = "Batt Leer:"; //alert screen
	static const char label_escerrorcounter[] PROGMEM = "ESC Fehler:"; //alert screen
	static const char label_alertcounter[] PROGMEM = "Alarms:"; //alert screen

	static const char menu_light[] PROGMEM = "Ruecklicht";
	static const char menu_cruise[] PROGMEM = "Tempomat";
	static const char menu_kers[] PROGMEM = "KERS";
	static const char menu_wheelsize[] PROGMEM = "Reifen";
	static const char menu_unit[] PROGMEM = "Einheit";
	static const char menu_battcells[] PROGMEM = "Batterie Zellen";
	static const char menu_battalertcell[] PROGMEM = "Alarm Zellen";
	static const char menu_battalertlowvoltage[] PROGMEM = "Alarm Spannung";
	static const char menu_battalerttemp[] PROGMEM = "Alarm Temp Batt";
	static const char menu_escalerttemp[] PROGMEM = "Alarm Temp Mot";
	static const char menu_espbusmode[] PROGMEM = "UART Modus";
	static const char menu_espwifirestart[] PROGMEM = "Wifi Start";
	static const char menu_espwifionstart[] PROGMEM = "Wifi Auto An";
	static const char menu_m365lock[] PROGMEM = "SPERREN";
	static const char menu_m365unlock[] PROGMEM = "ENTSPERREN";
	static const char menu_m365turnoff[] PROGMEM = "AUSSCHALTEN";
	static const char menu_exit[] PROGMEM = "ENDE";
	static const char menu_on[] PROGMEM = "AN";
	static const char menu_off[] PROGMEM = "AUS";
	static const char menu_weak[] PROGMEM = "schwach";
	static const char menu_medium[] PROGMEM = "mittel";
	static const char menu_strong[] PROGMEM = "stark";
	static const char menu_km[] PROGMEM = "km";
	static const char menu_miles[] PROGMEM = "meilen";
	static const char menu_active[] PROGMEM = "aktiv";
	static const char menu_passive[] PROGMEM = "passiv";
	static const char menu_beeponalert[] PROGMEM = "Ton bei Alarm";
	static const char menu_scooter[] PROGMEM = "Scooter";
	static const char menu_alerts[] PROGMEM = "Alarme";
	static const char menu_esp[] PROGMEM = "ESP";

	static const char error_error[] PROGMEM = "FEHLER";
	static const char error_10[] PROGMEM = "BLE Kommunikation";
	static const char error_11[] PROGMEM = "Motor Phase A Strom";
	static const char error_12[] PROGMEM = "Motor Phase B Strom";
	static const char error_13[] PROGMEM = "Motor Phase C Strom";
	static const char error_14[] PROGMEM = "GAS SENSOR";
	static const char error_15[] PROGMEM = "BREMS SENSOR";
	static const char error_18[] PROGMEM = "MOTOR HALL SENSOR";
	static const char error_21[] PROGMEM = "BMS Kommunikation";
	static const char error_22[] PROGMEM = "BMS Kennwort falsch";
	static const char error_23[] PROGMEM = "BMS Seriennummer";
	static const char error_24[] PROGMEM = "SPANNUNGSFEHLER";
	static const char error_26[] PROGMEM = "EEPROM/FLASH CRC";
	static const char error_27[] PROGMEM = "ESC Kennwort falsch";
	static const char error_28[] PROGMEM = "hS FET Fehler";
	static const char error_29[] PROGMEM = "lS FET Fehler";
	static const char error_31[] PROGMEM = "Program Fehler";
	static const char error_35[] PROGMEM = "ESC Seriennummer";
	static const char error_36[] PROGMEM = "ESC Aktivierung";
	static const char error_39[] PROGMEM = "BATT TEMP ALARM";
	static const char error_40[] PROGMEM = "ESC TEMP ALARM";
	static const char error_other[] PROGMEM = "unbekannter Fehler";

	static const char s_firmware[] PROGMEM = "Firmware: ";
	static const char s_wlan[] PROGMEM = "WLAN: ";
	static const char s_wlanoff[] PROGMEM = "WLAN AUS";
	static const char s_wlansearch[] PROGMEM = "suche...";
	static const char s_wlancon[] PROGMEM = "Verbunden\r\nSSID: ";
	static const char s_wlanap[] PROGMEM = "AP Modus\r\nSSID: ";
	static const char s_wlansearchshort[] PROGMEM = "WS";
	static const char s_wlanconshort[] PROGMEM = "WC";
	static const char s_wlanapshort[] PROGMEM = "WA";

	static const char s_off[] PROGMEM = "AUS";
	static const char s_bleoff[] PROGMEM = "BT: AUS";
	static const char s_bmsfw[] PROGMEM = "BMS   FW: %x.%x.%x\r\nSN: %s\r\n";
	static const char s_escfw[] PROGMEM = "ESC   FW: %x.%x.%x\r\nSN: %s\r\n";
	static const char s_pin[] PROGMEM = "Pin: %s\r\n";
	static const char s_miles[] PROGMEM = "Kilometer: %.2f km\r\n";
	static const char s_battdate[] PROGMEM = "Batt Datum:20%02d-%02d-%02d";
	static const char s_locked[] PROGMEM = "GESPERRT";
	static const char s_alert[] PROGMEM = "ALARM";
	static const char s_timeout1[] PROGMEM = "KEINE";
	static const char s_timeout2[] PROGMEM = "DATEN";
	static const char s_timeout3[] PROGMEM = "TIMEOUT";
	static const char s_blank[] PROGMEM = " ";
	static const char s_charging[] PROGMEM = "Lade";
	static const char s_charged[] PROGMEM = "geladen";
	static const char s_settemp[] PROGMEM = "Temperatur:";
	static const char s_setvolt[] PROGMEM = "Spannung:";
#endif //de
#endif
