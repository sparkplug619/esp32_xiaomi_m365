#include "m365.h"

HardwareSerial M365Serial(2);  // UART2 for M365

uint8_t bledata[512];
uint8_t x1data[512];
uint8_t escdata[512];
uint8_t bmsdata[512];

blestruct* bleparsed = (blestruct*)bledata;
x1struct* x1parsed = (x1struct*)x1data;
escstruct* escparsed = (escstruct*)escdata;
bmsstruct* bmsparsed = (bmsstruct*)bmsdata;


bool newdata = false; //flag - we have update at least one byte in one of the data arrays
bool senddata = false; //flag - we should send our data request _now_

uint8_t hkstate = 0;
uint8_t hkstateold=0;
unsigned long housekeepertimestamp=0;
uint8_t sendcommand = 0;
uint8_t x1command[x1commandlen] = { 0x55,0xAA,0x06,0x21,0x64,0x00,0x02,0x07,0x00,0x03,0x68,0xff};
uint8_t esccommand[esccommandlen] = { 0x55,0xAA,0x04,0x20,0x03,0x01,0x02,0x03,0xB7,0xFF};

unsigned long m365packettimestamp = 0;
unsigned long m365packetlasttimestamp = 0;
unsigned long duration_requestcycle=0;
unsigned long timestamp_requeststart=0;

uint8_t m365packetstate = 0;
uint8_t m365packetstateold = 0;

uint8_t crc1=0;
uint8_t crc2=0;
uint16_t crccalc=0;
uint8_t sbuf[maxlen];
uint8_t len=0;
uint8_t readindex=0;
uint8_t m365receiverstate = 0;
uint8_t m365receiverstateold = 0;


//M365 - Statistics
  uint16_t packets_rec=0;
  uint16_t packets_crcok=0;
  uint16_t packets_crcfail=0;
  uint16_t packets_rec_bms=0;
  uint16_t packets_rec_esc=0;
  uint16_t packets_rec_x1=0;
  uint16_t packets_rec_ble=0;
  uint16_t packets_rec_unhandled=0;

  uint16_t packetsperaddress[256];
  uint16_t requests_sent_bms=0;
  uint16_t requests_sent_esc=0;
  uint16_t esccommands_sent=0;
  uint16_t x1commands_sent=0;
  uint16_t requests_sent_housekeeper=0;

  int16_t speed_min = 0;
  int16_t speed_max = 0;
  int16_t current_min = 0;
  int16_t current_max = 0;
  int32_t watt_min = 0;
  int32_t watt_max = 0;
  uint8_t tb1_min = 255; //temperature batt 1 min
  uint8_t tb1_max = 0; //temperature batt 1 max
  uint8_t tb2_min = 255;  //temperature batt 2 min
  uint8_t tb2_max = 0;  //temperature batt 2 max
  uint16_t te_min = 1000;  //temperature esc min
  uint16_t te_max = 0;  //temperature esc max
  uint16_t lowest=10000; //Cell Voltages - Lowest
  uint16_t highest=0; //Cell Voltages - Highest

//alerts
  bool alert_escerror = false;
  bool alert_cellvoltage = false;
  bool alert_bmstemp = false;
  bool alert_esctemp = false;
  bool alert_undervoltage = false;
  uint16_t alertcounter_escerror = 0;
  uint16_t alertcounter_cellvoltage = 0;
  uint16_t alertcounter_bmstemp = 0;
  uint16_t alertcounter_esctemp = 0;
  uint16_t alertcounter_undervoltage = 0;

//scooter-config stuff
  float wheelfact = 0;

//misc
  uint16_t capacitychargestart = 0;

void reset_statistics() {
  packets_rec=0;
  packets_crcok=0;
  packets_crcfail=0;
  for(uint16_t j = 0; j<=255; j++) {
    packetsperaddress[j]=0;  
  }
  for(uint16_t j = 0; j<=511; j++) {
    escdata[j]=0;  
    bmsdata[j]=0;  
    bledata[j]=0;  
    x1data[j]=0;  
  }
  speed_min = 0;
  speed_max = 0;
  current_min = 0;
  current_max = 0;
  watt_min = 0;
  watt_max = 0;
  tb1_min = 255;
  tb1_max = 0;
  tb2_min = 255;
  tb2_max = 0;
  te_min = 1000;
  te_max = 0;
} //reset_statistics


