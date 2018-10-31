#ifndef M365_h
#define M365_h

#include "definitions.h"
#include "config.h"
#include "strings.h"
#include "display.h"

#include <HardwareSerial.h>

extern HardwareSerial M365Serial;  // UART2 for M365

//M365 - serial receiver
  #define maxlen 256 //serial buffer size in bytes
  extern uint8_t crc1;
  extern uint8_t crc2;
  extern uint16_t crccalc;
  extern uint8_t sbuf[maxlen];
  extern uint8_t len;
  extern uint8_t readindex;
  extern uint8_t m365receiverstate;
  extern uint8_t m365receiverstateold;
  #define m365receiveroff 0 //Serial1 will not be read...
  #define m365receiverready 1 //reading bytes until we find 0x55
  #define m365receiverpacket1 2 //preamble 0x55 received, waiting for 0xAA
  #define m365receiverpacket2 3 //len preamble received waiting for LEN
  #define m365receiverpacket3 4 //payload this state is kept until LEN bytes received
  #define m365receiverpacket4 5 //checksum receiving checksum
  #define m365receiverpacket5 6 //received a packet, test for checksum
  #define m365receiverstorepacket 7 //packet received, checksum valid, store in array, jump back to receiverready for next packet, set newpacket flag

//M365 - packets
  extern uint8_t m365packetstate;
  extern uint8_t m365packetstateold;
  #define m365packetidle 0
  #define m365newpacket 1
  //offsets in sbuf
  #define i_address 0
  #define i_hz 1
  #define i_offset 2
  #define i_payloadstart 3
  #define address_ble 0x20 //actively sent data by BLE Module with gas/brake & mode values
  #define address_x1 0x21 //actively sent data by ?BLE? with status like led on/off, normal/ecomode...
  #define address_esc 0x23 
  #define address_bms_request 0x22 //used by apps to read data from bms
  #define address_bms 0x25 //data from bms sent with this address (only passive if requested via address_esc_request)

//M365 - Statistics
  extern uint16_t packets_rec;
  extern uint16_t packets_crcok;
  extern uint16_t packets_crcfail;
  extern uint16_t packets_rec_bms;
  extern uint16_t packets_rec_esc;
  extern uint16_t packets_rec_x1;
  extern uint16_t packets_rec_ble;
  extern uint16_t packets_rec_unhandled;

  extern uint16_t packetsperaddress[256];
  extern uint16_t requests_sent_bms;
  extern uint16_t requests_sent_esc;
  extern uint16_t esccommands_sent;
  extern uint16_t x1commands_sent;
  extern uint16_t requests_sent_housekeeper;

  extern int16_t speed_min;
  extern int16_t speed_max;
  extern int16_t current_min;
  extern int16_t current_max;
  extern int32_t watt_min;
  extern int32_t watt_max;
  extern uint8_t tb1_min; //temperature batt 1 min
  extern uint8_t tb1_max; //temperature batt 1 max
  extern uint8_t tb2_min;  //temperature batt 2 min
  extern uint8_t tb2_max;  //temperature batt 2 max
  extern uint16_t te_min;  //temperature esc min
  extern uint16_t te_max;  //temperature esc max
  extern uint16_t lowest; //Cell Voltages - Lowest
  extern uint16_t highest; //Cell Voltages - Highest

  
  extern unsigned long m365packettimestamp;
  extern unsigned long m365packetlasttimestamp;
  extern unsigned long duration_requestcycle;
  extern unsigned long timestamp_requeststart;


