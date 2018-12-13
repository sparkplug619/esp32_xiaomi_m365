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
bool conf_espwifionstart;
bool conf_beeponalert;
//display config
	bool conf_usedisplay;     //true to globally enable displays, not stored but derived from usedisplay1/2 -> not shown in web
  bool conf_usedisplay1;    //true to enable display1
	bool conf_usedisplay2;    //true to enable display2
	bool conf_display_i2cspi; //true = i2c, false = spi

	int8_t conf_display_i2c_gpio_data; //gpio number for i2c data, only used then display_i2cspi=true
	int8_t conf_display_i2c_gpio_clock; //gpio number for i2c clock, only used then display_i2cspi=true
	int8_t conf_display_i2c_gpio_reset; //gpio number for display reset pin, only used then display_i2cspi=true
	bool conf_display_i2c_doreset; //shall resetpin be used on display-init?
	uint32_t conf_display_i2c_clockspeed; //clockspeed in Hz for i2c
	int8_t conf_display_spi_gpio_miso; 
	int8_t conf_display_spi_gpio_mosi; //gpio number for spi mosi, only used then display_i2cspi=false
	int8_t conf_display_spi_gpio_clk; //gpio number for spi clock, only used then display_i2cspi=false
	int8_t conf_display_spi_gpio_dc; //gpio number for display data/command mode, only used then display_i2cspi=false
	uint32_t conf_display_spi_clockspeed; //clockspeed in Hz for spi
	
	int8_t conf_display1_i2c_address; //i2c address of display1, only used then display_i2cspi=true
	int8_t conf_display1_spi_gpio_cs; //gpio number for display1 spi chipselect, only used then display_i2cspi=false
	int8_t conf_display1_spi_gpio_reset; //gpio number for display1 spi reset, only used then display_i2cspi=false
	int8_t conf_display1_rotation; //rotation of display1 0-3 = 90° steps from 0 to 270
	int8_t conf_display2_i2c_address; //i2c address of display2, only used then display_i2cspi=true
	int8_t conf_display2_spi_gpio_cs; //gpio number for display2 spi chipselect, only used then display_i2cspi=false
	int8_t conf_display2_spi_gpio_reset; //gpio number for display2 spi reset, only used then display_i2cspi=false
	int8_t conf_display2_rotation; //rotation of display2 0-3 = 90° steps from 0 to 270

//uart config
	int8_t conf_uart_gpio_rx; //gpio number for uart rx, mandatory
	int8_t conf_uart_gpio_tx; //gpio number for uart tx, mandatory

//misc config
	int8_t conf_led_gpio; //gpio number for status led, -1 if unused
	int8_t conf_gpio_debug1; //gpio number for debug1 pin, -1 if unused
	int8_t conf_gpio_debug2; //gpio number for debug2 pin, -1 if unused


float wheelfact = 0;

void applyconfig() {
  if ((conf_wheelsize==0) & (conf_unit==0)) { wheelfact = wheelfact8km; } //8" and kilometers
  if ((conf_wheelsize==0) & (conf_unit==1)) { wheelfact = wheelfact8miles; } //8" and miles
  if ((conf_wheelsize==1) & (conf_unit==0)) { wheelfact = wheelfact10km; } //10" and kilometers
  if ((conf_wheelsize==1) & (conf_unit==1)) { wheelfact = wheelfact10miles; } //10" and miles
}

