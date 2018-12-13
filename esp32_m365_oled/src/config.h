#ifndef CONFIG_h
#define CONFIG_h

#include "definitions.h"

#include <Preferences.h>

extern Preferences preferences;

  #define wheelfact8km 1.0f
  #define wheelfact8miles  0.621371192f
  #define wheelfact10km    1.176470588f
  #define wheelfact10miles 0.731024932f

//scooter-config stuff
	extern uint8_t conf_wheelsize;
	extern uint8_t conf_battcells;
	extern uint8_t conf_alert_batt_celldiff;
	extern uint8_t conf_alert_batt_temp;
	extern uint8_t conf_alert_batt_voltage;
	extern uint8_t conf_alert_esc_temp;
	extern uint8_t conf_unit;
	extern bool conf_flashprotect;
	extern bool conf_espbusmode;
	extern bool conf_espwifionstart;
	extern bool conf_beeponalert;

	extern float wheelfact;

//display config
	extern bool conf_usedisplay;
	extern bool conf_usedisplay1;
	extern bool conf_usedisplay2;
	extern bool conf_display_i2cspi; //true = i2c, false = spi

	extern int8_t conf_display_i2c_gpio_data;
	extern int8_t conf_display_i2c_gpio_clock;
	extern int8_t conf_display_i2c_gpio_reset;
	extern bool conf_display_i2c_doreset;
	extern uint32_t conf_display_i2c_clockspeed;
	extern int8_t conf_display_spi_gpio_miso;
	extern int8_t conf_display_spi_gpio_mosi;
	extern int8_t conf_display_spi_gpio_clk;
	extern int8_t conf_display_spi_gpio_dc;
	extern uint32_t conf_display_spi_clockspeed;
	
	extern int8_t conf_display1_i2c_address;
	extern int8_t conf_display1_spi_gpio_cs;
	extern int8_t conf_display1_spi_gpio_reset;
	extern int8_t conf_display1_rotation;
	extern int8_t conf_display2_i2c_address;
	extern int8_t conf_display2_spi_gpio_cs;
	extern int8_t conf_display2_spi_gpio_reset;
	extern int8_t conf_display2_rotation;

//uart config
	extern int8_t conf_uart_gpio_rx;
	extern int8_t conf_uart_gpio_tx;

//misc config
	extern int8_t conf_led_gpio;
	extern int8_t conf_gpio_debug1;
	extern int8_t conf_gpio_debug2;


void applyconfig(void);
void loadconfig(void);
void saveconfig(void);
void printconfig(void);
void printnvsconfig(void);
 
#endif