//M365 Device Buffers & Structs
  typedef struct {
    uint8_t u1[1];
    uint8_t throttle;
    uint8_t brake;
    //uint8_t u2[509];
  }__attribute__((packed))   blestruct;

  typedef struct {
    uint8_t mode; //offset 0x00 mode: 0-stall, 1-drive, 2-eco stall, 3-eco drive
    uint8_t battleds;  //offset 0x01 battery status 0 - min, 7(or 8...) - max
    uint8_t light;  //offset 0x02 0= off, 0x64 = on
    uint8_t beepaction;  //offset 0x03 "beepaction" ?
    //uint8_t u1[508];
  }__attribute__((packed))   x1struct;

  typedef struct {
    uint16_t u1[0x10]; //offset 0-0x1F
    char serial[14]; //offset 0x20-0x2D
    char pin[6]; //offset 0x2E-0x33
    uint8_t fwversion[2]; //offset 0x34-035,  e.g. 0x133 = 1.3.3
    uint16_t u2[10]; //offset 0x36-0x49
    uint16_t remainingdistance; //offset 0x4a-0x4B e.g. 123 = 1.23km
    uint16_t u3[20]; //offset 0x4C-0x73
    uint16_t ontime1; //offset 0x74-0x75 power on time in seconds
    uint16_t triptime; //offset 0x76-0x77 trip time in seconds
    uint16_t u4[2]; //offset 0x78-0x7C
    uint16_t frametemp1; //offset 0x7C-0x7D /10 = temp in °C //Unused on M365/FW1.4.0
    uint16_t u5[54]; //offset 0x7e-0xe9
    uint16_t ecomode; //offset 0xEA-0xEB; on=1, off=0
    uint16_t u6[5]; //offset 0xec-0xf5
    uint16_t kers; //offset 0xf6-0xf7; 0 = weak, 1= medium, 2=strong
    uint16_t cruisemode; //offset 0xf8-0xf9, off 0, on 1
    uint16_t taillight; //offset 0xfa-0xfb, off 0, on 2
    uint16_t u7[50]; //offset  0xfc-0x15f
    uint16_t error; //offset 0x160-0x161
    uint16_t u8[1]; //offset 0x162-0x163
    uint8_t lockstate; //offset 0x164
    uint8_t u9; //offset 0x165
    uint16_t u10; //offset 0x166-0x167
    uint16_t battpercent; //offset 0x168-0x169
    int16_t speed; //0x16A-0x16B /1000 in km/h, negative value = backwards...
    uint16_t averagespeed; //0x16C-0x16D /1000 in km/h?
    uint32_t totaldistance; //0x16e-0x171 /1000 in km
    uint16_t tripdistance; //0x172-0x173
    uint16_t ontime2; //offset 0x174-0x175 power on time 2 in seconds
    uint16_t frametemp2; //0x176-0x177 /10 = temp in °C
    uint16_t u11[68]; //offset 0x178-0x200 
  }__attribute__((packed))   escstruct;

  typedef struct {
    uint16_t u1[0x10]; //offset 0-0x1F
    char serial[14]; //offset 0x20-0x2D
    uint8_t fwversion[2]; //offset 0x2E-0x2f e.g. 0x133 = 1.3.3
    uint16_t totalcapacity; //offset 0x30-0x31 mAh
    uint16_t u2a[2]; //offset 0x32-0x35
    uint16_t cycles; //offset 0x36-0x37
    uint16_t chargingtimes; //offset 0x38-0x39
    uint16_t u2b[3]; //offset 0x3a-0x3f
    uint16_t proddate; //offset 0x40-0x41
        //fecha a la batt 7 MSB->año, siguientes 4bits->mes, 5 LSB ->dia ex:
      //b 0001_010=10, año 2010
      //        b 1_000= 8 agosto
      //            b  0_1111=15 
      //  0001_0101_0000_1111=0x150F 
    uint16_t u3[0x10]; //offset 0x42-0x61
    uint16_t remainingcapacity; //offset 0x62-0x63
    uint16_t remainingpercent; //offset 0x64-0x65
    int16_t current; //offset 0x66-67 - negative = charging; /100 = Ampere
    uint16_t voltage; //offset 0x68-69 /10 = Volt
    uint8_t temperature[2]; //offset 0x6A-0x6B -20 = °C
    uint16_t u4[5]; //offset 0x6C-0x75
    uint16_t health; //offset 0x76-0x77; 0-100, 60 schwellwert "kaputt"
    uint16_t u5[4]; //offset 0x78-0x7F
    uint16_t Cell1Voltage; //offset 0x80-0x81
    uint16_t Cell2Voltage; //offset 0x82-0x83
    uint16_t Cell3Voltage; //offset 0x84-0x85
    uint16_t Cell4Voltage; //offset 0x86-0x87
    uint16_t Cell5Voltage; //offset 0x88-0x89
    uint16_t Cell6Voltage; //offset 0x8A-0x8B
    uint16_t Cell7Voltage; //offset 0x8C-0x8D
    uint16_t Cell8Voltage; //offset 0x8E-0x8F
    uint16_t Cell9Voltage; //offset 0x90-0x91
    uint16_t Cell10Voltage; //offset 0x92-0x93
    uint16_t Cell11Voltage; //offset 0x94-0x95 not stock, custom bms with 12S battery, 0 if not connected
    uint16_t Cell12Voltage; //offset 0x96-0x97 not stock, custom bms with 12S battery, 0 if not connected
    //uint16_t u6[178]; //offset 0x98-0x
  }__attribute__((packed))  bmsstruct;

  extern uint8_t bledata[512];
  extern uint8_t x1data[512];
  extern uint8_t escdata[512];
  extern uint8_t bmsdata[512];

  extern blestruct* bleparsed;
  extern x1struct* x1parsed;
  extern escstruct* escparsed;
  extern bmsstruct* bmsparsed;

  extern bool newdata; //flag - we have update at least one byte in one of the data arrays
  extern bool senddata; //flag - we should send our data request _now_

