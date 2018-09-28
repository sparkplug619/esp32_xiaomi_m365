# esp32_xiaomi_m365
Sample Project for decoding serial bus data on a Xiaomi M365 Scooter on Espressif ESP32:
- Supports one or two 0.96" 128x64 SSD1306 based OLED screen(s) (SSD1351/128x128 Color OLED support to follow)
- Display connectivity via i2c or spi (spi preffered as it's way faster)
- ESP8266 has a bug (see issues) and currently i'm using preferences class to store settings which is not available for ESP8266 (wrapper for nvram would be needed)
- Firmwareupdates can be done over WiFi using ArduinoOTA
- optional Telnet Servers for debugging
- Config Menu for changing scooter & esp settings
- Supports different Screen Languages, Miles/Kilometers, Wheel Sizes, 10/12s batteries
- use Throttle in stopped mode to Swipe through different screens
- press Throttle and Brake at the same time while not moving to enter menu -> throttle navigates, brake selects/changes items

# Done/Details
- WiFi Auto Connect to known SSIDs or AP-Mode & Telnet with Timeouts to auto-turnoff Telnet/WiFi
- Firmwareupdates (of ESP Device, not the scooter) over WiFi
- Telnet on Port 36523 with different Screens
- Telnet on Port 36524 with raw byte dump from M365 Bus ("read only")
- Telnet on Port 36525 with packet decode/dump from M365 Bus ("read only")
- M365 Serial Receiver and Packet decoder into Array per Address
- M365 Data Requestor obey the M365 Bus Timing, does not interfere with Xiaomi's Firmware-Flash process, but can be disabled in the menu (ESP Bus Mode Passive)
- Data Requestor internally allows subscription (via bit-fields) to predefined groups of data (e.g. OLED display only needs Cell-Voltage Data when it's displayed).
- Internal Timers/Counters for debugging Receiver/Requestor & Oled-Draw/Transfer Durations - see Telnet/36523 Statistics Screen
- Display Shows LED/Light Status (to recognize accidental switched on Light @ noon while switching normal/eco mode)
- Display Eco/Normal Mode Status (and shows Batt-Percent, so the original Status LEDs on the head unit are not needed
- Different Screens for Drive/Stop/Charge/Error/Firmwareupgrade/Locked State/Timeout-Screen
	- Charge Screen also shows Cell Voltages (if using single-display use throttle to switch to cellvoltage screen)
- Alerts: Background Task ("Housekeeper") periodically requests additional data from M365 and validates against tresholds (set in configmenu), shows alert-popup and increases alert-counters

# Todos
 - fix - data-requestor timing currently causes ~10% crc errors on m365 bus
 - OLED: Add Popup Messages for Events (e.g. Scooter Error, Temp, BMS CellVoltage variations > treshold, WLAN/BLE On/Off, Client Connected,...)
 - "LOCKED" screen mode when scooter has been locked
 - add display-sleeptimer - x seconds after last event (gas/brake/throttle/speed/chargerun-plug/telnet/AP-Client Connection & speed = 0)
 - advanced thief/lock protection
 - fix speed for int16 overflow in m/h -> faster than 31.7 -> overflow

# further Ideas & Visions:
 - add Scooter-Flashing Protection (so no one can flash broken firmware to your scooter while waiting at a red traffic light
 - advanced trip computer (which keeps trip-totals/averages between 2 charge cycles or 2 times with the same available SSID (leaving/coming home)
 - MQTT Logging of Trip-Summary Data
 - Navigation Arrow Display e.g. with Komoot (https://github.com/komoot/BLEConnect)
 
# Telnet / Debugging Interface
 - Telemetrie Screen shows decoded known values: Batt Voltage, Current, Speed,... 
 - Statistics Screen dumps some internal counters, e.g. Packet Counters, CRC Files, Timing,...
 - ESC/BLE/BMS/"X1" RAW Screens dumps the 512 Byte Array for each device (format "00 00 ...")
 
use the letters
 - s,t,e,b,n,x to switch between the screens
 - r to reset statistics & zero m365 data arrays

# Wiring
M365 has a Serial One Wire Bus between BLE Module and ESC which consists of 4 wires, the connection as seen on the BLE Module:
- Ground  (Black, "G")
- One Wire Serial Connection (Yellow, "T", 115200bps, 8n1)
- VBatt (Green, "P", always available)
- 5V (Red, "5", only when scooter is turned on)

ESP32/8266 needs a Vcc of 3.3V, while at the same time the GPIO Pins are 5V save, so you can wire the 5V to a Vreg for 3.3v which feed the ESP, while the Serial Connection can be wired to RX/TX Pins.
It might be a idea to use e.g. 680R or 1k in series to protect the gpio, as well as add a diode from rx in series with a ~100-200R towards TX

# PCB Bugs v180723
 - 1nF or 470pF C from EN towards GND and 12-18k R from EN towards 3.3V are missing (sad copy and paste error)
 - the SOT23 Diode housing might be wrong - depending on your Diode (BAS70xx versions) -> the Diode-Package can be rotated 120° CCW to fix that

# PATCH your libraries!
 - Adafruit_SSD1306 uses 100kHz I2C Clock per default and does not support individual GPIO Pins for Clock and Data, same goes for SPI Support in that library. forked & fixed version: https://github.com/smartinick/Adafruit_SSD1306
 - arduino-esp32 core implementation of HardwareSerial and esp32-hal-uart only triggers a uart-rx event/interrupt every 112 bytes which makes it impossible to stay within the timing necersarry for the m365 one-wire-uart. (see config.h comments for more details) forked & fixed version: https://github.com/smartinick/arduino-esp32