uint8_t request_bms[requestbmslen] = { 0x55,0xAA,0x03,0x22,0x01,0x10,0x3A,0xB7,0xFF};
uint8_t request_esc[requestesclen] = { 0x55,0xAA,0x06,0x20,0x61,0x10,0xAC,0x02,0x28,0x27,0xB7,0xFF};
uint8_t requestindex = 0;
uint8_t requests[requestmax][3]= {
        { address_esc, 0xB0, 0x20}, //error, lockstate, battpercent,speed,averagespeed,totaldistance,tripdistance,ontime2,frametemp2
        { address_esc, 0x25, 0x02}, //remaining distance
        { address_esc, 0x3A, 0x0A}, //ontime, triptime, frametemp1
        { address_bms, 0x31, 0x0A}, //remaining cap & percent, current, voltage, temperature
        { address_bms, 0x40, 0x18}, //cell voltages (Cells 1-10 & 11-12 for 12S Batt/Custom BMS)
        { address_bms, 0x3B, 0x02}, //Battery Health
        { address_bms, 0x10, 0x22}, //serial,fwversion,totalcapacity,cycles,chargingtimes,productiondate
        { address_esc, 0x10, 0x16}, //serial,pin,fwversion
        { address_esc, 0x7B, 0x06}  //kers, cruisemode, taillight
    };

       uint16_t rqsarray[numscreens] = {
      rq_esc_essentials|rq_esc_remdist|rq_esc_essentials2|rq_bms_essentials|rq_bms_cells|rq_bms_health|rq_bms_versioninfos|rq_esc_versioninfos, //rq_esc_essentials|rq_bms_essentials|rq_esc_essentials2, //screen_stop
      //0xff, //screen_stop, TODO: Request infos like Serial/FW Version only _once_ (when entering subscreen)
      rq_esc_essentials|rq_bms_essentials, //screen_drive
      rq_esc_essentials, //screen_error
      rq_esc_essentials, //screen_timeout
      rq_esc_essentials|rq_bms_essentials|rq_bms_cells, //charging
      rq_esc_essentials|rq_esc_config, //configmenu
      rq_esc_essentials|rq_esc_essentials2|rq_bms_essentials, //alarm
      rq_esc_essentials //screen_locked
    };

  uint16_t subscribedrequests =  rqsarray[0];
  uint16_t housekeeperrequests = rq_bms_cells|rq_esc_essentials|rq_bms_essentials;
  uint8_t hkrequestindex=0;


void start_m365() {
  subscribedrequests=rqsarray[0];
  M365SerialFull
  m365receiverstate = m365receiverready;
  m365packetstate=m365packetidle;
  reset_statistics();
}  //startm365