//M365 - command stuff
  #define esccommandlen 10
  extern uint8_t esccommand[esccommandlen];
  #define esccommand_cmd 5
  #define esccommand_value1 6
  #define esccommand_value2 7
  #define esccommand_crcstart 2
  #define esccommand_crc1 8
  #define esccommand_crc2 9

  #define x1commandlen 12
  extern uint8_t x1command[x1commandlen];
  #define x1command_beepnum 9
  #define x1command_crcstart 2
  #define x1command_crc1 10
  #define x1command_crc2 11


  extern uint8_t sendcommand;
  #define cmd_none 0
  #define cmd_kers_weak 1
  #define cmd_kers_medium 2
  #define cmd_kers_strong 3
  #define cmd_cruise_on 4
  #define cmd_cruise_off 5
  #define cmd_light_on 6
  #define cmd_light_off 7
  #define cmd_turnoff 8
  #define cmd_lock 9
  #define cmd_unlock 10
  #define cmd_beep5 11

//housekeeper (manages alerts,...)
  extern uint8_t hkstate;
  extern uint8_t hkstateold;
  #define hkwaiting 0
  #define hkrequesting 1
  #define hkevaluating 2

  
  extern unsigned long housekeepertimestamp;

//alerts
  extern bool alert_escerror;
  extern bool alert_cellvoltage;
  extern bool alert_bmstemp;
  extern bool alert_esctemp;
  extern bool alert_undervoltage;
  extern uint16_t alertcounter_escerror;
  extern uint16_t alertcounter_cellvoltage;
  extern uint16_t alertcounter_bmstemp;
  extern uint16_t alertcounter_esctemp;
  extern uint16_t alertcounter_undervoltage;

//scooter-config stuff
  #define wheelfact8km 1.0f
  #define wheelfact8miles  0.621371192f
  #define wheelfact10km    1.176470588f
  #define wheelfact10miles 0.731024932f
  extern float wheelfact;

//M365 - request stuff 
  /*
  request data from esc:   (Read 0x7x 2 words)
  PREAMBLE  LEN  Adr  HZ   Off  LEN  FIX1 FIX2 FIX30x
  0x55 0xaa 0x06 0x20 0x61 0x7c 0x02 0x02 0x28 0x27 CRC1 CRC2
  request data from bms: (Read Serial @ Offset 0x10, 0x10 words)
  PREAMBLE    LEN   Adr   HZ    Off   Len   CRC1  CRC2
  0x55  0xAA  0x03  0x22  0x01  0x10  0x12  0xB7  0xFF
  */

  //packet for bms requests and offsets
    #define requestbmslen 9
    extern uint8_t request_bms[requestbmslen];
    #define bms_request_offset 5
    #define bms_request_len 6
    #define bms_request_crcstart 2
    #define bms_request_crc1 7
    #define bms_request_crc2 8
  //packets for esc requests and offsets
    #define requestesclen 12
    extern uint8_t request_esc[requestesclen];
    #define esc_request_offset 5 
    #define esc_request_len 6
    #define esc_request_throttle 8
    #define esc_request_brake 9
    #define esc_request_crcstart 2
    #define esc_request_crc1 10
    #define esc_request_crc2 11

  //request arrays  
    extern uint8_t requestindex;
    #define requestmax 9
    extern uint8_t requests[requestmax][3];

  //friendly names
    #define rq_esc_essentials 0x0001
    #define rq_esc_remdist 0x0002
    #define rq_esc_essentials2 0x0004
    #define rq_bms_essentials 0x0008
    #define rq_bms_cells 0x0010
    #define rq_bms_health 0x0020
    #define rq_bms_versioninfos 0x0040
    #define rq_esc_versioninfos 0x0080
    #define rq_esc_config 0x0100

  //mapping oled screens / requeired data
    #define numscreens 8
    extern uint16_t rqsarray[numscreens];
  extern uint16_t subscribedrequests;
  extern uint16_t housekeeperrequests;
  extern uint8_t hkrequestindex;

    extern uint16_t capacitychargestart;

void reset_statistics(void);
void start_m365(void);
void m365_updatebattstats(void);
void m365_updatestats(void);
void m365_sendesccommand(uint8_t cvalue, uint8_t cparam1, uint8_t cparam2);
void m365_sendx1command(uint8_t beepnum);
void m365_sendrequest(uint8_t radr, uint8_t roffset, uint8_t rlen);
void m365_handlerequests(void);
void m365_handlepacket(void);
void m365_receiver(void);
void handle_housekeeper(void);

#endif