void loadconfig() {
  preferences.begin("espm365config", false); //open in rw mode
  conf_wheelsize = preferences.getUChar("wheelsize", 0); //0=8.5", 1=10"
  conf_battcells = preferences.getUChar("battcells", 10); //10 = 10S, 12 = 12S
  conf_alert_batt_celldiff = preferences.getUChar("alertcelldiff", 5); //50mV difference -> alert
  conf_alert_batt_voltage = preferences.getUChar("alertlowvoltage", 30); //default 30V 
  conf_alert_batt_temp = preferences.getUChar("alertbatttemp", 50); //50° Celsius -> alert
  conf_alert_esc_temp = preferences.getUChar("alertesctemp", 50);  //50° Celsius -> alert
  conf_flashprotect = preferences.getBool("flashprotect",false);
  conf_espbusmode = preferences.getBool("busmode",true); //false = ESP does not request data, true = ESP requests data
  conf_espwifionstart = preferences.getBool("wifistart",true); //false = ESP does not start wifi on poweron
  conf_beeponalert = preferences.getBool("alertbeep",true); //beep if a error occured?
  conf_unit = preferences.getUChar("unit",0); //unit: 0 = kilmeters, 1 = miles
//change once everything is done at runtime...
  #ifdef useoled1
	  conf_usedisplay1 = preferences.getBool("ud1",true);
//change once everything is done at runtime...
    #ifdef usei2c
      conf_display1_i2c_address = preferences.getChar("i2cadr1", oled1_address);
      conf_display1_spi_gpio_cs = preferences.getChar("spics1", -1);
      conf_display1_spi_gpio_reset = preferences.getChar("spirst1", -1);
    #else
      conf_display1_i2c_address = preferences.getChar("i2cadr1", -1);
      conf_display1_spi_gpio_cs = preferences.getChar("spics1", OLED1_CS);
      conf_display1_spi_gpio_reset = preferences.getChar("spirst1", OLED1_RESET);
    #endif
      conf_display1_rotation = preferences.getChar("drot1", OLED1_ROTATION);
  #else
    conf_usedisplay1 = preferences.getBool("ud1",false);
    conf_display1_i2c_address = preferences.getChar("i2cadr1", -1);
    conf_display1_spi_gpio_cs = preferences.getChar("spics1", -1);
    conf_display1_spi_gpio_reset = preferences.getChar("spirst1", -1);
    conf_display1_rotation = preferences.getChar("drot1", -1);
  #endif
//change once everything is done at runtime...  
  #ifdef useoled2
    conf_usedisplay2 = preferences.getBool("ud2",true);
//change once everything is done at runtime...
    #ifdef usei2c
      conf_display2_i2c_address = preferences.getChar("i2cadr2", oled2_address);
      conf_display2_spi_gpio_cs = preferences.getChar("spics2", -1);
      conf_display2_spi_gpio_reset = preferences.getChar("spirst2", -1);
    #else
      conf_display2_i2c_address = preferences.getChar("i2cadr2", -1);
      conf_display2_spi_gpio_cs = preferences.getChar("spics2", OLED2_CS);
      conf_display2_spi_gpio_reset = preferences.getChar("spirst2", OLED2_RESET);

    #endif
    conf_display2_rotation = preferences.getChar("drot2", OLED2_ROTATION);
  #else
    conf_usedisplay2 = preferences.getBool("ud2",false);
    conf_display2_i2c_address = preferences.getChar("i2cadr2", -1);
    conf_display2_spi_gpio_cs = preferences.getChar("spics2", -1);
    conf_display2_spi_gpio_reset = preferences.getChar("spirst2", -1);
    conf_display2_rotation = preferences.getChar("drot2", -1);
  #endif
  conf_usedisplay = conf_usedisplay1|conf_usedisplay2;

	conf_display_i2cspi = preferences.getBool("di2c",true); //true = i2c, false = spi
//change once everything is done at runtime...  
  //if (conf_display_i2cspi) {
  #ifdef usei2c
    conf_display_i2c_gpio_data = preferences.getChar("i2cdata", oled_sda);
    conf_display_i2c_gpio_clock = preferences.getChar("i2cclock", oled_scl);
    conf_display_i2c_gpio_reset = preferences.getChar("i2creset", oled_reset);
    conf_display_i2c_doreset = preferences.getBool("i2cdoreset",oled_doreset);
    conf_display_i2c_clockspeed = preferences.getUInt("i2cspd",oled_clock);
    conf_display_spi_gpio_miso = preferences.getChar("spimiso", -1);
    conf_display_spi_gpio_mosi = preferences.getChar("spimosi", -1);
    conf_display_spi_gpio_clk = preferences.getChar("spiclk", -1);
    conf_display_spi_gpio_dc = preferences.getChar("spidc", -1);
    conf_display_spi_clockspeed = preferences.getUInt("spispd",0);
  //} else {
  #else
    conf_display_i2c_gpio_data = preferences.getChar("i2cdata", -1);
    conf_display_i2c_gpio_clock = preferences.getChar("i2cclock", -1);
    conf_display_i2c_gpio_reset = preferences.getChar("i2creset", -1);
    conf_display_i2c_doreset = preferences.getBool("i2cdoreset",false);
    conf_display_i2c_clockspeed = preferences.getUInt("i2cspd",0);
    conf_display_spi_gpio_miso = preferences.getChar("spimiso", OLED_MISO);
    conf_display_spi_gpio_mosi = preferences.getChar("spimosi", OLED_MOSI);
    conf_display_spi_gpio_clk = preferences.getChar("spiclk", OLED_CLK);
    conf_display_spi_gpio_dc = preferences.getChar("spidc", OLED_DC);
    conf_display_spi_clockspeed = preferences.getUInt("spispd",oled_clock);
  //}
  #endif

//uart config
	conf_uart_gpio_rx = preferences.getChar("urx", UART2RX);
	conf_uart_gpio_tx = preferences.getChar("utx", UART2TX);
//misc config
//change once everything is done at runtime...
	#ifdef usestatusled
    conf_led_gpio = preferences.getChar("led", led);
  #else
    conf_led_gpio = preferences.getChar("led", -1);
  #endif
//change once everything is done at runtime...  
	#if defined(debuggpio1)
    conf_gpio_debug1 = preferences.getChar("dbg1", debuggpio1);
  #else
    conf_gpio_debug1 = preferences.getChar("dbg1", -1);
  #endif
//change once everything is done at runtime...  
  #if defined(debuggpio2)
	  conf_gpio_debug2 = preferences.getChar("dbg2", debuggpio2);
  #else
    conf_gpio_debug2 = preferences.getChar("dbg2", -1);
  #endif
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
  preferences.putBool("wifistart",conf_espwifionstart); //false = ESP does not start wifi on poweron
  preferences.putBool("alertbeep",conf_beeponalert); //beep if a error occured? 
  preferences.putUChar("unit",conf_unit); //unit: 0 = kilmeters, 1 = miles
//display config
  preferences.putBool("ud1", conf_usedisplay1);    //true to enable display1
	preferences.putBool("ud2", conf_usedisplay2);    //true to enable display2
	preferences.putBool("di2c", conf_display_i2cspi); //true = i2c, false = spi

	preferences.putChar("i2cdata", conf_display_i2c_gpio_data); //gpio number for i2c data, only used then display_i2cspi=true
	preferences.putChar("i2cclock", conf_display_i2c_gpio_clock); //gpio number for i2c clock, only used then display_i2cspi=true
	preferences.putChar("i2creset", conf_display_i2c_gpio_reset); //gpio number for display reset pin, only used then display_i2cspi=true
	preferences.putBool("i2cdoreset", conf_display_i2c_doreset); //shall resetpin be used on display-init?
	preferences.putUInt("i2cspd", conf_display_i2c_clockspeed); //clockspeed in Hz for i2c
	preferences.putChar("spimiso", conf_display_spi_gpio_miso); 
	preferences.putChar("spimosi", conf_display_spi_gpio_mosi); //gpio number for spi mosi, only used then display_i2cspi=false
	preferences.putChar("spiclk", conf_display_spi_gpio_clk); //gpio number for spi clock, only used then display_i2cspi=false
	preferences.putChar("spidc", conf_display_spi_gpio_dc); //gpio number for display data/command mode, only used then display_i2cspi=false
	preferences.putUInt("spispd", conf_display_spi_clockspeed); //clockspeed in Hz for spi
	
	preferences.putChar("i2cadr1", conf_display1_i2c_address); //i2c address of display1, only used then display_i2cspi=true
	preferences.putChar("spics1", conf_display1_spi_gpio_cs); //gpio number for display1 spi chipselect, only used then display_i2cspi=false
	preferences.putChar("spirst1", conf_display1_spi_gpio_reset); //gpio number for display1 spi reset, only used then display_i2cspi=false
	preferences.putChar("drot1", conf_display1_rotation); //rotation of display1 0-3 = 90° steps from 0 to 270
	preferences.putChar("i2cadr2", conf_display2_i2c_address); //i2c address of display2, only used then display_i2cspi=true
	preferences.putChar("spics2", conf_display2_spi_gpio_cs); //gpio number for display2 spi chipselect, only used then display_i2cspi=false
	preferences.putChar("spirst2", conf_display2_spi_gpio_reset); //gpio number for display2 spi reset, only used then display_i2cspi=false
	preferences.putChar("drot2", conf_display2_rotation); //rotation of display2 0-3 = 90° steps from 0 to 270

//uart config
	preferences.putChar("urx", conf_uart_gpio_rx); //gpio number for uart rx, mandatory
	preferences.putChar("utx", conf_uart_gpio_tx); //gpio number for uart tx, mandatory

//misc config
	preferences.putChar("led", conf_led_gpio); //gpio number for status led, -1 if unused
	preferences.putChar("dbg1", conf_gpio_debug1); //gpio number for debug1 pin, -1 if unused
	preferences.putChar("dbg2", conf_gpio_debug2); //gpio number for debug2 pin, -1 if unused
  preferences.end();
}