void m365_updatebattstats() {
  //Cell Voltages
    highest = 0;
    lowest =10000;
    if (conf_battcells>=1 & bmsparsed->Cell1Voltage>0) { lowest=_min(lowest,bmsparsed->Cell1Voltage); highest=_max(highest,bmsparsed->Cell1Voltage); }
    if (conf_battcells>=2 & bmsparsed->Cell2Voltage>0) { lowest=_min(lowest,bmsparsed->Cell2Voltage); highest=_max(highest,bmsparsed->Cell2Voltage); }
    if (conf_battcells>=3 & bmsparsed->Cell3Voltage>0) { lowest=_min(lowest,bmsparsed->Cell3Voltage); highest=_max(highest,bmsparsed->Cell3Voltage); }
    if (conf_battcells>=4 & bmsparsed->Cell4Voltage>0) { lowest=_min(lowest,bmsparsed->Cell4Voltage); highest=_max(highest,bmsparsed->Cell4Voltage); }
    if (conf_battcells>=5 & bmsparsed->Cell5Voltage>0) { lowest=_min(lowest,bmsparsed->Cell5Voltage); highest=_max(highest,bmsparsed->Cell5Voltage); }
    if (conf_battcells>=6 & bmsparsed->Cell6Voltage>0) { lowest=_min(lowest,bmsparsed->Cell6Voltage); highest=_max(highest,bmsparsed->Cell6Voltage); }
    if (conf_battcells>=7 & bmsparsed->Cell7Voltage>0) { lowest=_min(lowest,bmsparsed->Cell7Voltage); highest=_max(highest,bmsparsed->Cell7Voltage); }
    if (conf_battcells>=8 & bmsparsed->Cell8Voltage>0) { lowest=_min(lowest,bmsparsed->Cell8Voltage); highest=_max(highest,bmsparsed->Cell8Voltage); }
    if (conf_battcells>=9 & bmsparsed->Cell9Voltage>0) { lowest=_min(lowest,bmsparsed->Cell9Voltage); highest=_max(highest,bmsparsed->Cell9Voltage); }
    if (conf_battcells>=10 & bmsparsed->Cell10Voltage>0) { lowest=_min(lowest,bmsparsed->Cell10Voltage); highest=_max(highest,bmsparsed->Cell10Voltage); }
    if (conf_battcells>=11 & bmsparsed->Cell11Voltage>0) { lowest=_min(lowest,bmsparsed->Cell11Voltage); highest=_max(highest,bmsparsed->Cell11Voltage); }
    if (conf_battcells>=12 & bmsparsed->Cell12Voltage>0) { lowest=_min(lowest,bmsparsed->Cell12Voltage); highest=_max(highest,bmsparsed->Cell12Voltage); }
}

void m365_updatestats() {
  //automatic gas learning:
    if (bleparsed->throttle!=0) {
      if (throttlemin>bleparsed->throttle) { 
        throttlemin=bleparsed->throttle; 
        stopwindowsize = (uint8_t)((throttlemax-throttlemin)/stopsubscreens);
        chargewindowsize = (uint8_t)((throttlemax-throttlemin)/chargesubscreens);
        configwindowsize = (uint8_t)((throttlemax-throttlemin)/(configsubscreens-1));
      }
      if (throttlemax<bleparsed->throttle) { 
        throttlemax=bleparsed->throttle;
        stopwindowsize = (uint8_t)((throttlemax-throttlemin)/stopsubscreens);
        chargewindowsize = (uint8_t)((throttlemax-throttlemin)/chargesubscreens);
        configwindowsize = (uint8_t)((throttlemax-throttlemin)/(configsubscreens-1));
      }

    }
  //Trip
    if (speed_min>escparsed->speed) { speed_min = escparsed->speed; }
    if (speed_max<escparsed->speed) { speed_max = escparsed->speed; }
    if (current_min>bmsparsed->current) { current_min = bmsparsed->current; }
    if (current_max<bmsparsed->current) { current_max = bmsparsed->current; }
    int32_t currentwatt = (int32_t)((float)(bmsparsed->voltage/100.0f)*(float)bmsparsed->current/100.0f);
    if (watt_min>currentwatt) { watt_min = currentwatt; }
    if (watt_max<currentwatt) { watt_max = currentwatt; }
  //Temperatures
    if (tb1_min>bmsparsed->temperature[0] | tb1_min==0) { tb1_min = bmsparsed->temperature[0]; }
    if (tb1_max<bmsparsed->temperature[0]) { tb1_max = bmsparsed->temperature[0]; }
    if (tb2_min>bmsparsed->temperature[1] | tb2_min==0) { tb2_min = bmsparsed->temperature[1]; }
    if (tb2_max<bmsparsed->temperature[1]) { tb2_max = bmsparsed->temperature[1]; }
    if (te_min>escparsed->frametemp2 | te_min==0) { te_min = escparsed->frametemp2; }
    if (te_max<escparsed->frametemp2) { te_max = escparsed->frametemp2; }
  m365_updatebattstats();
} //m365_updatestats

