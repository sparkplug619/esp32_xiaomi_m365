#ifndef CONFIG_h
#define CONFIG_h

#include "definitions.h"
#include "m365.h"

#include <Preferences.h>

extern Preferences preferences;
extern uint8_t conf_wheelsize;
extern uint8_t conf_battcells;
extern uint8_t conf_alert_batt_celldiff;
extern uint8_t conf_alert_batt_temp;
extern uint8_t conf_alert_batt_voltage;
extern uint8_t conf_alert_esc_temp;
extern uint8_t conf_unit;
extern bool conf_flashprotect;
extern bool conf_espbusmode;
extern bool conf_beeponalert;

void applyconfig(void);
void loadconfig(void);
void saveconfig(void);
 
#endif
