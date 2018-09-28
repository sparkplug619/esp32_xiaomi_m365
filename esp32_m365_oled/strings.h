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

static const char unit_speed[] PROGMEM = "km/h";
static const char unit_distance[] PROGMEM = "km";
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
static const char label_bmstemp[] PROGMEM = "BMS Temp:"; //alert screen
static const char label_esctemp[] PROGMEM = "ESC Temp:"; //alert screen
static const char label_cellvoltage[] PROGMEM = "CellVoltDiff:"; //alert screen
static const char label_undervoltage[] PROGMEM = "UnderVoltage"; //alert screen

static const char menu_light[] PROGMEM = "Tail Light: ";
static const char menu_cruise[] PROGMEM = "Cruise Control: ";
static const char menu_kers[] PROGMEM = "KERS: ";
static const char menu_wheelsize[] PROGMEM = "Wheelsize: ";
static const char menu_unit[] PROGMEM = "Unit: ";
static const char menu_battcells[] PROGMEM = "Battery Cells: ";
static const char menu_battalertcell[] PROGMEM = "Cell Alert: ";
static const char menu_battalertlowvoltage[] PROGMEM = "Batt Alert Low Voltage";
static const char menu_battalerttemp[] PROGMEM = "Batt Temp Alert: ";
static const char menu_escalerttemp[] PROGMEM = "ESC Temp Alert: ";
static const char menu_espbusmode[] PROGMEM = "ESP Busmode: ";
static const char menu_espwifirestart[] PROGMEM = "ESP Restart Wifi";
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
static const char s_off[] PROGMEM = "OFF";
static const char s_bleoff[] PROGMEM = "BT: OFF";
static const char s_bmsfw[] PROGMEM = "BMS   FW: %x.%x.%x\r\nSN: %s\r\n";
static const char s_escfw[] PROGMEM = "ESC   FW: %x.%x.%x\r\nSN: %s\r\n";
static const char s_pin[] PROGMEM = "Pin: %s\r\n";
static const char s_miles[] PROGMEM = "Miles: %.2f km\r\n";
static const char s_battdate[] PROGMEM = "Batt-Date: 20%02d-%02d-%02d";
static const char s_locked[] PROGMEM = "LOCKED";
static const char s_timeout1[] PROGMEM = "NO";
static const char s_timeout2[] PROGMEM = "DATA";
static const char s_timeout3[] PROGMEM = "TIMEOUT";