void m365_sendesccommand(uint8_t cvalue, uint8_t cparam1, uint8_t cparam2) {
  esccommand[esccommand_cmd] = cvalue;
  esccommand[esccommand_value1] = cparam1;
  esccommand[esccommand_value2] = cparam2;
  crccalc = 0x00;
  for(uint8_t i=esccommand_crcstart;i<esccommand_crc1;i++) {
    crccalc=crccalc + esccommand[i];
  }
  crccalc = crccalc ^ 0xffff;
  esccommand[esccommand_crc1]=(uint8_t)(crccalc&0xff);
  esccommand[esccommand_crc2]=(uint8_t)((crccalc&0xff00)>>8);
  M365Serial.write((unsigned char*)&esccommand,esccommandlen);
  esccommands_sent++;
} //m365_sendesccommand


void m365_sendx1command(uint8_t beepnum) {
  x1command[x1command_beepnum] = beepnum;
  crccalc = 0x00;
  for(uint8_t i=x1command_crcstart;i<x1command_crc1;i++) {
    crccalc=crccalc + x1command[i];
  }
  crccalc = crccalc ^ 0xffff;
  x1command[x1command_crc1]=(uint8_t)(crccalc&0xff);
  x1command[x1command_crc2]=(uint8_t)((crccalc&0xff00)>>8);
  M365Serial.write((unsigned char*)&x1command,x1commandlen);
  x1commands_sent++;
} //m365_sendesccommand

void m365_sendrequest(uint8_t radr, uint8_t roffset, uint8_t rlen) {
    if (radr==address_bms) {
    request_bms[bms_request_offset] = roffset;
    request_bms[bms_request_len] = rlen;
    crccalc = 0x00;
    for(uint8_t i=bms_request_crcstart;i<bms_request_crc1;i++) {
      crccalc=crccalc + request_bms[i];
    }
    crccalc = crccalc ^ 0xffff;
    request_bms[bms_request_crc1]=(uint8_t)(crccalc&0xff);
    request_bms[bms_request_crc2]=(uint8_t)((crccalc&0xff00)>>8);
    M365Serial.write((unsigned char*)&request_bms,requestbmslen);
    requests_sent_bms++;
  } //if address address_bms
  if (radr==address_esc) {
    request_esc[esc_request_offset] = roffset;
    request_esc[esc_request_len] = rlen;
    request_esc[esc_request_throttle] = bleparsed->throttle;
    request_esc[esc_request_brake] = bleparsed->brake;
    crccalc = 0x00;
    for(uint8_t i=esc_request_crcstart;i<esc_request_crc1;i++) {
      crccalc=crccalc + request_esc[i];
    }
    crccalc = crccalc ^ 0xffff;
    request_esc[esc_request_crc1]=(uint8_t)(crccalc&0xff);
    request_esc[esc_request_crc2]=(uint8_t)((crccalc&0xff00)>>8);
    M365Serial.write((unsigned char*)&request_esc,requestesclen);
    requests_sent_esc++;
  } //if address address_esc
} //m365_sendrequest

