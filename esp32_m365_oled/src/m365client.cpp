#include "m365client.h"
#include "definitions.h"
#include "display.h"
#include "main.h"
#include "config.h"
//#include "strings.h"
#ifdef usengcode
  #include "ngcode.h"
#endif

#ifdef usetelnetserver
  #include "telnet.h"
#endif

//queuehandles
    QueueHandle_t receivedqueue;
    QueueHandle_t sendqueue;

M365Client::M365Client() {
}

#ifdef usedisplay
  void M365Client::begin(HardwareSerialPatched *uart, M365Display *disp) {
    _disp = disp;
#else
  void M365Client::begin(HardwareSerialPatched *uart) {
#endif
    _uart = uart;
    DebugSerial.printf("Creating queues, rec item size %d, send item size %d\r\n", sizeof(queuebuf),sizeof(queuebuf));
    //create queues for communication controller <> core:
    receivedqueue = xQueueCreate(3, sizeof(queuebuf));
    if(receivedqueue == NULL){
      DebugSerial.println("Error creating the receiver queue");
    }/* else {
      DebugSerial.println("Created Received Queue");
    }*/
    sendqueue = xQueueCreate(3, sizeof(queuebuf)); //escrequest and x1command are longest with 12 bytes
    if(sendqueue == NULL){
      DebugSerial.println("Error creating the Send queue");
    }/* else {
      DebugSerial.println("Created Send Queue");
    }*/
    subscribedrequests=rqsarray[0];
    m365receiverstate = m365receiverready;
    reset_statistics();
  }  //m365::begin

void M365Client::loop() {
  //handle any received data on _uart
    receiver(); 
  //handle received packets
  if (uxQueueMessagesWaiting(receivedqueue)!=0) {
    m365packetlasttimestamp = millis()-m365packettimestamp;
    m365packettimestamp=millis();
    handlepacket(); //wrapper for queuebuffers...
  } else {
    if ((m365packettimestamp+m365packettimeout)>millis()) {
      //packet timeout
    }
  }
  //handle updated buffer data:
  if (newdata) {
      updatestats();
      handlehousekeeper();
  }
  //check if we can prepare the next request
  if (uxQueueMessagesWaiting(sendqueue)==0) {
      handlerequests(); 
  }
} //m365::loop

void M365Client::end() {
  //stop tasks
  //delete queues
} //m365::end
  
void M365Client::reset_statistics() {
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

void M365Client::updatebattstats() {
  //Cell Voltages
    highest = 0;
    lowest =10000;
    if ((conf_battcells>=1) & (bmsparsed->Cell1Voltage>0)) { lowest=_min(lowest,bmsparsed->Cell1Voltage); highest=_max(highest,bmsparsed->Cell1Voltage); }
    if ((conf_battcells>=2) & (bmsparsed->Cell2Voltage>0)) { lowest=_min(lowest,bmsparsed->Cell2Voltage); highest=_max(highest,bmsparsed->Cell2Voltage); }
    if ((conf_battcells>=3) & (bmsparsed->Cell3Voltage>0)) { lowest=_min(lowest,bmsparsed->Cell3Voltage); highest=_max(highest,bmsparsed->Cell3Voltage); }
    if ((conf_battcells>=4) & (bmsparsed->Cell4Voltage>0)) { lowest=_min(lowest,bmsparsed->Cell4Voltage); highest=_max(highest,bmsparsed->Cell4Voltage); }
    if ((conf_battcells>=5) & (bmsparsed->Cell5Voltage>0)) { lowest=_min(lowest,bmsparsed->Cell5Voltage); highest=_max(highest,bmsparsed->Cell5Voltage); }
    if ((conf_battcells>=6) & (bmsparsed->Cell6Voltage>0)) { lowest=_min(lowest,bmsparsed->Cell6Voltage); highest=_max(highest,bmsparsed->Cell6Voltage); }
    if ((conf_battcells>=7) & (bmsparsed->Cell7Voltage>0)) { lowest=_min(lowest,bmsparsed->Cell7Voltage); highest=_max(highest,bmsparsed->Cell7Voltage); }
    if ((conf_battcells>=8) & (bmsparsed->Cell8Voltage>0)) { lowest=_min(lowest,bmsparsed->Cell8Voltage); highest=_max(highest,bmsparsed->Cell8Voltage); }
    if ((conf_battcells>=9) & (bmsparsed->Cell9Voltage>0)) { lowest=_min(lowest,bmsparsed->Cell9Voltage); highest=_max(highest,bmsparsed->Cell9Voltage); }
    if ((conf_battcells>=10) & (bmsparsed->Cell10Voltage>0)) { lowest=_min(lowest,bmsparsed->Cell10Voltage); highest=_max(highest,bmsparsed->Cell10Voltage); }
    if ((conf_battcells>=11) & (bmsparsed->Cell11Voltage>0)) { lowest=_min(lowest,bmsparsed->Cell11Voltage); highest=_max(highest,bmsparsed->Cell11Voltage); }
    if ((conf_battcells>=12) & (bmsparsed->Cell12Voltage>0)) { lowest=_min(lowest,bmsparsed->Cell12Voltage); highest=_max(highest,bmsparsed->Cell12Voltage); }
}

void M365Client::updatestats() {
  //automatic gas learning:
  //Trip
    if (speed_min>escparsed->speed) { speed_min = escparsed->speed; }
    if (speed_max<escparsed->speed) { speed_max = escparsed->speed; }
    if (current_min>bmsparsed->current) { current_min = bmsparsed->current; }
    if (current_max<bmsparsed->current) { current_max = bmsparsed->current; }
    int32_t currentwatt = (int32_t)((float)(bmsparsed->voltage/100.0f)*(float)bmsparsed->current/100.0f);
    if (watt_min>currentwatt) { watt_min = currentwatt; }
    if (watt_max<currentwatt) { watt_max = currentwatt; }
  //Temperatures
    if ((tb1_min>bmsparsed->temperature[0]) | (tb1_min==0)) { tb1_min = bmsparsed->temperature[0]; }
    if (tb1_max<bmsparsed->temperature[0]) { tb1_max = bmsparsed->temperature[0]; }
    if ((tb2_min>bmsparsed->temperature[1]) | (tb2_min==0)) { tb2_min = bmsparsed->temperature[1]; }
    if (tb2_max<bmsparsed->temperature[1]) { tb2_max = bmsparsed->temperature[1]; }
    if ((te_min>escparsed->frametemp2) | (te_min==0)) { te_min = escparsed->frametemp2; }
    if (te_max<escparsed->frametemp2) { te_max = escparsed->frametemp2; }
  //battery cell statistics
    updatebattstats();
  //alert when in locked mode:
    if (escparsed->alert!=0) {
      if (!alert_lockedalarm) {
        alertcounter_lockedalarm++;
        //decide if we use screen_alert oralertpopup here...
        alert_lockedalarm = true;
        #ifdef usedisplay
          _disp->alertpopup((char*)"ALARM", (char*)"SCOOTER LOCKED!", popupalertduration);
        #endif
      }
    } else {
      alert_lockedalarm = false;  
    }
} //m365_updatestats

void M365Client::sendesccommand(uint8_t cvalue, uint8_t cparam1, uint8_t cparam2) {
  queuebuf sendbuffer = {esccommandlen,{0x55,0xAA,0x04,0x20,0x03,0x01,0x02,0x03,0xB7,0xFF}};
  sendbuffer.buffer[esccommand_cmd] = cvalue;
  sendbuffer.buffer[esccommand_value1] = cparam1;
  sendbuffer.buffer[esccommand_value2] = cparam2;
  crccalc = 0x00;
  for(uint8_t i=esccommand_crcstart;i<esccommand_crc1;i++) {
    crccalc=crccalc + sendbuffer.buffer[i];
  }
  crccalc = crccalc ^ 0xffff;
  sendbuffer.buffer[esccommand_crc1]=(uint8_t)(crccalc&0xff);
  sendbuffer.buffer[esccommand_crc2]=(uint8_t)((crccalc&0xff00)>>8);
  //switch2TX;
  //_uart->write((unsigned char*)&esccommand,esccommandlen);
  //switch2RX;
  if (uxQueueSpacesAvailable(sendqueue)>0) {
    //sendbuffer.len = esccommandlen;
    //memcpy((void*)& sendbuffer.buffer[0], (void*)& esccommand[0], esccommandlen);
    xQueueSend(sendqueue, &sendbuffer, 0);
    esccommands_sent++;
    sendqueuein_ok++;
  } else {
    sendqueuein_fail++;
  }
} //m365_sendesccommand

void M365Client::sendx1command(uint8_t beepnum) {
  queuebuf sendbuffer = {x1commandlen,{ 0x55,0xAA,0x06,0x21,0x64,0x00,0x02,0x07,0x00,0x03,0x68,0xff}};
  sendbuffer.buffer[x1command_beepnum] = beepnum;
  crccalc = 0x00;
  for(uint8_t i=x1command_crcstart;i<x1command_crc1;i++) {
    crccalc=crccalc + sendbuffer.buffer[i];
  }
  crccalc = crccalc ^ 0xffff;
  sendbuffer.buffer[x1command_crc1]=(uint8_t)(crccalc&0xff);
  sendbuffer.buffer[x1command_crc2]=(uint8_t)((crccalc&0xff00)>>8);
  if (uxQueueSpacesAvailable(sendqueue)>0) {
    //sendbuffer.len = x1commandlen;
    //memcpy((void*)& sendbuffer.buffer[0], (void*)& x1command[0], x1commandlen);
    xQueueSend(sendqueue, &sendbuffer, 0);
    x1commands_sent++;
    sendqueuein_ok++;
  } else {
    sendqueuein_fail++;
  }
} //m365_sendesccommand

void M365Client::sendrequest(uint8_t radr, uint8_t roffset, uint8_t rlen) {
  
  if (radr==address_bms) {
    queuebuf sendbuffer = {requestbmslen,{ 0x55,0xAA,0x03,0x22,0x01,0x10,0x3A,0xB7,0xFF}};
    sendbuffer.buffer[bms_request_offset] = roffset;
    sendbuffer.buffer[bms_request_len] = rlen;
    crccalc = 0x00;
    for(uint8_t i=bms_request_crcstart;i<bms_request_crc1;i++) {
      crccalc=crccalc + sendbuffer.buffer[i];
    }
    crccalc = crccalc ^ 0xffff;
    sendbuffer.buffer[bms_request_crc1]=(uint8_t)(crccalc&0xff);
    sendbuffer.buffer[bms_request_crc2]=(uint8_t)((crccalc&0xff00)>>8);
    if (uxQueueSpacesAvailable(sendqueue)>0) {
      xQueueSend(sendqueue, &sendbuffer, 0);
      requests_sent_bms++;
      sendqueuein_ok++;
    } else {
      sendqueuein_fail++;
    }
  } //if address address_bms
  if (radr==address_esc) {
    queuebuf sendbuffer = {requestesclen,{ 0x55,0xAA,0x06,0x20,0x61,0x10,0xAC,0x02,0x28,0x27,0xB7,0xFF}};
    sendbuffer.buffer[esc_request_offset] = roffset;
    sendbuffer.buffer[esc_request_len] = rlen;
    sendbuffer.buffer[esc_request_throttle] = bleparsed->throttle;
    sendbuffer.buffer[esc_request_brake] = bleparsed->brake;
    crccalc = 0x00;
    for(uint8_t i=esc_request_crcstart;i<esc_request_crc1;i++) {
      crccalc=crccalc + sendbuffer.buffer[i];
    }
    crccalc = crccalc ^ 0xffff;
    sendbuffer.buffer[esc_request_crc1]=(uint8_t)(crccalc&0xff);
    sendbuffer.buffer[esc_request_crc2]=(uint8_t)((crccalc&0xff00)>>8);
    if (uxQueueSpacesAvailable(sendqueue)>0) {
      xQueueSend(sendqueue, &sendbuffer, 0);
      requests_sent_esc++;
      sendqueuein_ok++;
    } else {
      sendqueuein_fail++;
    }
  } //if address address_esc
} //m365_sendrequest

void M365Client::handlerequests() {
  uint8_t startindex;
  //1st prio: send comands?
  if (sendcommand != cmd_none) {
      switch(sendcommand) {
          case cmd_kers_weak: sendesccommand(0x7b,0,0); break;
          case cmd_kers_medium: sendesccommand(0x7b,1,0); break;
          case cmd_kers_strong: sendesccommand(0x7b,2,0); break;
          case cmd_cruise_on: sendesccommand(0x7c,1,0); break;
          case cmd_cruise_off: sendesccommand(0x7c,0,0); break;
          case cmd_light_on: sendesccommand(0x7d,2,0); break;
          case cmd_light_off: sendesccommand(0x7d,0,0); break;
          case cmd_turnoff: sendesccommand(0x79,01,0); break;
          case cmd_lock: sendesccommand(0x70,01,0); break;
          case cmd_unlock: sendesccommand(0x71,01,0); break;
          case cmd_beep5: sendx1command(0x05); break;
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
          sendrequest(requests[hkrequestindex][0], requests[hkrequestindex][1], requests[hkrequestindex][2]);
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
        sendrequest(requests[requestindex][0], requests[requestindex][1], requests[requestindex][2]);
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

void M365Client::handlepacket() {
  queuebuf receivebuf;
  xQueueReceive(receivedqueue, &receivebuf,0);
  packetsperaddress[receivebuf.buffer[i_address]]++;  
  #ifdef debug_dump_packetdecode
    sprintf(tmp1,"[%03d] PACKET Len %02X Addr %02X HZ %02X Offset %02X CRC %04X Payload: ",m365packetlasttimestamp,receivebuf.len,receivebuf.buffer[i_address],receivebuf.buffer[i_hz],receivebuf.buffer[i_offset], crccalc);
    DebugSerial.print(tmp1);
    for(i = 0; i < receivebuf.len-3; i++){
        DebugSerial.printf("%02X ",receivebuf.buffer[i_payloadstart+i]);
    }
    DebugSerial.println("");
  #endif
  switch (receivebuf.buffer[i_address]) {
    case address_bms:
        packets_rec_bms++;
        memcpy((void*)& bmsdata[receivebuf.buffer[i_offset]<<1], (void*)& receivebuf.buffer[i_payloadstart], receivebuf.len-3);
        newdata=true;
      break;
    case address_esc:
        packets_rec_esc++;
        memcpy((void*)& escdata[receivebuf.buffer[i_offset]<<1], (void*)& receivebuf.buffer[i_payloadstart], receivebuf.len-3);
        //[fixspeed] -> new range: -10km/h to +55km/h, negative speed (backwards) is a positive value.
        if (((receivebuf.buffer[i_offset]<<1)<=0x16B) && (((receivebuf.buffer[i_offset]<<1)+receivebuf.len-3)>=0x16B)) {
          //long version for backup:
            //int16_t *wrongspeed = (int16_t*)&escdata[0x16A];
            //DebugSerial.printf("speedfixed (%d -> ",*wrongspeed);
            //uint16_t realspeed = (*wrongspeed<=-10000?(uint16_t)((int32_t)*wrongspeed+(int32_t)65536):(int16_t)abs(*wrongspeed));
            //DebugSerial.printf("%d -> ",realspeed);
            //escparsed->speed = realspeed;
          //short version
            escparsed->speed = (*(int16_t*)&escdata[0x16A]<=-10000?(uint16_t)((int32_t)*(int16_t*)&escdata[0x16A]+(int32_t)65536):(int16_t)abs(*(int16_t*)&escdata[0x16A]));
        }
        newdata=true;
      break;
    case address_ble:
        packets_rec_ble++;
        memcpy((void*)& bledata[receivebuf.buffer[i_offset]<<1], (void*)& receivebuf.buffer[i_payloadstart], receivebuf.len-3);
        newdata=true;
      break;
    case address_x1:
        packets_rec_x1++;
        memcpy((void*)& x1data[receivebuf.buffer[i_offset]<<1], (void*)& receivebuf.buffer[i_payloadstart], receivebuf.len-3);
        newdata=true;
      break;
    default:
        packets_rec_unhandled++;
      break;
  } //switch
  recqueueout_ok++;
} //m365_handlepacket

void M365Client::receiver() { //receives data until packet is complete
  #ifndef headunit
    queuebuf tosendbuffer;
  #endif
  uint8_t newbyte;
  while (_uart->available()) {
    newbyte = _uart->read();
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
          receiverbuffer.len = newbyte+1; //next byte will be 1st in recbuf, this is the packet-type1, len counted from 2nd byte in recbuf
          crccalc=newbyte;
          readindex=0;
          m365receiverstate = m365receiverpacket3;
        break;
      case m365receiverpacket3: //we are receiving the payload
          receiverbuffer.buffer[readindex]=newbyte;
          readindex++;
          crccalc=crccalc+newbyte;
          if (readindex==receiverbuffer.len) {
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
              sprintf(tmp1,"55 AA %02X ", receiverbuffer.len-1);
              sprintf(tmp2,"%02X %02X [CRCCalc:%04X]\r\n", crc1,crc2,crccalc);
              switch (receiverbuffer.buffer[i_address]) {
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
              for(i = 0; i < receiverbuffer.len; i++){
                packetclient.printf("%02X ",receiverbuffer.buffer[i]);
              } //for i...
              packetclient.print(tmp2);
            }
          #endif
          #ifdef debug_dump_rawpackets
            sprintf(tmp1,"Packet received: 55 AA %02X ", receiverbuffer.len-1);
            sprintf(tmp2,"CRC %02X %02X %04X\r\n", crc1,crc2,crccalc);
            DebugSerial.print(tmp1);
            for(i = 0; i < receiverbuffer.len; i++){
             DebugSerial.printf("%02X ",receiverbuffer.buffer[i]);
            }
            DebugSerial.print(tmp2);
          #endif
          packets_rec++;
          if (crccalc==((uint16_t)(crc2<<8)+(uint16_t)crc1)) {
            packets_crcok++;
            //queue packet for core
              if (uxQueueSpacesAvailable(receivedqueue)>0) {
                xQueueSend(receivedqueue,&receiverbuffer,0);
                recqueuein_ok++;
              } else {
                recqueuein_fail++;
              }
          } else {
            packets_crcfail++;
          }
        m365receiverstate = m365receiverready; //reset and wait for next packet
        #ifndef headunit
          if (receiverbuffer.buffer[i_address]==0x20 && receiverbuffer.buffer[i_hz]==0x65 && receiverbuffer.buffer[i_offset]==0x00 && conf_espbusmode) {
            //senddata = true;
            //handlerequests();
            //DebugSerial.printf("---REQUEST-- %d\r\n",millis());
            //check if we have to send a packet from core:
              if (uxQueueMessagesWaiting(sendqueue)>0) {
                //readqueueitem and send	
                digitalWrite(debuggpio1,HIGH);
                xQueueReceive(sendqueue,&tosendbuffer,0);
                _uart->write(tosendbuffer.buffer, tosendbuffer.len);
                sendqueueout_ok++;
                digitalWrite(debuggpio1,LOW);
              } else {
                sendqueueout_fail++;
              }
          }
        #endif //headunit
        break;
    } //switch
    //DebugSerial.printf("#S# %d\r\n",M365Serial.available());
  }//serial available
} //m365_receiver

void M365Client::handlehousekeeper() {
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
    //alarm in locked-mode is checked on every data frame -> see m365_updatestats
    //check error register
      if (escparsed->error!=0) {
        if (!alert_escerror) {
          alertcounter_escerror++;
          alert_escerror = true;
          sprintf(tmp1,"%d (error screen!)",escparsed->error);
          #ifdef usedisplay
            _disp->alertpopup((char*)"ESC ERROR", tmp1, popupalertduration);
          #endif
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
            #ifdef usedisplay
              _disp->alertpopup((char*)"LOW BATTERY", tmp1, popupalertduration);
            #endif
          }
        } else {
          alert_undervoltage=false;  
        }
      }
    //cell voltage difference alarm:
      updatebattstats();
      if(highest!=lowest) {
        if ((highest-lowest) >= conf_alert_batt_celldiff*10) {
          if (!alert_cellvoltage) {
            alertcounter_cellvoltage++;
            alert_cellvoltage = true;
            sprintf(tmp1,"%d > %d",(highest-lowest),conf_alert_batt_celldiff*10);
            #ifdef usedisplay
              _disp->alertpopup((char*)"CELL ALERT", tmp1, popupalertduration);
            #endif
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
          #ifdef usedisplay
            _disp->alertpopup((char*)"BATT TEMP", tmp1, popupalertduration);
          #endif
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
            #ifdef usedisplay
              _disp->alertpopup((char*)"ESC TEMP", tmp1, popupalertduration);
            #endif
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
