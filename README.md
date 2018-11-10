# esp32_xiaomi_m365
Sample Project for decoding serial bus data on a Xiaomi M365 Scooter on Espressif ESP32:
- Supports one or two 0.96" 128x64 SSD1306 based OLED screen(s)
- Display connectivity via i2c or spi (spi preffered as it's much faster)
- Firmwareupdates (of the ESP32) can be done over WiFi using ArduinoOTA, Scooterfirmwareupdates are not blocked, can be done the usual way
- Connects to known WiFi Networks or opens it's own SSID in AP Mode
- optional Telnet Servers for debugging
- Config Menu for changing scooter & esp settings
- Supports different Screen Languages, Miles/Kilometers, Wheel Sizes, 10/12s batteries
- Alerts for Low Battery, ESC, BMS Temperature, Cell-Voltage Monitoring,...
- displays speeds above 32.768km/h (set for speeds from 10km/h backwards to 55,44km/h forward, backwardspeed is displayed as a positive number)

# Details
- Telnet on Port 36523 with different Screens
- Telnet on Port 36524 with raw byte dump from M365 Bus ("read only")
- Telnet on Port 36525 with packet decode/dump from M365 Bus ("read only")
- M365 Serial Receiver and Packet decoder into Array per Address
- M365 Data Requestor obey the M365 Bus Timing, does not interfere with Xiaomi's Firmware-Flash process, but can be disabled in the menu (ESP Bus Mode Passive)
- Data Requestor internally allows subscription to predefined groups of data (e.g. OLED display only needs Cell-Voltage Data when it's displayed) to reduce requested data and therefore get a higher update rate on the interesting data fields.
- Internal Timers/Counters for debugging Receiver/Requestor & Oled-Draw/Transfer Durations - see Telnet/36523 Statistics Screen
- Different Screens for Drive/Stop/Charge/Error/Firmwareupgrade/Locked State/Timeout-Screen
- Alerts: Background Task ("Housekeeper") periodically requests additional data from M365 and validates against tresholds (set in configmenu), shows alert-popup and increases alert-counters
- Popups on screen if alerts occur, alert-counters in data screens when stopped
- Scooter can be locked/unlocked/switched off from config-menu
- Custom Screens for
	- Driving
	- "Stopped" - multiple Data screens when not moving (swipe through them using throttle)
	- "Charging" - in single-screen configuration multiple screens while charging (swipe through them using throttle)
	- Config Menu (push Brake & Throttle at the same time to enter, select items using throttle, change/enter using brake)
 	- "LOCKED" screen mode when scooter has been locked

# Todos
 - fix: cmd_beep does not work
 - fix: data-requestor timing currently causes ~10% crc errors on m365 bus
 - add display-sleeptimer - x seconds after last event (gas/brake/throttle/speed/chargerun-plug/telnet/AP-Client Connection & speed = 0)
 - "beep" command does not work in current version
 - add trip computer
 - add navigation code
 - add flashprotetection function

# Telnet / Debugging Interface
 - Telemetrie Screen shows decoded known values: Batt Voltage, Current, Speed,... 
 - Statistics Screen dumps some internal counters, e.g. Packet Counters, CRC Files, Timing,...
 - ESC/BLE/BMS/"X1" RAW Screens dumps the 512 Byte Array for each device (format "00 00 ...")
 
use the letters
 - s,t,e,b,n,x to switch between the screens
 - r to reset statistics & zero m365 data arrays

# Wiring
## Connector in M365 Headunit
M365 has a Serial One Wire Bus between BLE Module and ESC which consists of 4 wires, the connection as seen on the BLE Module:
- Ground  (Black, "G")
- One Wire Serial Connection (Yellow, "T", 115200bps, 8n1)
- VBatt (Green, "P", always available)
- 5V (Red, "5", only when scooter is turned on)

## ESP32 Powering
ESP32 needs a Vcc of 3.3V, so you can wire the 5V from M365 to a Vreg for 3.3v which powers the ESP. DO NOT POWER THE ESP32 WITH 5V!

## M365 Serial Data Bus Connection

Other then the VCC which can not be 5V, the GPIO Pins can be connected to 5V signals. the logic-high from 3.3v powered ESP32 is still enough for detection as high-level for a 5v device.
So the Serial RX & TX Pins can be directly connected, but to ensure communication on the one-wire uart bus we need 2, to be on the safe side 3 parts:
 - 680R or 1k between RX and M365 Data Bus
 - And a Diode from M365 Data Bus in series with a ~100-200R towards TX Pin of  ESP32

## ESP32 Pin assignment

The ESP32 has a io-switchmatrix, so (nearly) any hardwareunit can be mapped to nearly any pin.
Just be sure to not to use:
 - GPIO_NUM_34 – GPIO_NUM_39 are only GPI - Input, NO OUTPUT, No Pullup or Pulldown!
 - GPIO 6-11 are used by flash - not usable
 - GPIO 0 is for boot/flashing with pullup


# PCB Bugs v180723
 - 1nF or 470pF C from EN towards GND and 12-18k R from EN towards 3.3V are missing (sad copy and paste error)
 - the SOT23 Diode housing might be wrong - depending on your Diode (BAS70xx versions) -> the Diode-Package can be rotated 120° CCW to fix that

# Before Compiling

## PATCH your libraries!
 - Adafruit_SSD1306 uses 100kHz I2C Clock per default and does not support individual GPIO Pins for Clock and Data, same goes for SPI Support in that library. forked & fixed version: https://github.com/smartinick/Adafruit_SSD1306 (platformio.ini is configured to use this library - no user interaction needed)
 - arduino-esp32 core implementation of HardwareSerial and esp32-hal-uart only triggers a uart-rx event/interrupt every 112 bytes which makes it impossible to stay within the timing necersarry for the m365 one-wire-uart. (see config.h comments for more details) forked & fixed version: https://github.com/smartinick/arduino-esp32 (for now this has to be done locally on your dev machine)

## setup individual settings
2 files are not included in the repository on purpose: in those files your ssid/passwords and your pinout for  your board are stored. that way you can update the code from the repo and your custom settings are not changed

### secrets.h
- Make a secrets.h file in the same directory as the .ino and other files are, copy this template and adopt the ssids/passwords:
```
#ifndef SECRETS_h
#define SECRETS_h

//Wifi - CHANGE IT TO YOUR OWN SETTINGS!!!
    #define maxssids 1
    #define ssid1 "yourssid"
    #define password1 "yourpassphrase"
    #define ssid2 "..."
    #define password2 "..."
    #define ssid3 "..."
    #define password3 "..."

//SSID/Pass for Access-Point Mode - CHANGE IT TO YOUR OWN SEQUENCE!!!
    #define ap_ssid "m365oled"
    #define ap_password "appassword"

//Over The Air Firmwareupdates - password - CHANGE IT TO YOUR OWN SEQUENCE!!!
    #define OTApwd "yourotapassword"

#endif
```
- save the file ;)
## setup board.h
- Make a boards.h file in the same directory as the .ino and other files are, copy this template and adopt the ssids/passwords:
```
#ifndef boards_h
#define boards_h

//display config
 
  #define usei2c //comment out for SPI
  #define useoled1 //comment out to disable oled functionality
  #define useoled2 //comment out if you use only one display
  #define OLED1_ROTATION 2 //0 = normal, 1= 90, 2=180, 3=270° CW
  #define OLED2_ROTATION 0 //0 = normal, 1= 90, 2=180, 3=270° CW

//pin definitions for i2c display:
  #if (defined usei2c && defined useoled1) //one display, I2C Mode
    #define oled_scl GPIO_NUM_32 //SCLK Pad on PCB
    #define oled_sda GPIO_NUM_33 //MOSI Pad on PCB
    #define oled_reset -1
    #define oled1_address 0x3C
  #endif

  #if (defined usei2c && defined useoled2) //2nd display, i2c mode
      #define oled2_address 0x3D
  #endif

//pin definitions for spi display:
  #if (!defined usei2c && defined useoled1 && defined ESP32) //one display, ESP32/Hardware SPI Mode
      #define OLED_MISO   GPIO_NUM_19
      //this is just a unused GPIO pin - SPI Lib needs a MISO Pin, display off course not :D
      #define OLED_MOSI   GPIO_NUM_33
      #define OLED_CLK    GPIO_NUM_32
      #define OLED_DC     GPIO_NUM_25
      //OLED1 on OLED1 Connector:
        #define OLED1_CS    GPIO_NUM_26
        #define OLED1_RESET GPIO_NUM_27
      //OLED1 on OLED2 Connector:
        //#define OLED1_CS    GPIO_NUM_14
        //#define OLED1_RESET GPIO_NUM_12
        //#define OLED1_ROTATION 0
  #endif

  #if (!defined usei2c && defined useoled2 && defined ESP32) //2nd display, ESP32/Hardware SPI Mode
      #define OLED2_CS    GPIO_NUM_14
      #define OLED2_RESET GPIO_NUM_12
  #endif

  //definitions for uart with 2 gpio pins
    #define UART2RX GPIO_NUM_23 //PCB v180723
    #define UART2TX GPIO_NUM_22 //PCB v180723
    #define UART2RXunused GPIO_NUM_21 //PCB v180723; ESP32 does not support RX or TX only modes - so we remap the rx pin to a unused gpio during sending

#endif
```
- comment/uncomment & adopt the definitions in that file to match your setup.
- after creating the file, you have one step to complete (and this step has to be done after each "git pull":  then open definitions.h and comment out all "#define dev_*" lines in the beginning (put // in front of the line). then uncomment,(remove the //) the line "#define dev_customboard"
- save the 2 files ;)

## ngcode.h/cpp
 - If it does not compile because there's a missing file, goto definitions.h and remove the 2 lines dealing with "ngcode". In that case i was to lazy or forgot to remove it myself before committing to the repo.
 - Don't ask about ncode - it's just some test stuff and advanced features that must not be shared with public.

# Support, Questions,...?
 - Telegram Group https://t.me/esp32brain4m365

# Credits & Thanks to
 - Paco Gorina for his research on the protocoll and packet structure (-> http://www.gorina.es/9BMetrics/)
 - CamiAlfa for his research and providing informations about the contents of the packets & arrays (-> https://github.com/CamiAlfa/M365-BLE-PROTOCOL)
 - french translation kindly provided by Technoo' Loggie (Telegram Handle @TechnooLoggie)

# Licensed under Creative Commons V4 - BY NC SA