void m365_handlerequests() {
  uint8_t startindex;
  //1st prio: send comands?
  if (sendcommand != cmd_none) {
      switch(sendcommand) {
          case cmd_kers_weak: m365_sendesccommand(0x7b,0,0); break;
          case cmd_kers_medium: m365_sendesccommand(0x7b,1,0); break;
          case cmd_kers_strong: m365_sendesccommand(0x7b,2,0); break;
          case cmd_cruise_on: m365_sendesccommand(0x7c,1,0); break;
          case cmd_cruise_off: m365_sendesccommand(0x7c,0,0); break;
          case cmd_light_on: m365_sendesccommand(0x7d,2,0); break;
          case cmd_light_off: m365_sendesccommand(0x7d,0,0); break;
          case cmd_turnoff: m365_sendesccommand(0x79,01,0); break;
          case cmd_lock: m365_sendesccommand(0x70,01,0); break;
          case cmd_unlock: m365_sendesccommand(0x71,01,0); break;
          case cmd_beep5: m365_sendx1command(0x05); break;
            break;
        } //switch sendcommand
    sendcommand = cmd_none; //reset  
  } else {
    //2nd prio: housekeeper todos?
        if (hkstate==hkrequesting) {
          //find next subscribed index
          startindex = hkrequestindex;
          while (!(housekeeperrequests&(1<<hkrequestindex)))  {
            //DebugSerial.printf("HK skipping RQ index %d\r\n",hkrequestindex+1);
            hkrequestindex++;
            if (hkrequestindex==requestmax) {
                //DebugSerial.println("HK rollover in rqloop1");
                hkrequestindex=0;
                hkstate=hkevaluating; //we are done with one request-cycle, evaluate... 
            }
            if (hkrequestindex==startindex) { break; }
          }
        //request data for current index if we are interested in
        if (housekeeperrequests&(1<<hkrequestindex)) {
          //DebugSerial.printf("HK requesting RQ index %d\r\n",hkrequestindex+1);
          m365_sendrequest(requests[hkrequestindex][0], requests[hkrequestindex][1], requests[hkrequestindex][2]);
        } /*else {
          DebugSerial.printf("HK skipping RQ index %d\r\n",hkrequestindex+1);
        }*/
        //prepare next requestindex for next call
        hkrequestindex++;
        if (hkrequestindex==requestmax) { 
          //DebugSerial.println("HK rollover in rqloop2");
          hkrequestindex=0; 
          hkstate=hkevaluating; //we are done with one request-cycle, evaluate... 
        }
        //DebugSerial.printf("---HKREQUEST-- %d\r\n",millis());
      } else {
      //3rd prio - request other data for display
      //find next subscribed index
        startindex = requestindex;
        while (!(subscribedrequests&(1<<requestindex)))  {
          //DebugSerial.printf("skipping RQ index %d\r\n",requestindex+1);
          requestindex++;
          if (requestindex==requestmax) {
              //DebugSerial.println("rollover in rqloop1");
              requestindex=0; 
              duration_requestcycle=millis()-timestamp_requeststart;
              timestamp_requeststart=millis(); 
          }
          if (requestindex==startindex) { break; }
        }
      //request data for current index if we are interested in
      if (subscribedrequests&(1<<requestindex)) {
        //DebugSerial.printf("requesting RQ index %d\r\n",requestindex+1);
        m365_sendrequest(requests[requestindex][0], requests[requestindex][1], requests[requestindex][2]);
      } /*else {
        DebugSerial.printf("skipping RQ index %d\r\n",requestindex+1);
      }*/
      //prepare next requestindex for next call
      requestindex++;
      if (requestindex==requestmax) { 
        //DebugSerial.println("rollover in rqloop2");
        requestindex=0; 
        duration_requestcycle=millis()-timestamp_requeststart;
        timestamp_requeststart=millis();
      }
      //DebugSerial.printf("---REQUEST-- %d\r\n",millis());
    } //else if hkstate
  } //else if sendcommand
} //m365_handlerequests

void m365_handlepacket() {
 if (m365packetstate==m365newpacket) {
    m365packetlasttimestamp = millis()-m365packettimestamp;
    m365packettimestamp=millis();
    packetsperaddress[sbuf[i_address]]++;  
    #ifdef debug_dump_packetdecode
      sprintf(tmp1,"[%03d] PACKET Len %02X Addr %02X HZ %02X Offset %02X CRC %04X Payload: ",m365packetlasttimestamp,len,sbuf[i_address],sbuf[i_hz],sbuf[i_offset], crccalc);
      DebugSerial.print(tmp1);
      for(i = 0; i < len-3; i++){
        DebugSerial.printf("%02X ",sbuf[i_payloadstart+i]);
      }
      DebugSerial.println("");
    #endif
    switch (sbuf[i_address]) {
      case address_bms:
          packets_rec_bms++;
          memcpy((void*)& bmsdata[sbuf[i_offset]<<1], (void*)& sbuf[i_payloadstart], len-3);
        break;
      case address_esc:
          packets_rec_esc++;
          memcpy((void*)& escdata[sbuf[i_offset]<<1], (void*)& sbuf[i_payloadstart], len-3);
        break;
      case address_ble:
          packets_rec_ble++;
          memcpy((void*)& bledata[sbuf[i_offset]<<1], (void*)& sbuf[i_payloadstart], len-3);
        break;
      case address_x1:
          packets_rec_x1++;
          memcpy((void*)& x1data[sbuf[i_offset]<<1], (void*)& sbuf[i_payloadstart], len-3);
        break;
      default:
          packets_rec_unhandled++;
        break;
    }
    newdata=true;
    m365packetstate=m365packetidle;
 } //if (m365packetstate==m365newpacket)
 else {
  if ((m365packettimestamp+m365packettimeout)>millis()) {
    //packet timeout
  }
 }
} //m365_handlepacket
  
