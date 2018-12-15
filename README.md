attention - last update of this repo - i moved it to gitlab -> https://gitlab.com/esp32m365/esp32_xiaomi_m365_oleddisplay

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
- M365 Data Requestor obey the M365 Bus Timing, does not interfere with Xiaomi's Firmware-Flash process, but can be disabled in the menu (ESP Bus Mode = Passive)
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
 - add display-sleeptimer - x seconds after last event (gas/brake/throttle/speed/chargerun-plug/telnet/AP-Client Connection & speed = 0)

# Telnet / Debugging Interface
 - Telemetrie Screen shows decoded known values: Batt Voltage, Current, Speed,... 
 - Statistics Screen dumps some internal counters, e.g. Packet Counters, CRC Files, Timing,...
 - ESC/BLE/BMS/"X1" RAW Screens dumps the 512 Byte Array for each device (format "00 00 ...")

 use the letters
  - s,t,e,b,n,x to switch between the screens
  - r to reset statistics & zero m365 data arrays

# Wiring
## Connector in M365 Headunit
M365 has a Serial Data Bus over one Wire (UART in half-duplex mode) between BLE Module and ESC (Motor Controller) and we want to connect the ESP32 to that bus, as well as get Power from the Scooter.
The Connection between BLE-Module and ESC consists of 4 wires, the connection as seen on the BLE Module:
- Ground  (Black, "G")
- One Wire Serial Connection (Yellow, "T", 115200bps, 8n1)
- VBatt (Green, "P", always available)
- 5V (Red, "5", only when scooter is turned on)

## ESP32 Powering
ESP32 needs a Vcc of 3.3V, so you can wire the 5V from M365 to a Vreg for 3.3v which powers the ESP. DO NOT POWER THE ESP32 WITH 5V!

## M365 Serial Data Bus Connection

Other then the VCC which can not be 5V, the ESP32 GPIO Pins can be connected to 5V signals.
So the Serial RX & TX Pins can be directly connected, but to ensure communication on the one-wire uart bus we need 2, to be on the safe side 3 parts:
 - 680R or 1k between RX and M365 Data Bus
 - And a Diode from M365 Data Bus in series with a ~100-200R towards TX Pin of  ESP32
That way Hardware-UART on the ESP32 can be used.

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

## Choose your environment  (platformio):
  - PlatformIO supports different (build-)"Environments", you will mostly want to select "m365client" which implements the "client" for a stock m365 scooter and the oled display.
## PATCH your libraries!
  - Plattformio includes my patched Adafuit_SSD1306 library for you, if u are using a different ide, please look in platformio.ini for references to the patches libraries and make sure those are used. In the meanwhile Adafruit updated it's libraries to get rid of the problems, so one day i'll update my code to use the new lib from Adafruit and get rid of the patched libraries.
  - earlier code versions needed some files patched in arduino-esp32 framework in order for correct uart functionality. now i've included the patched UART-Files in this repo, so the patch in arduino-esp32 is not needed anymore (save you time and solves the problem that your other projects would also use that patched framework). But one day when arduino-esp32 framework get's updated this might brake and the files included in this project need to be updated.
  - unfortunately the latest update of platform-io included the arduino-esp32 framework with a broken i2c implementation. This needs to be patched manually, but it fixes a general problem, so this will help your other projects using i2c as well. For patching please see this issue and replace the 4 files as mentioned - otherwise the i2c-display will hang at most restarts: https://github.com/platformio/platform-espressif32/issues/126

## setup individual settings
### ssid's and passwords:
 - please open secrets.h and fill in your own ssid/passwords for client/ap mode and ota updates!
### setup for your hardware
 - copy one of my board*.h files that fits your hardware best, update it for your setup and include this file in definitions.h
 - when updating my code you just need to reapply the include of your boards file in definitions.h

# Support, Questions,...?
 - Telegram Group https://t.me/esp32brain4m365

# Credits & Thanks to
 - Paco Gorina for his research on the protocoll and packet structure (-> http://www.gorina.es/9BMetrics/)
 - CamiAlfa for his research and providing informations about the contents of the packets & arrays (-> https://github.com/CamiAlfa/M365-BLE-PROTOCOL)
 - french translation kindly provided by Technoo' Loggie (Telegram Handle @TechnooLoggie)

# Licensed under Creative Commons V4 - BY NC SA
 - THIS MEANS YOU CAN USE THIS CODE FOR NON COMMERCIAL PURPOSES, HAVE TO RESHARE YOUR MODIFICATIONS AND REFERENCE THIS REPO.
