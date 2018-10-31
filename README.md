# esp32_xiaomi_m365
Sample Project for decoding serial bus data on a Xiaomi M365 Scooter on Espressif ESP32:
- Supports one or two 0.96" 128x64 SSD1306 based OLED screen(s) (SSD1351/128x128 Color OLED support to follow)
- Display connectivity via i2c or spi (spi preffered as it's way faster)
- Firmwareupdates can be done over WiFi using ArduinoOTA
- Connects to known WiFi Networks or opens it's own SSID in AP Mode
- optional Telnet Servers for debugging
- Config Menu for changing scooter & esp settings
- Supports different Screen Languages, Miles/Kilometers, Wheel Sizes, 10/12s batteries
- Alerts for Low Battery, ESC, BMS Temperature, Cell-Voltage Monitoring,...

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
 - fix speed for int16 overflow in m/h -> faster than 31.7 -> overflow
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
M365 has a Serial One Wire Bus between BLE Module and ESC which consists of 4 wires, the connection as seen on the BLE Module:
- Ground  (Black, "G")
- One Wire Serial Connection (Yellow, "T", 115200bps, 8n1)
- VBatt (Green, "P", always available)
- 5V (Red, "5", only when scooter is turned on)

ESP32 needs a Vcc of 3.3V, while at the same time the GPIO Pins are 5V save, so you can wire the 5V to a Vreg for 3.3v which feed the ESP, while the Serial Connection can be wired to RX/TX Pins.
It might be a idea to use e.g. 680R or 1k in series to protect the gpio, as well as add a diode from rx in series with a ~100-200R towards TX

# PCB Bugs v180723
 - 1nF or 470pF C from EN towards GND and 12-18k R from EN towards 3.3V are missing (sad copy and paste error)
 - the SOT23 Diode housing might be wrong - depending on your Diode (BAS70xx versions) -> the Diode-Package can be rotated 120° CCW to fix that

# PATCH your libraries!
 - Adafruit_SSD1306 uses 100kHz I2C Clock per default and does not support individual GPIO Pins for Clock and Data, same goes for SPI Support in that library. forked & fixed version: https://github.com/smartinick/Adafruit_SSD1306
 - arduino-esp32 core implementation of HardwareSerial and esp32-hal-uart only triggers a uart-rx event/interrupt every 112 bytes which makes it impossible to stay within the timing necersarry for the m365 one-wire-uart. (see config.h comments for more details) forked & fixed version: https://github.com/smartinick/arduino-esp32