void m365_receiver() { //recieves data until packet is complete
  uint8_t newbyte;
  if (M365Serial.available()) {
    newbyte = M365Serial.read();
#ifdef userawserver
     if (rawclient && rawclient.connected()) { rawclient.write(newbyte); }
#endif    
    switch(m365receiverstate) {
      case m365receiverready: //we are waiting for 1st byte of packet header 0x55
          if (newbyte==0x55) {
            m365receiverstate = m365receiverpacket1;
          }
        break;
      case m365receiverpacket1: //we are waiting for 2nd byte of packet header 0xAA
          if (newbyte==0xAA) {
            m365receiverstate = m365receiverpacket2;
            }
        break;
      case m365receiverpacket2: //we are waiting for packet length
          len = newbyte+1; //next byte will be 1st in sbuf, this is the packet-type1, len counted from 2nd byte in sbuf
          crccalc=newbyte;
          readindex=0;
          m365receiverstate = m365receiverpacket3;
        break;
      case m365receiverpacket3: //we are receiving the payload
          sbuf[readindex]=newbyte;
          readindex++;
          crccalc=crccalc+newbyte;
          if (readindex==len) {
            m365receiverstate = m365receiverpacket4;
          }
        break;
      case m365receiverpacket4: //we are waiting for 1st CRC byte
          crc1 = newbyte;
          m365receiverstate = m365receiverpacket5;
        break;
      case m365receiverpacket5: //we are waiting for 2nd CRC byte
          crc2 = newbyte;
          crccalc = crccalc ^ 0xffff;
          #ifdef usepacketserver
            if (packetclient && packetclient.connected()) { 
              sprintf(tmp1,"55 AA %02X ", len-1);
              sprintf(tmp2,"%02X %02X [CRCCalc:%04X]\r\n", crc1,crc2,crccalc);
              switch (sbuf[i_address]) {
                case address_bms:
                    packetclient.print(ansiRED);
                    packetclient.print("BMS: ");
                  break;
                case address_esc:
                    packetclient.print(ansiGRN);
                    packetclient.print("ESC: ");
                  break;
                case address_ble:
                    packetclient.print(ansiBLU);
                    packetclient.print("BLE: ");
                  break;
                case address_x1:
                    packetclient.print(ansiBLUF);
                    packetclient.print("X1 : ");
                  break;
                default:
                    packetclient.print(ansiREDF);
                    packetclient.print("???: ");
                  break;
                } //switch                else {
              packetclient.print(ansiEND);
              packetclient.print(tmp1);
              for(i = 0; i < len; i++){
                packetclient.printf("%02X ",sbuf[i]);
              } //for i...
              packetclient.print(tmp2);
            }
          #endif
          #ifdef debug_dump_rawpackets
            sprintf(tmp1,"Packet received: 55 AA %02X ", len-1);
            sprintf(tmp2,"CRC %02X %02X %04X\r\n", crc1,crc2,crccalc);
            DebugSerial.print(tmp1);
            for(i = 0; i < len; i++){
             DebugSerial.printf("%02X ",sbuf[i]);
            }
            DebugSerial.print(tmp2);
          #endif
          packets_rec++;
          if (crccalc==((uint16_t)(crc2<<8)+(uint16_t)crc1)) {
            m365packetstate = m365newpacket;
            packets_crcok++;
          } else {
            packets_crcfail++;
          }
        m365receiverstate = m365receiverready; //reset and wait for next packet
        if (sbuf[i_address]==0x20 && sbuf[i_hz]==0x65 && sbuf[i_offset]==0x00 && conf_espbusmode) {
          //senddata = true;
          m365_handlerequests();
          DebugSerial.printf("---REQUEST-- %d\r\n",millis());
        }
        break;
    } //switch
    //DebugSerial.printf("#S# %d\r\n",M365Serial.available());
  }//serial available
} //m365_receiver