void printconfig() {
  DebugSerial.printf("#  Wheelsize: %d\r\n",conf_wheelsize);
  DebugSerial.printf("#  Number of Cells: %d\r\n",conf_battcells);
  DebugSerial.printf("#  Cell Voltage Difference Alert: %d0 mV\r\n",conf_alert_batt_celldiff);
  DebugSerial.printf("#  Low Voltage Alert: %d V\r\n",conf_alert_batt_voltage);
  DebugSerial.printf("#  BMS Temperature Alert: %d °C\r\n",conf_alert_batt_temp);
  DebugSerial.printf("#  ESC Temperature Alert: %d °C\r\n",conf_alert_esc_temp);
  DebugSerial.printf("#  Flash Protection: %d\r\n",conf_flashprotect);
  DebugSerial.printf("#  ESP Busmode: %d\r\n",conf_espbusmode);
  DebugSerial.printf("#  ESP Wifi on Start: %d\r\n",conf_espwifionstart);
  DebugSerial.printf("#  Beep on Alert: %d\r\n",conf_beeponalert);
  DebugSerial.printf("#  Units: %d\r\n",conf_unit);
}

void printnvsconfig() {
  //display config
	DebugSerial.printf("Displays enabled: %d\r\n", conf_usedisplay);     //true to globally enable displays, not stored but derived from usedisplay1/2 -> not shown in web
	if (conf_display_i2cspi) {
    DebugSerial.println("I2C Config:"); //true = i2c, false = spi
  	DebugSerial.printf("GPIO Pin Data: %d\r\n", conf_display_i2c_gpio_data); //gpio number for i2c data, only used then display_i2cspi=true
	  DebugSerial.printf("GPIO Pin Clock: %d\r\n", conf_display_i2c_gpio_clock); //gpio number for i2c clock, only used then display_i2cspi=true
	  DebugSerial.printf("GPIO Pin Reset: %d\r\n", conf_display_i2c_gpio_reset); //gpio number for display reset pin, only used then display_i2cspi=true
	  DebugSerial.printf("doReset: %d\r\n", conf_display_i2c_doreset); //shall resetpin be used on display-init?
	  DebugSerial.printf("I2C Clockspeed: %d\r\n", conf_display_i2c_clockspeed); //clockspeed in Hz for i2c
  } else {
    DebugSerial.println("SPI Config:"); //true = i2c, false = spi
    DebugSerial.printf("GPIO Pin MISO: %d\r\n", conf_display_spi_gpio_miso); 
	  DebugSerial.printf("GPIO Pin MOSI: %d\r\n", conf_display_spi_gpio_mosi); //gpio number for spi mosi, only used then display_i2cspi=false
	  DebugSerial.printf("GPIO Pin CLK: %d\r\n", conf_display_spi_gpio_clk); //gpio number for spi clock, only used then display_i2cspi=false
	  DebugSerial.printf("GPIO Pin DC: %d\r\n", conf_display_spi_gpio_dc); //gpio number for display data/command mode, only used then display_i2cspi=false
	  DebugSerial.printf("SPI Clockspeed: %d\r\n", conf_display_spi_clockspeed); //clockspeed in Hz for spi
  }
  if (conf_usedisplay1) {
    DebugSerial.println("Display 1 Config:");    //true to enable display1
    if (conf_display_i2cspi) {
      DebugSerial.printf("  I2C Address: %d\r\n", conf_display1_i2c_address); //i2c address of display1, only used then display_i2cspi=true
    } else {
      DebugSerial.printf("  GPIO Pin SPI CS: %d\r\n", conf_display1_spi_gpio_cs); //gpio number for display1 spi chipselect, only used then display_i2cspi=false
      DebugSerial.printf("  GPIO Pin SPI Reset: %d\r\n", conf_display1_spi_gpio_reset); //gpio number for display1 spi reset, only used then display_i2cspi=false
    }
    DebugSerial.printf("  Rotation: %d\r\n", conf_display1_rotation); //rotation of display1 0-3 = 90° steps from 0 to 270
  }
  if (conf_usedisplay2) {
	  DebugSerial.println("Display 2 Config:");    //true to enable display2
    if (conf_display_i2cspi) {
      DebugSerial.printf("  I2C Address: %d\r\n", conf_display2_i2c_address); //i2c address of display2, only used then display_i2cspi=true
    } else {
  	  DebugSerial.printf("  GPIO Pin SPI CS: %d\r\n", conf_display2_spi_gpio_cs); //gpio number for display2 spi chipselect, only used then display_i2cspi=false
	    DebugSerial.printf("  GPIO Pin SPI Reset: %d\r\n", conf_display2_spi_gpio_reset); //gpio number for display2 spi reset, only used then display_i2cspi=false
    }
	  DebugSerial.printf("  Rotation: %d\r\n", conf_display2_rotation); //rotation of display2 0-3 = 90° steps from 0 to 270
  }
//uart config
  DebugSerial.println("UART Config:");
	DebugSerial.printf("  GPIO Pin UART RX: %d\r\n", conf_uart_gpio_rx); //gpio number for uart rx, mandatory
	DebugSerial.printf("  GPIO Pin UART TX: %d\r\n", conf_uart_gpio_tx); //gpio number for uart tx, mandatory

//misc config
  DebugSerial.println("Misc Config:");
	DebugSerial.printf("  GPIO Pin LED: %d\r\n", conf_led_gpio); //gpio number for status led, -1 if unused
	DebugSerial.printf("  GPIO Pin Debug1: %d\r\n", conf_gpio_debug1); //gpio number for debug1 pin, -1 if unused
	DebugSerial.printf("  GPIO Pin debug2: %d\r\n", conf_gpio_debug2); //gpio number for debug2 pin, -1 if unused

}