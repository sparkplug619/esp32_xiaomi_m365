#include "telnet.h"
#include "main.h"
#include "strings.h"
#include "wlan.h"
#ifdef headunit
  #include "m365headunit.h"
#else
  #include "m365client.h"
#endif
#include <WiFi.h>
#include <WiFiUdp.h>

#ifdef usengcode
  #include "ngcode.h"
#endif


#ifdef usetelnetserver
  WiFiServer telnetserver(telnet_port_statistics);
  WiFiClient telnetclient;
  #ifdef userawserver
    WiFiServer rawserver(telnet_port_raw);
    WiFiClient rawclient;
  #endif
  #ifdef usepacketserver
    WiFiServer packetserver(telnet_port_packet);
    WiFiClient packetclient;
  #endif
  uint8_t telnetstate = 0;
  uint8_t telnetstateold = 0;
  uint8_t telnetrawstate = 0;
  uint8_t telnetrawstateold = 0;
  
  unsigned long userconnecttimestamp = 0;
  unsigned long telnetnextrefreshtimestamp = 0;
  
  //telnet screens
  uint8_t telnetscreen = ts_statistics;
  uint8_t telnetscreenold = telnetscreen;

  uint16_t tmpi;

  //ANSI
  String ansiPRE  = "\033"; // escape code
  String ansiHOME = "\033[H"; // cursor home
  String ansiESC  = "\033[2J"; // esc
  String ansiCLC  = "\033[?25l"; // invisible cursor
  String ansiEND  = "\033[0m";   // closing tag for styles
  String ansiBOLD = "\033[1m";
  String ansiRED  = "\033[41m"; // red background
  String ansiGRN  = "\033[42m"; // green background
  String ansiBLU  = "\033[44m"; // blue background
  String ansiREDF = "\033[31m"; // red foreground
  String ansiGRNF = "\033[34m"; // green foreground
  String ansiBLUF = "\033[32m"; // blue foreground
  String BELL     = "\a";

  void telnet_refreshscreen() {
    uint8_t k=0;
    //for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (telnetclient && telnetclient.connected()){
          if (telnetscreenold!=telnetscreen) { telnetclient.print(ansiESC); telnetscreenold=telnetscreen; }
          telnetclient.print(ansiHOME);
          telnetclient.print(ansiCLC); 
          switch (telnetscreen) {
            case ts_telemetrie:
                telnetclient.print("M365 Telemetrie Screen"); 
              break;
            case ts_esc_raw:
                telnetclient.print("M365 ESC RAW Screen"); 
              break;
            case ts_ble_raw:
                telnetclient.print("M365 BLE RAW Screen"); 
              break;
            case ts_bms_raw:
                telnetclient.print("M365 BMS RAW Screen"); 
              break;
            case ts_x1_raw:
                telnetclient.print("M365 X1 RAW Screen"); 
              break;
            case ts_statistics:
                telnetclient.print("M365 Statistics Screen");
              break;
          }
          tmpi++;
          if (tmpi % 2) {
            telnetclient.println(" .\r\n");
          } else {
            telnetclient.println("  \r\n");
          }
          switch (telnetscreen) {
            case ts_telemetrie:
                  telnetclient.printf("\r\nBLE\r\n Throttle: %03d Brake %03d\r\n",ptrm365bus->bleparsed->throttle,ptrm365bus->bleparsed->brake);
                  sprintf(tmp1,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c",ptrm365bus->bmsparsed->serial[0],ptrm365bus->bmsparsed->serial[1],ptrm365bus->bmsparsed->serial[2],ptrm365bus->bmsparsed->serial[3],ptrm365bus->bmsparsed->serial[4],ptrm365bus->bmsparsed->serial[5],ptrm365bus->bmsparsed->serial[6],ptrm365bus->bmsparsed->serial[7],ptrm365bus->bmsparsed->serial[8],ptrm365bus->bmsparsed->serial[9],ptrm365bus->bmsparsed->serial[10],ptrm365bus->bmsparsed->serial[11],ptrm365bus->bmsparsed->serial[12],ptrm365bus->bmsparsed->serial[13]);
                  telnetclient.printf("\r\n\r\nBMS Serial: %s Version: %x.%x.%x\r\n", tmp1,ptrm365bus->bmsparsed->fwversion[1],(ptrm365bus->bmsparsed->fwversion[0]&0xf0)>>4,ptrm365bus->bmsparsed->fwversion[0]&0x0f);
                  telnetclient.printf(" Capacity Total: %5d mAh Remaining %5d mAh Percent %03d%%\r\n",ptrm365bus->bmsparsed->totalcapacity, ptrm365bus->bmsparsed->remainingcapacity, ptrm365bus->bmsparsed->remainingpercent);
                  telnetclient.printf(" Temperature1 %3d°C Temperature2 %3d°C Health %05d\r\n",ptrm365bus->bmsparsed->temperature[1]-20, ptrm365bus->bmsparsed->temperature[0]-20, ptrm365bus->bmsparsed->health);
                  telnetclient.printf(" Production Date: %d-%d-%d ",((ptrm365bus->bmsparsed->proddate)&0xFE00)>>9,((ptrm365bus->bmsparsed->proddate)&0x1E0)>>5,(ptrm365bus->bmsparsed->proddate)&0x1f);
                  telnetclient.printf(" Charging Cycles: %d Charging Times: %d\r\n",ptrm365bus->bmsparsed->cycles,ptrm365bus->bmsparsed->chargingtimes);
                  telnetclient.printf(" Voltage: %2.2f V Current %05d mA\r\n",(float)ptrm365bus->bmsparsed->voltage/100.0f, ptrm365bus->bmsparsed->current);
                  telnetclient.printf(" C1: %1.3f C2: %1.3f C3: %1.3f C4: %1.3f C5: %1.3f C6: %1.3f C7: %1.3f C8: %1.3f C9: %1.3f C10: %1.3f\r\n",(float)ptrm365bus->bmsparsed->Cell1Voltage/1000.0f,(float)ptrm365bus->bmsparsed->Cell2Voltage/1000.0f,(float)ptrm365bus->bmsparsed->Cell3Voltage/1000.0f,(float)ptrm365bus->bmsparsed->Cell4Voltage/1000.0f,(float)ptrm365bus->bmsparsed->Cell5Voltage/1000.0f,(float)ptrm365bus->bmsparsed->Cell6Voltage/1000.0f,(float)ptrm365bus->bmsparsed->Cell7Voltage/1000.0f,(float)ptrm365bus->bmsparsed->Cell8Voltage/1000.0f,(float)ptrm365bus->bmsparsed->Cell9Voltage/1000.0f,(float)ptrm365bus->bmsparsed->Cell10Voltage/1000.0f);
                  telnetclient.printf(" Highest: %1.3f Lowest: %1.3f Difference: %1.3f\r\n",(float)ptrm365bus->highest/1000.0f, (float)ptrm365bus->lowest/1000.0f, (float)(ptrm365bus->highest-ptrm365bus->lowest)/1000.0f); 
                  sprintf(tmp1,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c",ptrm365bus->escparsed->serial[0],ptrm365bus->escparsed->serial[1],ptrm365bus->escparsed->serial[2],ptrm365bus->escparsed->serial[3],ptrm365bus->escparsed->serial[4],ptrm365bus->escparsed->serial[5],ptrm365bus->escparsed->serial[6],ptrm365bus->escparsed->serial[7],ptrm365bus->escparsed->serial[8],ptrm365bus->escparsed->serial[9],ptrm365bus->escparsed->serial[10],ptrm365bus->escparsed->serial[11],ptrm365bus->escparsed->serial[12],ptrm365bus->escparsed->serial[13]);
                  telnetclient.printf("\r\n\r\nESC Serial: %s Version: %x.%x.%x\r\n", tmp1,ptrm365bus->escparsed->fwversion[1],(ptrm365bus->escparsed->fwversion[0]&0xf0)>>4,ptrm365bus->escparsed->fwversion[0]&0x0f);
                  telnetclient.printf(" Distance Total: %.2f km Trip: %.2f km Remaining %.2f km\r\n", (float)ptrm365bus->escparsed->totaldistance/1000.0f,(float)ptrm365bus->escparsed->tripdistance/100.0f,(float)ptrm365bus->escparsed->remainingdistance/100.0f);
                  telnetclient.printf(" Power On Time1: %d s, Power On Time1: %d s, Trip Time: %d s\r\n",ptrm365bus->escparsed->ontime1,ptrm365bus->escparsed->ontime2,ptrm365bus->escparsed->triptime);
                  telnetclient.printf(" FrameTemp1: %05d FrameTemp2: %.1f °C\r\n", (float)ptrm365bus->escparsed->frametemp1/10.0f, (float)ptrm365bus->escparsed->frametemp2/10.0f);
                  telnetclient.printf(" Speed: %.2f km/h Avg: %.2f km/h\r\n", (float)ptrm365bus->escparsed->speed/1000.0f, (float)ptrm365bus->escparsed->averagespeed/1000.0f);
                  telnetclient.printf(" Batt Percent: %3d%%\r\n",ptrm365bus->escparsed->battpercent);
                  telnetclient.printf(" Ecomode: %05d Kers: %05d Cruisemode: %05d Taillight: %05d\r\n", ptrm365bus->escparsed->ecomode, ptrm365bus->escparsed->kers, ptrm365bus->escparsed->cruisemode, ptrm365bus->escparsed->taillight);
                  sprintf(tmp1,"%c%c%c%c%c%c",ptrm365bus->escparsed->pin[0],ptrm365bus->escparsed->pin[1],ptrm365bus->escparsed->pin[2],ptrm365bus->escparsed->pin[3],ptrm365bus->escparsed->pin[4],ptrm365bus->escparsed->pin[5]);
                  telnetclient.printf(" Pin: %s\r\n Error %05d Lockstate %02d\r\n",tmp1,ptrm365bus->escparsed->error, ptrm365bus->escparsed->lockstate);
                  telnetclient.printf("\r\n\r\nX1 Mode %03d LEDs %03d Light ",ptrm365bus->x1parsed->mode, ptrm365bus->x1parsed->battleds);
                  if (ptrm365bus->x1parsed->light==0) { telnetclient.print("OFF "); } else {
                    if (ptrm365bus->x1parsed->light==100) { telnetclient.print("ON "); } else {
                     telnetclient.print("    ");
                    }
                  }
                  telnetclient.printf("%03d BeepAction %03d\r\n", ptrm365bus->x1parsed->light, ptrm365bus->x1parsed->beepaction);
                telnetnextrefreshtimestamp=millis()+telnetrefreshanyscreen;
              break;
            case ts_statistics:
                telnetclient.printf("Requests Sent:\r\n  ESC: %05d   BMS: %05d   Total: %05d\r\n", ptrm365bus->requests_sent_esc, ptrm365bus->requests_sent_bms, ptrm365bus->requests_sent_bms+ptrm365bus->requests_sent_esc);
                telnetclient.printf("Commands Sent: ESC: %05d   X1: %0dd   HK-Cycles: %05d\r\n", ptrm365bus->esccommands_sent, ptrm365bus->x1commands_sent, ptrm365bus->requests_sent_housekeeper);
                telnetclient.printf("Packets Received:\r\n  ESC: %05d   BMS: %05d   BLE: %05d   X1: %05d   unhandled: %05d\r\n", ptrm365bus->packets_rec_esc,ptrm365bus->packets_rec_bms,ptrm365bus->packets_rec_ble,ptrm365bus->packets_rec_x1,ptrm365bus->packets_rec_unhandled);
                telnetclient.printf("  CRC OK: %05d   CRC FAIL: %05d\r\n   Total: %05d\r\n",ptrm365bus->packets_crcok,ptrm365bus->packets_crcfail,ptrm365bus->packets_rec);
                telnetclient.printf("\r\nALARM States:\r\n  Cellvoltage Differnce (Is: %d mV, Alert: %d0 mV): %s\r\n",ptrm365bus->highest-ptrm365bus->lowest, conf_alert_batt_celldiff, ptrm365bus->alert_cellvoltage ? "true" : "false");
                telnetclient.printf("  BMS Temperature (Is: %d °C %d °C, Alert: %d °C): %s\r\n",ptrm365bus->bmsparsed->temperature[1]-20, ptrm365bus->bmsparsed->temperature[0]-20,conf_alert_batt_temp, ptrm365bus->alert_bmstemp ? "true" : "false");
                telnetclient.printf("  ESC Temperarture (Is: %3.1f °C, Alert: %d °C): %s\r\n",(float)ptrm365bus->escparsed->frametemp2/10.0f, conf_alert_esc_temp, ptrm365bus->alert_esctemp ? "true" : "false");
                telnetclient.printf("  ALARM Counters: Cellvoltage: %d, BMS Temperature: %d, ESC Temperature %d\r\n", ptrm365bus->alertcounter_cellvoltage, ptrm365bus->alertcounter_bmstemp, ptrm365bus->alertcounter_esctemp);
                telnetclient.printf("\r\nTimers:\r\n  Main:   %04.3f ms\r\n  Telnet: %04.3f ms\r\n",(float)duration_mainloop/1000.0f, (float)duration_telnet/1000.0f);
                telnetclient.printf("  OLED Main: %04.3f ms\r\n",(float)duration_oled/1000.0f);
                telnetclient.printf("  OLED1 Draw: %04.3f ms\r\n  OLED1 Transfer:  %04.3f ms\r\n",(float)duration_oled1draw/1000.0f,(float)duration_oled1transfer/1000.0f);
                telnetclient.printf("  OLED2 Draw: %04.3f ms\r\n  OLED2 Transfer:  %04.3f ms\r\n",(float)duration_oled2draw/1000.0f,(float)duration_oled2transfer/1000.0f);
                telnetclient.printf("  Request Cycle Loop: %04d ms\r\n  Request last Index: %03d\r\n", ptrm365bus->duration_requestcycle,ptrm365bus->requestindex);
                telnetclient.printf("  Time since last Packet: %05d ms\r\n",ptrm365bus->m365packetlasttimestamp);
                telnetclient.printf("\r\nPackets per device Address:\r\n");
                k = 0;
                for(uint16_t j = 0; j <=255; j++) {
                  if (ptrm365bus->packetsperaddress[j]>=1) {
                    k++;
                    telnetclient.printf("%02X -> %05d  ", j, ptrm365bus->packetsperaddress[j]);
                    if ((k % 5)==0) { telnetclient.println(""); }
                  }
                }
                
                #ifdef headunit
                  telnetclient.print("\r\n\r\nQueue Stats:\r\n");
                  telnetclient.printf("QRec Wanted/Ignored: %d / %d\r\n",ptrm365bus->recqueuewanted,ptrm365bus->recqueueignored);
                  telnetclient.printf("QRec  IN  OK/Failed: %d / %d\r\n",ptrm365bus->recqueuein_ok,ptrm365bus->recqueuein_fail);
                  telnetclient.printf("QRec  OUT OK/Failed: %d / %d\r\n",ptrm365bus->recqueueout_ok,ptrm365bus->recqueueout_fail);
                  telnetclient.printf("Qsend IN  OK/Failed: %d / %d\r\n",ptrm365bus->sendqueuein_ok,ptrm365bus->sendqueuein_fail);
                  telnetclient.printf("Qsend OUT OK/Failed: %d / %d\r\n",ptrm365bus->sendqueueout_ok,ptrm365bus->sendqueueout_fail);
                  telnetclient.printf("\r\nCtrl Iterations / Duration / Idle: %d / %d / %d\r\n",ptrm365bus->controller_iterations,ptrm365bus->controller_time_duration,ptrm365bus->controller_time_idle);
                  telnetclient.printf("Disp Iterations / Duration / Idle: %d / %d / %d\r\n",ptrm365bus->display_iterations,ptrm365bus->display_time_duration,ptrm365bus->display_time_idle);
                #endif
                telnetnextrefreshtimestamp=millis()+telnetrefreshanyscreen;           
              break;
            case ts_esc_raw:
                telnetclient.println("     00 01 02 03   04 05 06 07   08 09 0A 0B   0C 0D 0E 0F   10 11 12 13   14 15 16 17   18 19 1A 1B   1C 1D 1E 1F");
                for(uint16_t j = 0; j<=511; j++) {
                    if ((j % 32)==0) { 
                      telnetclient.printf("\r\n%03X: ",j); 
                    } else {
                      if ((j % 4)==0) { 
                        telnetclient.print("- "); 
                      }
                    } //mod32
                    telnetclient.printf("%02X ", ptrm365bus->escdata[j]);
                }
                telnetnextrefreshtimestamp=millis()+telnetrefreshrawarrayscreen;
              break;
            case ts_ble_raw:
                telnetclient.println("     00 01 02 03   04 05 06 07   08 09 0A 0B   0C 0D 0E 0F   10 11 12 13   14 15 16 17   18 19 1A 1B   1C 1D 1E 1F");
                for(uint16_t j = 0; j<=511; j++) {
                    if ((j % 32)==0) { 
                      telnetclient.printf("\r\n%03X: ",j); 
                    } else {
                      if ((j % 4)==0) { 
                        telnetclient.print("- "); 
                      }
                    } //mod32
                    telnetclient.printf("%02X ", ptrm365bus->bledata[j]);
                }
                telnetnextrefreshtimestamp=millis()+telnetrefreshrawarrayscreen;
              break;
            case ts_bms_raw:
                telnetclient.println("     00 01 02 03   04 05 06 07   08 09 0A 0B   0C 0D 0E 0F   10 11 12 13   14 15 16 17   18 19 1A 1B   1C 1D 1E 1F");
                for(uint16_t j = 0; j<=511; j++) {
                    if ((j % 32)==0) { 
                      telnetclient.printf("\r\n%03X: ",j); 
                    } else {
                      if ((j % 4)==0) { 
                        telnetclient.print("- "); 
                      }
                    } //mod32
                    telnetclient.printf("%02X ", ptrm365bus->bmsdata[j]);
                }
                telnetnextrefreshtimestamp=millis()+telnetrefreshrawarrayscreen;
              break;
            case ts_x1_raw:
                telnetclient.println("     00 01 02 03   04 05 06 07   08 09 0A 0B   0C 0D 0E 0F   10 11 12 13   14 15 16 17   18 19 1A 1B   1C 1D 1E 1F");
                for(uint16_t j = 0; j<=511; j++) {
                    if ((j % 32)==0) { 
                      telnetclient.printf("\r\n%03X: ",j); 
                    } else {
                      if ((j % 4)==0) { 
                        telnetclient.print("- "); 
                      }
                    } //mod32
                    telnetclient.printf("%02X ", ptrm365bus->x1data[j]);
                }
                telnetnextrefreshtimestamp=millis()+telnetrefreshrawarrayscreen;
              break;
          }
          yield();
      }
    //} //i 0 to maxclients
    //DebugSerial.printf("---TELNET--- %d\r\n",millis());
  }

  void handle_telnet() {
    timestamp_telnetstart=micros();
    boolean hc = false;
     switch(telnetstate) {
      case telnetoff:
        break;
      case telnetturnon:
          telnetserver.begin();
          telnetserver.setNoDelay(true);
          DebugSerial.print("### Telnet Debug Server @ ");
          if (wlanstate==wlanap) {
            DebugSerial.print(WiFi.softAPIP());  
          } else {
            DebugSerial.print(WiFi.localIP());  
          }
          DebugSerial.println(":36523");
  #ifdef userawserver
          rawserver.begin();
          rawserver.setNoDelay(true);
          DebugSerial.print("### Telnet RAW Server @ ");
          if (wlanstate==wlanap) {
            DebugSerial.print(WiFi.softAPIP());  
          } else {
            DebugSerial.print(WiFi.localIP());  
          }
          DebugSerial.println(":36524");
  #endif        
  #ifdef usepacketserver
          packetserver.begin();
          packetserver.setNoDelay(true);
          DebugSerial.print("### Telnet PACKET Server @ ");
          if (wlanstate==wlanap) {
            DebugSerial.print(WiFi.softAPIP());  
          } else {
            DebugSerial.print(WiFi.localIP());  
          }
          DebugSerial.println(":36525");
  #endif        
          telnetstate = telnetlistening;
          userconnecttimestamp = millis()+userconnecttimeout;  
        break;
      case telnetlistening: 
              if (telnetserver.hasClient()) {
                //for(i = 0; i < MAX_SRV_CLIENTS; i++){
                  //find free/disconnected spot
                  if (!telnetclient || !telnetclient.connected()){
                    if(telnetclient) telnetclient.stop();
                    telnetclient = telnetserver.available();
                    if (!telnetclient) DebugSerial.println("available broken");
                    DebugSerial.print("### Telnet New client: ");
                    DebugSerial.print(i); DebugSerial.print(' ');
                    DebugSerial.println(telnetclient.remoteIP());
                    telnetstate = telnetclientconnected;
                    #ifdef usestatusled
                      ledontime=250; ledofftime=0; ledcurrenttime = millis();
                    #endif
                    telnetnextrefreshtimestamp=millis()+telnetrefreshanyscreen;
                    break;
                  }
              //}
              /*if (i >= MAX_SRV_CLIENTS) {
                //no free/disconnected spot so reject
                telnetserver.available().stop();
                DebugSerial.println("### Telnet rejected connection (max Clients)");
              }*/
            }
  #ifdef userawserver
            if (rawserver.hasClient()) {
              //for(i = 0; i < MAX_SRV_CLIENTS; i++){
                //find free/disconnected spot
                if (!rawclient || !rawclient.connected()){
                  if(rawclient) rawclient.stop();
                  rawclient = rawserver.available();
                  if (!rawclient) DebugSerial.println("available broken");
                  DebugSerial.print("### Telnet RAW New client: ");
                  DebugSerial.println(rawclient.remoteIP());
                  telnetstate = telnetclientconnected;
                  #ifdef usestatusled
                    ledontime=250; ledofftime=0; ledcurrenttime = millis();
                  #endif
                  //telnetnextrefreshtimestamp=millis()+telnetrefreshanyscreen;
                  //break;
                }
              //}
            }
  #endif
  #ifdef usepacketserver
            if (packetserver.hasClient()) {
              //for(i = 0; i < MAX_SRV_CLIENTS; i++){
                //find free/disconnected spot
                if (!packetclient || !packetclient.connected()){
                  if(packetclient) packetclient.stop();
                  packetclient = packetserver.available();
                  if (!packetclient) DebugSerial.println("available broken");
                  DebugSerial.print("### Telnet PACKET New client: ");
                  DebugSerial.println(packetclient.remoteIP());
                  telnetstate = telnetclientconnected;
                  #ifdef usestatusled
                    ledontime=250; ledofftime=0; ledcurrenttime = millis();
                  #endif
                  //telnetnextrefreshtimestamp=millis()+telnetrefreshanyscreen;
                  //break;
                }
              //}
            }
  #endif
            if (userconnecttimestamp<millis()) {
              telnetstate = telnetturnoff;
            }
        break;
      case telnetclientconnected: 
          //TODO check if clients are still connected... else start client connect timer and fall back to listening mode
          hc = false;
          //for(i = 0; i < MAX_SRV_CLIENTS; i++){
            if (telnetclient && telnetclient.connected()) { hc=true; }
          //} //i 0 to maxclients
  #ifdef userawserver
          if (rawclient && rawclient.connected()) { hc=true; }
  #endif
  #ifdef usepacketserver
          if (packetclient && packetclient.connected()) { hc=true; }
  #endif

          if (!hc) {
            //no more clients, restart listening timer....
            telnetstate = telnetturnon;
            DebugSerial.println("### Telnet lost all clients - restarting telnet");
            #ifdef usestatusled
              ledontime=500; ledofftime=500; ledcurrenttime = millis();
            #endif
          } else {
            //still has clients, update telnet stuff
              if (telnetnextrefreshtimestamp<millis()) {
                    telnet_refreshscreen();
              } //telnetrefreshtimer

          } //else !hc
          //check clients for data
          //for(i = 0; i < MAX_SRV_CLIENTS; i++){
            if (telnetclient && telnetclient.connected()){
              if(telnetclient.available()){
                //get data from the telnet client and push it to the UART
                uint8_t tcmd = telnetclient.read();
                DebugSerial.printf("Telnet Command: %02X\r\n",tcmd);
                switch (tcmd) {
                  case 0x73: telnetscreen=ts_statistics; break; //s
                  case 0x74: telnetscreen=ts_telemetrie; break; //t
                  case 0x65: telnetscreen=ts_esc_raw; break; //e
                  case 0x62: telnetscreen=ts_bms_raw; break; //b
                  case 0x6E: telnetscreen=ts_ble_raw; break; //n
                  case 0x78: telnetscreen=ts_x1_raw; break; //x
                  case 0x72: ptrm365bus->reset_statistics(); break;  //r
                } //switch
                //while(serverClients[i].available()) M365DebugSerial.write(serverClients[i].read());
              } //serverclients available
            } //if connected
          //}  //for i
  #ifdef userawserver        
          //TODO: Check RAW Client for Data and send to bus? immediately send or queue and send in next free timeslot?
  #endif        
        break;
      case telnetdisconnectclients:
          if (telnetclient) telnetclient.stop();
          #ifdef userawserver
            if (rawclient) rawclient.stop();
          #endif        
          #ifdef userawserver
            if (packetclient) packetclient.stop();
          #endif
          telnetstate = telnetturnoff;
        break;
      case telnetturnoff:
          #ifdef ESP32
            telnetserver.end();
          #endif
          #if defined userawserver && defined ESP32
            rawserver.end();
          #endif      
          #if defined usepacketserver && defined ESP32
            packetserver.end();
          #endif
          wlanstate=wlanturnoff; 
          telnetstate = telnetoff;
        break;
     } //switch (telnetstate)
     duration_telnet = micros()-timestamp_telnetstart;
  } //handle_telnet


#endif //usetelnetserver