void handle_housekeeper() {
  if (hkstate!=hkstateold) {
    hkstateold=hkstate;
    #ifdef debug_dump_states
      Serial.printf("### HKSTATE %d -> %d\r\n",hkstateold,hkstate);
      print_states();
    #endif
  }
  switch(hkstate) {
    case hkwaiting: //check timeout, if over request data
        if (millis()>housekeepertimestamp) {
          hkstate = hkrequesting;
          hkrequestindex = 0; //start new hk requestcycle
          requests_sent_housekeeper++;
          //set requestor stuff
        }
      break;
    case hkrequesting: //waiting until we received data...
    break;
  case hkevaluating: //evaluate alarms,... rearm timeout
    //check error register
      if (escparsed->error!=0) {
        if (!alert_escerror) {
          alertcounter_escerror++;
          alert_escerror = true;
          sprintf(tmp1,"%d (eror screen!)",escparsed->error);
          alertpopup((char*)"ESC ERROR", tmp1, popupalertduration);

        }
      } else {
        alert_escerror = false;  
      }
    //batt low voltage alarm:
      if (bmsparsed->voltage!=0) {
        if ((uint8_t)((float)bmsparsed->voltage/100.0f) < conf_alert_batt_voltage) {
          if (!alert_undervoltage) {
            alertcounter_undervoltage++;
            alert_undervoltage=true;
            sprintf(tmp1,"%f < %d",(float)bmsparsed->voltage/100.0f,conf_alert_batt_voltage);
            alertpopup((char*)"LOW BATTERY", tmp1, popupalertduration);
          }
        } else {
          alert_undervoltage=false;  
        }
      }
    //cell voltage difference alarm:
      m365_updatebattstats();
      if(highest!=lowest) {
        if ((highest-lowest) >= conf_alert_batt_celldiff*10) {
          if (!alert_cellvoltage) {
            alertcounter_cellvoltage++;
            alert_cellvoltage = true;
            sprintf(tmp1,"%d > %d",(highest-lowest),conf_alert_batt_celldiff*10);
            alertpopup((char*)"CELL ALERT", tmp1, popupalertduration);
            //popup((char*)"CELL ALERT", (char*)"Volt Difference", 5000);
          }
        } else {
          alert_cellvoltage = false;
        }
      }
    //bms temp alarm
      if (((bmsparsed->temperature[1]-20) >=conf_alert_batt_temp) | ((bmsparsed->temperature[0]-20) >= conf_alert_batt_temp)) {
        if (!alert_bmstemp) {
          alertcounter_bmstemp++;
          alert_bmstemp = true;
          sprintf(tmp1,"%d / %d > %d",(bmsparsed->temperature[0]-20),(bmsparsed->temperature[1]-20), conf_alert_batt_temp);
          alertpopup((char*)"BATT TEMP", tmp1, popupalertduration);
          //popup((char*)"BATT TEMP", (char*)"Temp over limit!", 5000);
        }
      } else {
        alert_bmstemp = false;
      }
    //esc temp alarm
      if (escparsed->frametemp2!=0) {
        if (((float)escparsed->frametemp2/10.0f) >= (float)conf_alert_esc_temp) {
          if (!alert_esctemp) {
            alertcounter_esctemp++;
            alert_esctemp = true;
            sprintf(tmp1,"%f > %d",(float)escparsed->frametemp2/10.0f, conf_alert_esc_temp);
            alertpopup((char*)"ESC TEMP", tmp1, popupalertduration);
          }
        } else {
          alert_esctemp = false;
        }
      }
    //last step: rearm
      housekeepertimestamp = millis() + housekeepertimeout;
      hkstate = hkwaiting;
      //popup((char*)"hk title", (char*)"hk text", 1000);
    break;
  } //switch hkstate
} //handle_housekeeper
 