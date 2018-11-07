#include "display.h"
#include "strings.h"

#if (defined usei2c && defined useoled1)
    Adafruit_SSD1306 display1(oled_reset);
    Adafruit_SSD1306* displaydraw;
#endif
#if (defined usei2c && defined useoled2)
    Adafruit_SSD1306 display2(oled_reset);
#endif

#if (!defined usei2c && defined useoled1)
    //software spi Adafruit_SSD1306 display1(OLED_MOSI, OLED_CLK, OLED_DC, OLED1_RESET, OLED1_CS);
    //hardware spi with patched Adafruit_SSD1306 Library:
    Adafruit_SSD1306 display1(OLED_MISO, OLED_MOSI, OLED_CLK, OLED_DC, OLED1_RESET, OLED1_CS, 4000000UL);
    Adafruit_SSD1306* displaydraw;
#endif
#if (!defined usei2c && defined useoled2)
    //software spi Adafruit_SSD1306 display2(OLED_MOSI, OLED_CLK, OLED_DC, OLED1_RESET, OLED2_CS);
    //hardware spi
    Adafruit_SSD1306 display2(OLED_MISO, OLED_MOSI, OLED_CLK, OLED_DC, OLED2_RESET, OLED2_CS, 4000000UL);
#endif

#if (defined useoled1 || defined useoled2)
    boolean updatescreen = false;
    boolean updatescreen2 = false; //workaround for i2c/uart timing for 2 displays
    unsigned long olednextrefreshtimestamp = 0;
    unsigned long timeout_oled = 0;
    boolean oled_blink=true;
    boolean oledreinit = false;
    unsigned long oledreinittimestamp = 0;
    unsigned long popuptimestamp = 0;
    boolean showpopup = false;
    boolean showdialog = false;
    char diag_title[30];
    uint8_t *diag_value;
    uint8_t diag_min;
    uint8_t diag_max;

    char val1buf[dsvalbuflen];
    char val2buf[dsvalbuflen];
    char val3buf[dsvalbuflen];
    char val4buf[dsvalbuflen];
    char popuptitle[30];
    char popuptext[100];

  //screenhandling
    uint8_t screen = 0;
    uint8_t subscreen = 0;
    uint8_t windowsubpos=0;
    uint8_t throttlemin = throttlemindefault;
    uint8_t throttlemax = throttlemaxdefault;
    uint8_t stopwindowsize = (uint8_t)((throttlemax-throttlemin)/stopsubscreens);
    uint8_t chargewindowsize = (uint8_t)((throttlemax-throttlemin)/chargesubscreens);

    uint8_t configwindowsize = (uint8_t)((throttlemax-throttlemin)/(configsubscreens-1));

    uint8_t configstartindex = 0;
    uint8_t configendindex = 0;
    bool configchanged = false;
    uint8_t configcurrentitem = 0;

  //buttonhandling
    uint8_t brakebuttonstate = 0;  //1 = short, 2 = long press
    boolean brakebuttonpressed = false; //helper for key-detection
    #define buttonbrakepressed1 60 //treshold for "button pressed"
    #define buttonbrakeshortpressedduration 50 //millis needed for short press
    #define buttonbrakelongpressedduration 500 //millis needed for long press
    unsigned long buttonbrakepressedtimestamp = 0;

    uint8_t throttlebuttonstate = 0;  //1 = short, 2 = long press
    boolean throttlebuttonpressed = false; //helper for key-detection
    #define buttonthrottlepressed1 150 //treshold for "button pressed"
    #define buttonthrottleshortpressedduration 50 //millis needed for short press
    #define buttonthrottlelongpressedduration 500 //millis needed for long press
    unsigned long buttonthrottlepressedtimestamp = 0;

    uint8_t bothbuttonstate = 0;  //1 = short, 2 = long press
    boolean bothbuttonpressed = false; //helper for key-detection
    #define buttonbothshortpressedduration 50 //millis needed for short press
    #define buttonbothlongpressedduration 500 //millis needed for long press
    unsigned long buttonbothpressedtimestamp = 0;


  void drawscreen_startscreen() {
    display1.setFont();
    display1.setTextSize(1);
    display1.setTextColor(WHITE);
    display1.clearDisplay();
    display1.setCursor(35,5);
    #ifdef usengcode
      display1.print("ESP32 OLED NG");
    #else
      display1.print("ESP32 OLED");
    #endif
    //display1.setCursor(64,20);
    //display1.print("for");
    display1.setCursor(35,25);
    display1.print("Xiaomi Mijia365");
    display1.setCursor(80,55);
    display1.print(swversion);
    display1.drawBitmap(0,0,  scooter, 64,64, 1);
  }

  void drawscreen_header(const char *h, uint8_t scrnum, uint8_t scrtotal) {
    //blestruct* bleparsed = (blestruct*)bledata;
    displaydraw->setFont();
    displaydraw->setCursor(0,0);
    if (scrtotal!=0) {
      displaydraw->printf("%-15.15s (%d/%d)\r\n",FPSTR(h),scrnum,scrtotal);
    } else {
      displaydraw->printf("%-21.21s\r\n",FPSTR(h));
    }
  }

  void drawscreen_data(bool headline, uint8_t lines, bool showunits,
                 const char *l1, char *v1, const char *u1,
                 const char *l2, char* v2, const char *u2,
                 const char *l3, char* v3, const char *u3,
                 const char *l4, char* v4, const char *u4) {
    uint8_t i;
    uint8_t line;
    for (i=0;i<lines;i++) {
        switch(i) {
          case 0:
              if (headline & lines==4) { line = dataoffset+baselineoffset; }
              if (!headline & lines==4) { line = baselineoffset; }
              if (headline & lines<=3) { line = dataoffset+baselineoffset+5;}
              if (!headline & lines<=3) { line = baselineoffset; }
              displaydraw->setFont(sp_fontlabel); displaydraw->setCursor(sp_lx,line); displaydraw->print(FPSTR(l1));
              if (showunits) {
                displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dx,line); displaydraw->printf(v1);
                displaydraw->setCursor(sp_ux,line); displaydraw->setFont(sp_fontunit); displaydraw->print(FPSTR(u1));
              } else {
                displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dxnu,line); displaydraw->printf(v1);
              }
            break;
          case 1:
              if (headline & lines==4) { line = dataoffset+baselineoffset*2+linespace; }
              if (!headline & lines==4) { line = baselineoffset*2+linespace+2;}
              if (headline & lines<=3) { line = dataoffset+baselineoffset*2+10; }
              if (!headline & lines<=3) { line = baselineoffset*2+5+4; }
              displaydraw->setFont(sp_fontlabel); displaydraw->setCursor(sp_lx,line); displaydraw->print(FPSTR(l2));
              if (showunits) {
                displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dx,line); displaydraw->printf(v2);
                displaydraw->setCursor(sp_ux,line); displaydraw->setFont(sp_fontunit); displaydraw->print(FPSTR(u2));
              } else {
                displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dxnu,line); displaydraw->printf(v2);
              }
            break;
          case 2:
              if (headline & lines==4) { line = dataoffset+baselineoffset*3+linespace*2; }
              if (!headline & lines==4) { line = baselineoffset*3+linespace*2+5;}
              if (headline & lines<=3) { line = dataoffset+baselineoffset*3+15; }
              if (!headline & lines<=3) { line = baselineoffset*3+10+8; }
              displaydraw->setFont(sp_fontlabel); displaydraw->setCursor(sp_lx,line); displaydraw->print(FPSTR(l3));
              if (showunits) {
                displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dx,line); displaydraw->printf(v3);
                displaydraw->setCursor(sp_ux,line); displaydraw->setFont(sp_fontunit); displaydraw->print(FPSTR(u3));
              } else {
                displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dxnu,line); displaydraw->printf(v3);
              }
            break;
          case 3:
              if (headline & lines==4) { line = dataoffset+baselineoffset*4+linespace*3; }
              if (!headline & lines==4) { line = baselineoffset*4+linespace*3+8;}
              displaydraw->setFont(sp_fontlabel); displaydraw->setCursor(sp_lx,line); displaydraw->print(FPSTR(l4));
              if (showunits) {
                displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dx,line); displaydraw->printf(v4);
                displaydraw->setCursor(sp_ux,line); displaydraw->setFont(sp_fontunit); displaydraw->print(FPSTR(u4));
              } else {
                displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dxnu,line); displaydraw->printf(v4);
              }
            break;
        } //Switch i
    } //for i
  } //drawscreen


  void infopopup (char *_popuptitle, char *_popuptext, uint16_t duration) {
    if (!showdialog) {
      sprintf(popuptitle, "%s", _popuptitle);
      sprintf(popuptext, "%s", _popuptext);
      showpopup = true;
      popuptimestamp = millis()+duration;
    }
  }

  void alertpopup (char *_popuptitle, char *_popuptext, uint16_t duration) {
    if (!showdialog) {
      sprintf(popuptitle, "%s", _popuptitle);
      sprintf(popuptext, "%s", _popuptext);
      showpopup = true;
      popuptimestamp = millis()+duration;
      if (conf_beeponalert) {
        sendcommand = cmd_beep5;
      }
    }
  }

  void drawscreen_popup() {
    //singlescreen version:
    displaydraw->fillRect(4,4,119,55,BLACK);
    displaydraw->drawRect(6,6,115,51,WHITE); //cheap frame
    displaydraw->drawRect(7,7,113,49,WHITE); //cheap frame
    displaydraw->setFont(popup_fontheader);
    displaydraw->setCursor(popup_header_x, popup_header_y); 
    displaydraw->print(popuptitle);
    displaydraw->setFont(popup_fonttext);
    displaydraw->setCursor(popup_text_x, popup_text_y); 
    displaydraw->print(popuptext);
  }

  void dialog_edit_int8(const char *_dialogtitle, uint8_t *_value, uint8_t _minvalue, uint8_t _maxvalue) {
    sprintf(diag_title, "%s", _dialogtitle);
    diag_value = _value;
    diag_min = _minvalue;
    diag_max = _maxvalue;
    showdialog = true;
  }

  void drawscreen_dialog() {
    //singlescreen version:
    displaydraw->fillRect(4,4,119,55,BLACK);
    displaydraw->drawRect(6,6,115,51,WHITE); //cheap frame
    displaydraw->drawRect(7,7,113,49,WHITE); //cheap frame
    displaydraw->setFont(popup_fontheader);
    displaydraw->setCursor(popup_header_x, popup_header_y); 
    displaydraw->print(diag_title);
    displaydraw->setFont(popup_fonttext);
    displaydraw->setCursor(popup_text_x, popup_text_y); 
    displaydraw->printf("%d",*diag_value);
  }

  /* no used, implemented in oled1_draw
  void drawscreen_screenconfigsingle() {
    uint8_t line = baselineoffset;
      display1.setFont(menu_fontlabel); display1.setCursor(menu_x,line); display1.print("conf entry 1");
    line = baselineoffset*2+linespace+1;
      display1.setFont(menu_fontlabelselected); display1.setCursor(menu_x,line); display1.print("conf entry selecte");
    line = baselineoffset*3+linespace*2+3;
      display1.setFont(menu_fontlabel); display1.setCursor(menu_x,line); display1.print("conf entry 2");
    
    display1.drawRect(120,0,8,64,BLACK);
    display1.drawRect(0,baselineoffset*2+linespace-12,122,16,WHITE);
    //display1.drawFastVLine(125,0,line+2,WHITE);  
    display1.drawFastVLine(126,0,line+2,WHITE);
    display1.drawRect(125,15,3,10,WHITE);

    display1.drawFastHLine(0,baselineoffset*3+linespace*2+3+4,128,WHITE);
    line = baselineoffset*4+linespace*3+8;
    display1.setFont(menu_fontdata); display1.setCursor(menu_x,63); display1.print("current value");
  }
  */

  void handle_configmenuactions() {
      switch(subscreen) {
        case cms_light:
            switch(escparsed->taillight) {
              case 0: sendcommand = cmd_light_on; break;
              case 2: sendcommand = cmd_light_off; break;
            }
          break;
        case cms_cruise:
            switch(escparsed->cruisemode) {
              case 0: sendcommand = cmd_cruise_on; break;
              case 1: sendcommand = cmd_cruise_off; break;
            }
          break;
        case cms_kers:
            switch(escparsed->kers) {
              case 0: sendcommand = cmd_kers_medium; break;
              case 1: sendcommand = cmd_kers_strong; break;
              case 2: sendcommand = cmd_kers_weak; break;
            }
          break;
        case cms_ws: 
            if (conf_wheelsize==0) { 
                conf_wheelsize=1; 
              } else {
                conf_wheelsize=0; 
              }
              configchanged = true;
          break;
        case cms_unit: 
            if (conf_unit==0) { 
                conf_unit=1; 
              } else {
                conf_unit=0; 
              }
              configchanged = true;
          break;
        case cms_buv:
              dialog_edit_int8(FPSTR(s_setvolt),&conf_alert_batt_voltage,10,40);
              configchanged = true;
            break;
        case cms_bc: 
            if (conf_battcells==10) { 
              conf_battcells=12; 
            } else {
              conf_battcells=10;
            }
            configchanged = true;
          break;
        case cms_bac:
            switch (conf_alert_batt_celldiff) {
              case 1: conf_alert_batt_celldiff=3; break;
              case 3: conf_alert_batt_celldiff=5; break;
              case 5: conf_alert_batt_celldiff=10; break;
              case 10: conf_alert_batt_celldiff=20; break;
              case 20: conf_alert_batt_celldiff=50; break;
              case 50: conf_alert_batt_celldiff=1; break;
              default: conf_alert_batt_celldiff=5; break;
            }
            configchanged = true;
          break;
        case cms_bat:
            dialog_edit_int8(FPSTR(s_settemp),&conf_alert_batt_temp,10,100);
            configchanged = true;
          break;
        case cms_eat:
            dialog_edit_int8(FPSTR(s_settemp),&conf_alert_esc_temp,10,100);
            configchanged = true;
          break;
        /*case cms_flashprotection: 
            conf_flashprotect = !conf_flashprotect;
            configchanged = true;
          break;
        case cms_navigation: display1.print("Start Navigation Mode");
          break;*/
        case cms_beeponalert:
            conf_beeponalert = !conf_beeponalert;
            sendcommand = cmd_beep5; //just for testing purposes, remove later...
            configchanged = true;
          break;
        case cms_busmode: 
            conf_espbusmode = !conf_espbusmode;
            configchanged = true;
          break;
        case cms_wifirestart:
            wlanstate = wlanturnstaon;
            screen = screen_stop;
            subscreen = 0;
            if (configchanged) { saveconfig(); applyconfig(); }
          break;
        case cms_changelock:
            if (escparsed->lockstate==0x00) { 
              sendcommand = cmd_lock;
            } else {
              sendcommand = cmd_unlock;
              screen = screen_stop;
              subscreen = 0;
            }
            screen = screen_stop;
            subscreen = 0;
            if (configchanged) { saveconfig(); applyconfig(); }
          break;
        case cms_turnoff:
            sendcommand = cmd_turnoff;
            if (configchanged) { saveconfig(); applyconfig(); }
          break;
        case cms_exit:
            screen = screen_stop;
            subscreen = 0;
            if (configchanged) { saveconfig(); applyconfig(); }
          break;
      } //switch curline
  } //handle_configmenuactions


  void oled_switchscreens() {
    uint8_t oldscreen = screen;
    
    //1st. prio: Data/Bus Timeout
      if ((m365packettimestamp+m365packettimeout)<millis() & screen!=screen_timeout) {
        screen=screen_timeout;
        updatescreen=true;
        return;
      }
      if ((screen==screen_timeout) & ((m365packettimestamp+m365packettimeout)>millis())) {
        if (escparsed->speed>0) {
          screen=screen_drive;  
        } else {
          screen=screen_stop;  
        }
        updatescreen=true; 
      }

    //2nd prio - locked or alert in lockmode?
    if (escparsed->lockstate==0x02 & screen!=screen_configmenu) {
      screen=screen_locked;
    }

    if (escparsed->lockstate==0x09 & screen!=screen_configmenu) {
      screen=screen_alert;
    }
    
    //"button" handling
      if (newdata & (bleparsed->brake>=buttonbrakepressed1) & !brakebuttonpressed) {
        buttonbrakepressedtimestamp = millis();
        brakebuttonpressed=true;
        brakebuttonstate=0;
      }
      if (newdata & (bleparsed->brake<buttonbrakepressed1) & brakebuttonpressed) {
        if (millis()>buttonbrakepressedtimestamp+buttonbrakelongpressedduration) { 
          brakebuttonstate=2;
        } else {
          if (millis()>buttonbrakepressedtimestamp+buttonbrakeshortpressedduration) { 
            brakebuttonstate=1;
          }
        }
        if (brakebuttonstate!=0) { brakebuttonpressed=false; }
      }

      if (newdata & (bleparsed->throttle>=buttonthrottlepressed1) & !throttlebuttonpressed) {
        buttonthrottlepressedtimestamp = millis();
        throttlebuttonpressed=true;
        throttlebuttonstate=0;
      }
      if (newdata & (bleparsed->throttle<buttonthrottlepressed1) & throttlebuttonpressed) {
        if (millis()>buttonthrottlepressedtimestamp+buttonthrottlelongpressedduration) { 
          throttlebuttonstate=2;
        } else {
          if (millis()>buttonthrottlepressedtimestamp+buttonthrottleshortpressedduration) { 
            throttlebuttonstate=1;
          }
        }
        if (throttlebuttonstate!=0) { throttlebuttonpressed=false; }
      }

      if (newdata & (bleparsed->brake>=buttonbrakepressed1) & (bleparsed->throttle>=buttonthrottlepressed1) & !bothbuttonpressed & screen!=screen_configmenu) {
        buttonbothpressedtimestamp = millis();
        bothbuttonpressed=true;
        bothbuttonstate=0;
      }
      if (newdata & (bleparsed->brake<buttonbrakepressed1) & (bleparsed->throttle<buttonthrottlepressed1) & bothbuttonpressed) {
        if (millis()>buttonbothpressedtimestamp+buttonbothlongpressedduration) { 
          bothbuttonstate=2;
        } else {
          if (millis()>buttonbothpressedtimestamp+buttonbothshortpressedduration) { 
            bothbuttonstate=1;
          }
        }
        if (bothbuttonstate!=0) { bothbuttonpressed=false; }
      }
    
    //execute commands from configscreen
      if (screen==screen_configmenu) {
        if (showdialog) {
          //1st prio: handle value-change dialogs
          updatescreen = true;
          *diag_value = map(bleparsed->throttle,throttlemin,throttlemax,diag_min,diag_max);
          if (brakebuttonstate!=0) { //brake pressed, exit dialog?
            showdialog = false;
          }
        } else {
          //2nd prio: handle brake-button actions
          //if (screen==screen_configmenu & brakebuttonstate!=0 & !showdialog) {
          if (brakebuttonstate!=0 & !showpopup) {  
            handle_configmenuactions();
            updatescreen = true;
          }
        }
      }

    //enter configmenu
      if (bothbuttonstate!=0 & escparsed->speed==0 & screen!=screen_configmenu) {
        screen = screen_configmenu;
        subscreen = 0;
        configchanged = false;
        updatescreen = true;
        infopopup((char*)"  MENU", (char*)"release throttle!", 1000);
      }
    /*
    //exit from configmenu via long-brake-press --> we use "Exit" option in menu
      if (brakebuttonstate==2 & screen==screen_configmenu) {
      //if (brakebuttonstate==1 & screen==screen_stop & subscreen == stopsubscreens) {
        if (configchanged) { saveconfig(); }
        screen = screen_stop;
        subscreen = 0;
        updatescreen = true;
      }
      */
      //exit from configmenu if speed > 5km/h
      if (screen==screen_configmenu & (abs((float)escparsed->speed/1000.0f)>5.0f)) {
      //if (brakebuttonstate==1 & screen==screen_stop & subscreen == stopsubscreens) {
        if (configchanged) { saveconfig(); }
        screen = screen_drive;
        subscreen = 0;
        updatescreen = true;
      }

    //configmenu navigaton via gas:
    if (newdata & (screen==screen_configmenu) & !(showdialog|showpopup)) {
      uint8_t oldsubscreen = subscreen;
      if (bleparsed->throttle>throttlemin+5) {
        subscreen = ((bleparsed->throttle-throttlemin) / configwindowsize)+1;
        subscreen=_min(subscreen,configsubscreens-1);
        windowsubpos = (uint8_t)((float)((bleparsed->throttle-throttlemin) % configwindowsize)*(float)oledwidth/(float)configwindowsize);
      } else {
        subscreen=0;
        windowsubpos=0;
      }
      if ((subscreen)>configsubscreens) { subscreen=configsubscreens; }
      if (subscreen!=oldsubscreen) { 
        configstartindex = _max(subscreen - configlinesabove,0);
        configendindex = _min(configstartindex + confignumlines-1,configsubscreens-1);
        if ((configendindex-confignumlines+1)<configstartindex) {
          configstartindex = configendindex - confignumlines+1;
        }
        updatescreen = true; 
      }
    }
    //charging screens: 
      if (newdata & (escparsed->speed==0) & (bmsparsed->current<0) & (screen!=screen_charging)) { 
        if  (screen!=screen_locked) {
          //only display if not locked...
          screen=screen_charging; 
        }
        //timeout_oled=millis()+oledchargestarttimeout;
        capacitychargestart = bmsparsed->remainingcapacity;
        updatescreen=true;
      }
      if (newdata & (screen==screen_charging) & (abs((float)escparsed->speed/1000.0f)>2.0f)) { 
        if (abs((float)escparsed->speed/1000.0f)>0.9f) {
          screen=screen_drive;  
        } else {
          screen=screen_stop;  
        }
        updatescreen=true;
      }

    //switch between driving/stop screens:
      //maybe add a speed treshold like 0.3km/h to keep showing "stop" screen
      //if (newdata & (screen==screen_drive) & (escparsed->speed==0)) {
      if (newdata & (screen==screen_drive) & (abs((float)escparsed->speed/1000.0f)<0.5f)) {
      //if (newdata & ((x1parsed->mode==0)|(x1parsed->mode==2))) {
        screen=screen_stop;
        updatescreen=true;
      }
      //if (newdata & (screen==screen_stop) & (escparsed->speed>0)) {
      if (newdata & (screen==screen_stop) & (abs((float)escparsed->speed/1000.0f)>0.9f)) {
        
      //if (newdata & ((x1parsed->mode==1)|(x1parsed->mode==3))) {
        screen=screen_drive;
        updatescreen=true;
      }

    //switching of subscreens via throttle while we are in STOP mode
    if (newdata & (screen==screen_stop)) {
      uint8_t oldsubscreen = subscreen;
      if (bleparsed->throttle>throttlemin+5) {
        subscreen = ((bleparsed->throttle-throttlemin) / stopwindowsize)+1;
        windowsubpos = (uint8_t)((float)((bleparsed->throttle-throttlemin) % stopwindowsize)*(float)oledwidth/(float)stopwindowsize);
      } else {
        subscreen=0;
        windowsubpos=0;
      }
      if ((subscreen)>stopsubscreens) { subscreen=stopsubscreens; }
      if (subscreen!=oldsubscreen) { updatescreen = true; }
    }
    
    #if !defined useoled2
    //switching of subscreens via throttle while we are in CHARGE mode, only needed with one screen
    if (newdata & (screen==screen_charging)) {
      uint8_t oldsubscreen = subscreen;
      if (bleparsed->throttle>=throttlemin) {
        subscreen = ((bleparsed->throttle-throttlemin) / chargewindowsize)+1;
        windowsubpos = (uint8_t)((float)((bleparsed->throttle-throttlemin) % chargewindowsize)*(float)oledwidth/(float)chargewindowsize);
      } else {
        subscreen=1;
        windowsubpos=0;
      }
      if ((subscreen)>chargesubscreens) { subscreen=chargesubscreens; }
      if (subscreen!=oldsubscreen) { updatescreen = true; }
    }
    #endif

    //update with new data, but honor refresh-rate
      if (newdata & (olednextrefreshtimestamp<millis())) { 
      //if ((olednextrefreshtimestamp<millis())) { 
        updatescreen=true;
        newdata=false; 
      }

    //update subscriptions if screen has been changed
      if (oldscreen!=screen) { 
        //DebugSerial.printf("---screen: %d\r\n",screen);
        subscribedrequests=rqsarray[screen];
      }
    //reset newdata flag if we consumed it
      if (updatescreen) { newdata=false; }
    
    //DebugSerial.printf("### Br: %03d Thr: %03d B: %01d T: %01d BT: %01d Scr: %01d\r\n",escparsed->brake, escparsed->throttle, brakebuttonstate, throttlebuttonstate, bothbuttonstate, screen);
    //reset buttonstate
      brakebuttonstate=0;
      throttlebuttonstate=0;
      bothbuttonstate=0;

    //popup timeout handling
    if ((showpopup) & (millis() > popuptimestamp)) {
      showpopup = false;
    }
  } //oled_switchscreens

  void cm_printKey(uint8_t entryid) {
            switch(entryid) {
              case cms_light: display1.print(FPSTR(menu_light));
                break;
              case cms_cruise: 
                  display1.print(FPSTR(menu_cruise));
                break;
              case cms_kers: 
                  display1.print(FPSTR(menu_kers));
                break;
              case cms_ws: display1.print(FPSTR(menu_wheelsize));
                break;
              case cms_unit: display1.print(FPSTR(menu_unit));
                break;
              case cms_buv: display1.print(FPSTR(menu_battalertlowvoltage));
                break;
              case cms_bc: display1.print(FPSTR(menu_battcells));
                break;
              case cms_bac: display1.print(FPSTR(menu_battalertcell));
                break;
              case cms_bat: display1.print(FPSTR(menu_battalerttemp));
                break;
              case cms_eat: display1.print(FPSTR(menu_escalerttemp));
                break;
              /*case cms_flashprotection: display1.print("***shprot");
                break;
              case cms_navigation: display1.print("***igation");
                break;*/
              case cms_beeponalert: display1.print(FPSTR(menu_beeponalert));
                break;
              case cms_busmode: display1.print(FPSTR(menu_espbusmode));
                break;
              case cms_wifirestart: display1.print(FPSTR(menu_espwifirestart));
                break;
              case cms_changelock: 
                    if (escparsed->lockstate==0x00) {
                      display1.print(FPSTR(menu_m365lock));
                    } else {
                      display1.print(FPSTR(menu_m365unlock));
                    }
                break;
              case cms_turnoff: display1.print(FPSTR(menu_m365turnoff));
                break;
              case cms_exit: display1.print(FPSTR(menu_exit));
                break;
            } //switch curline
  }

  void cm_printValue(uint8_t entryid) {
    #if!defined useoled2
      displaydraw->setCursor(5,dataoffset+baselineoffset + 3*(baselineoffset+linespace));
      displaydraw->drawFastHLine(0,dataoffset+baselineoffset + 2*(baselineoffset+linespace)-linespace-linespace,128,WHITE);
    #endif
    #ifdef useoled2
      displaydraw->setCursor(5,31-baselineoffset/2);
    #endif
            switch(entryid) {
              case cms_light:
                    switch(escparsed->taillight) {
                      case 0: displaydraw->print(FPSTR(menu_off)); break;
                      case 2: displaydraw->print(FPSTR(menu_on)); break;
                    }
                break;
              case cms_cruise: 
                    switch(escparsed->cruisemode) {
                      case 0: displaydraw->print(FPSTR(menu_off)); break;
                      case 1: displaydraw->print(FPSTR(menu_on)); break;
                    }
                break;
              case cms_kers: 
                    switch(escparsed->kers) {
                      case 0: displaydraw->print(FPSTR(menu_weak)); break;
                      case 1: displaydraw->print(FPSTR(menu_medium)); break;
                      case 2: displaydraw->print(FPSTR(menu_strong)); break;
                    }
                break;
              case cms_ws: 
                  if (conf_wheelsize==0) { displaydraw->print("8.5\""); }
                  if (conf_wheelsize==1) { displaydraw->print("10\""); }
                  if (conf_wheelsize>1) { displaydraw->print("???"); }
                break;
              case cms_unit: 
                  if (conf_unit==0) { displaydraw->print(FPSTR(menu_km)); }
                  if (conf_unit==1) { displaydraw->print(FPSTR(menu_miles)); }
                break;
              case cms_bc:
                  displaydraw->printf("%d", conf_battcells);
                break;
              case cms_buv: 
                  displaydraw->printf("%d V", conf_alert_batt_voltage);
                break;
              case cms_bac:
                  displaydraw->printf("%d0 mV", conf_alert_batt_celldiff);
                break;
              case cms_bat:
                    displaydraw->printf("%d C", conf_alert_batt_temp);
                break;
              case cms_eat:
                      displaydraw->printf("%d C", conf_alert_esc_temp);
                break;
              //case cms_flashprotection: displaydraw->print("no value");
              //case cms_navigation: displaydraw->print("no value");
              case cms_beeponalert:
                  if (conf_beeponalert) { 
                    displaydraw->print(FPSTR(menu_on));
                  } else { 
                    displaydraw->print(FPSTR(menu_off)); 
                  }
                break;
              case cms_busmode:
                  if (conf_espbusmode) { 
                    displaydraw->print(FPSTR(menu_active));
                  } else { 
                    displaydraw->print(FPSTR(menu_passive)); 
                  }
                break;
              //case cms_wifirestart: displaydraw->print("no value");
              //case cms_changelock:  displaydraw->print("no value");
              //case cms_turnoff:  displaydraw->print("no value");
              //case cms_exit:  displaydraw->print("no value");
            } //switch curline
  }

  
  void handle_oled() {
    timestamp_oledstart=micros();
    oled_switchscreens();
    //TEST TWEAK
    //screen=screen_charging;
    //screen=screen_timeout;
    //screen=screen_error; escparsed->error=12;
    //screen=screen_stop; subscreen=4;
    if (millis() > oledreinittimestamp) {
        oledreinit = true;
        oledreinittimestamp = millis() + oledreinitduration;
    }

    #ifdef useoled2 //workaround for slow i2c & 2 displays to avoid too long blocking of main code
      if (updatescreen2) {
        oled2_update();
        updatescreen2=false;
        //DebugSerial.printf("---OLED2----- %02d %d\r\n",screen, millis());
      } else {
    #endif
        if (updatescreen) {
          olednextrefreshtimestamp=millis()+oledrefreshanyscreen;
          oled1_update();
          //oled2_update();
          updatescreen=false;
          //DebugSerial.printf("---OLED1----- %02d %d\r\n",screen, millis());
        }
    #ifdef useoled2
      }
    #endif
    duration_oled = micros()-timestamp_oledstart;
    oled_blink!=oled_blink;
  }



#endif //useoled1 or useoled2

#ifdef useoled1
  void oled1_init() {
    #ifdef usei2c
      display1.begin(SSD1306_SWITCHCAPVCC, oled1_address, oled_doreset, oled_sda, oled_scl, 800000UL);
    #else
      display1.begin(SSD1306_SWITCHCAPVCC);
    #endif
    display1.setRotation(OLED1_ROTATION);
    display1.setTextColor(WHITE);
    display1.setFont();
    display1.setTextSize(1);
    display1.clearDisplay();
  }

  void oled1_update() {
    uint8_t line;
    uint8_t lineitem;
    timestamp_oled1draw=micros();

    if (oledreinit) {
      oled1_init();
      #if !defined useoled2
        oledreinit=false;
      #endif
    }
    display1.clearDisplay();
    displaydraw = &display1;
    
    if (screen==screen_drive) {
        display1.setFont(&ariblk42pt7b);
        display1.setCursor(0,62);
        display1.printf("%02d", uint8_t(abs((float)escparsed->speed/1000.0f*wheelfact)));
        display1.setFont(&ARIALNB18pt7b);
        display1.setCursor(108,28);
        display1.printf("%01d", uint16_t((float)escparsed->speed/100.0f*wheelfact) %10); 
        display1.setFont(&ARIALNB9pt7b);
        if (x1parsed->light==0x64) { 
          display1.setCursor(118,45);
          display1.print("L");
        }
        display1.setCursor(118,63);
        if (x1parsed->mode<2) { 
          //display1.print("N"); //normal mode
        } else {
          display1.print("E"); //eco mode
        }
    }
    if (screen==screen_stop) {
          display1.setFont();
          display1.setCursor(0,0);
          switch (subscreen) {
            case stopsubscreen_trip: //Trip Info: Average Speed, Distance, Time, Average Speed 
                drawscreen_header(FPSTR(headline_tripinfo),0,0);
                sprintf(val1buf,"%05.2f",(float)escparsed->averagespeed/1000.0f*wheelfact);
                sprintf(val2buf,"%05.2f",(float)escparsed->tripdistance/100.0f*wheelfact);
                sprintf(val3buf,"%02d:%02d",escparsed->triptime/60,escparsed->triptime % 60);
                sprintf(val4buf,"%05.2f",(float)escparsed->remainingdistance/100.0f);
                  drawscreen_data(true, 4, true,
                      FPSTR(label_averageshort),&val1buf[0],(conf_unit==0?FPSTR(unit_speed_km):FPSTR(unit_speed_miles)),
                      FPSTR(label_distanceshort),&val2buf[0],(conf_unit==0?FPSTR(unit_distance_km):FPSTR(unit_distance_miles)),
                      FPSTR(label_time),&val3buf[0],FPSTR(label_seconds),
                      FPSTR(label_remainingshort),&val4buf[0],(conf_unit==0?FPSTR(unit_distance_km):FPSTR(unit_distance_miles)));
              break;
              #if !defined useoled2
                case stopsubscreen_temp: //Single
                    drawscreen_header(FPSTR(headline_temperature),subscreen,stopsubscreens);
                    sprintf(val1buf,"%4.1f",(float)bmsparsed->temperature[0]-20.0f);
                    sprintf(val2buf,"%4.1f",(float)bmsparsed->temperature[1]-20.0f);
                    sprintf(val3buf,"%4.1f",(float)escparsed->frametemp2/10.0f);
                    drawscreen_data(true, 3, true,
                      FPSTR(label_batt1),&val1buf[0],FPSTR(unit_temp),
                      FPSTR(label_batt2),&val2buf[0],FPSTR(unit_temp),
                      FPSTR(label_esc),&val3buf[0],FPSTR(unit_temp),
                      NULL,NULL,NULL);
                  break;
              #endif
              case stopsubscreen_minmax: //Single/Dual/Same
              //fix this screen - single and dual version...
                display1.printf("TRIP Min/Max    (%d/%d)\r\n\r\n",subscreen,stopsubscreens);
                display1.printf("Speed : %5.1f %5.1f\r\n",(float)speed_min/1000.0f,(float)speed_max/1000.0f);
                display1.printf("Ampere: %5.1f %5.1f A\r\n",(float)current_min/100.0f,(float)current_max/100.0f);
                display1.printf("Watt  : %5d %5d W\r\n",watt_min,watt_max);
                display1.printf("BattT1: %3d.0 %3d.0 C\r\n",tb1_min-20,tb1_max-20); display1.drawCircle(117,42,1,WHITE);
                display1.printf("BattT2: %3d.0 %3d.0 C\r\n",tb2_min-20,tb2_max-20); display1.drawCircle(117,50,1,WHITE);
                display1.printf("ESC T : %5.1f %5.1f C\r\n",(float)te_min/10.0f,(float)te_max/10.0f); display1.drawCircle(117,58,1,WHITE);
              break;
              case stopsubscreen_batt1: //Single/Dual/Same
                #if !defined useoled2
                  drawscreen_header(FPSTR(headline_battery1),subscreen,stopsubscreens);
                #else
                  drawscreen_header(FPSTR(headline_batterystatus),subscreen,stopsubscreens);
                #endif
                sprintf(val1buf,"%5.2f",(float)bmsparsed->voltage/100.0f);
                sprintf(val2buf,"%4d%",bmsparsed->remainingpercent);
                sprintf(val3buf,"%4d",bmsparsed->cycles);
                sprintf(val4buf,"%4d",bmsparsed->chargingtimes);
                drawscreen_data(true, 4, true,
                    FPSTR(label_volt),&val1buf[0],FPSTR(unit_volt),
                    FPSTR(label_percent),&val2buf[0],FPSTR(unit_percent),
                    FPSTR(label_cycles),&val3buf[0],FPSTR(unit_count),
                    FPSTR(label_charges),&val4buf[0],FPSTR(unit_count));
              break;
              #if !defined useoled2  
                case stopsubscreen_batt2: //Single Screen Batt Info 2: Health/Cycles/Charge Num/Prod Date
                    drawscreen_header(FPSTR(headline_battery2),subscreen,stopsubscreens);
                    sprintf(val1buf,"%4d",bmsparsed->health);
                    sprintf(val2buf,"%5d",bmsparsed->remainingcapacity);
                    sprintf(val3buf,"%5d",bmsparsed->totalcapacity);
                    sprintf(val4buf,"%02d/%02d",bmsparsed->temperature[0]-20,bmsparsed->temperature[1]-20);
                    drawscreen_data(true, 4, true,
                        FPSTR(label_health),&val1buf[0],FPSTR(unit_percent),
                        FPSTR(label_capacityshort),&val2buf[0],FPSTR(unit_mah),
                        FPSTR(label_totalcapacityshort),&val3buf[0],FPSTR(unit_mah),
                        FPSTR(label_tempshort),&val4buf[0],FPSTR(unit_temp));
              #endif
              case stopsubscreen_cells: //single/dual/different
                drawscreen_header(FPSTR(headline_cellvolt),subscreen,stopsubscreens);
                #if !defined useoled2
                  if (conf_battcells>=1) { display1.printf("01: %5.03f ",(float)bmsparsed->Cell1Voltage/1000.0f); }
                  if (conf_battcells>=2) { display1.printf("02: %5.03f  ",(float)bmsparsed->Cell2Voltage/1000.0f); }
                  if (conf_battcells>=3) { display1.printf("03: %5.03f ",(float)bmsparsed->Cell3Voltage/1000.0f); }
                  if (conf_battcells>=4) { display1.printf("04: %5.03f  ",(float)bmsparsed->Cell4Voltage/1000.0f); }
                  if (conf_battcells>=5) { display1.printf("05: %5.03f ",(float)bmsparsed->Cell5Voltage/1000.0f); }
                  if (conf_battcells>=6) { display1.printf("06: %5.03f  ",(float)bmsparsed->Cell6Voltage/1000.0f); }
                  if (conf_battcells>=7) { display1.printf("07: %5.03f ",(float)bmsparsed->Cell7Voltage/1000.0f); }
                  if (conf_battcells>=8) { display1.printf("08: %5.03f  ",(float)bmsparsed->Cell8Voltage/1000.0f); }
                  if (conf_battcells>=9) { display1.printf("09: %5.03f  ",(float)bmsparsed->Cell9Voltage/1000.0f); }
                  if (conf_battcells>=10) { display1.printf("10: %5.03f ",(float)bmsparsed->Cell10Voltage/1000.0f); }
                  if (conf_battcells>=11) { display1.printf("11: %5.03f ",(float)bmsparsed->Cell11Voltage/1000.0f); }
                  if (conf_battcells>=12) { display1.printf("12: %5.03f  ",(float)bmsparsed->Cell12Voltage/1000.0f); }
                  display1.printf("%s %5.03f", FPSTR(label_maxdiff), (float)(highest-lowest)/1000.0f);
                #else
                  display1.setFont(&ARIALN9pt7b); 
                  display1.setCursor(0,21);
                  if (conf_battcells>=1) { display1.printf(" 1: %5.03f ",(float)bmsparsed->Cell1Voltage/1000.0f); }
                  if (conf_battcells>=2) { display1.printf(" 2: %5.03f",(float)bmsparsed->Cell2Voltage/1000.0f); }
                  display1.setCursor(0,35);
                  if (conf_battcells>=3) { display1.printf(" 3: %5.03f ",(float)bmsparsed->Cell3Voltage/1000.0f); }
                  if (conf_battcells>=4) { display1.printf(" 4: %5.03f",(float)bmsparsed->Cell4Voltage/1000.0f); }
                  display1.setCursor(0,49);
                  if (conf_battcells>=5) { display1.printf(" 5: %5.03f ",(float)bmsparsed->Cell5Voltage/1000.0f); }
                  if (conf_battcells>=6) { display1.printf(" 6: %5.03f",(float)bmsparsed->Cell6Voltage/1000.0f); }
                  display1.setCursor(0,63);
                  if (conf_battcells>=7) { display1.printf(" 7: %5.03f ",(float)bmsparsed->Cell7Voltage/1000.0f); }
                  if (conf_battcells>=8) { display1.printf(" 8: %5.03f",(float)bmsparsed->Cell8Voltage/1000.0f); }
                #endif  
              break;
              case stopsubscreen_assets: //Single/Dual/Same
                drawscreen_header(FPSTR(headline_assests),subscreen,stopsubscreens);
                sprintf(tmp1,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c",bmsparsed->serial[0],bmsparsed->serial[1],bmsparsed->serial[2],bmsparsed->serial[3],bmsparsed->serial[4],bmsparsed->serial[5],bmsparsed->serial[6],bmsparsed->serial[7],bmsparsed->serial[8],bmsparsed->serial[9],bmsparsed->serial[10],bmsparsed->serial[11],bmsparsed->serial[12],bmsparsed->serial[13]);
                display1.printf(FPSTR(s_bmsfw), bmsparsed->fwversion[1],(bmsparsed->fwversion[0]&0xf0)>>4,bmsparsed->fwversion[0]&0x0f,tmp1);
                sprintf(tmp1,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c",escparsed->serial[0],escparsed->serial[1],escparsed->serial[2],escparsed->serial[3],escparsed->serial[4],escparsed->serial[5],escparsed->serial[6],escparsed->serial[7],escparsed->serial[8],escparsed->serial[9],escparsed->serial[10],escparsed->serial[11],escparsed->serial[12],escparsed->serial[13]);
                display1.printf(FPSTR(s_escfw), escparsed->fwversion[1],(escparsed->fwversion[0]&0xf0)>>4,escparsed->fwversion[0]&0x0f,tmp1);
                sprintf(tmp1,"%c%c%c%c%c%c",escparsed->pin[0],escparsed->pin[1],escparsed->pin[2],escparsed->pin[3],escparsed->pin[4],escparsed->pin[5]);
                display1.printf(FPSTR(s_pin), tmp1);
                display1.printf(FPSTR(s_miles),(float)escparsed->totaldistance/1000.0f);
                display1.printf(FPSTR(s_battdate),((bmsparsed->proddate)&0xFE00)>>9,((bmsparsed->proddate)&0x1E0)>>5,(bmsparsed->proddate)&0x1f);
              break;
              #if !defined useoled2  
                case stopsubscreen_espstate: //single
                    drawscreen_header(FPSTR(headline_espstate),subscreen,stopsubscreens);
                    display1.print(FPSTR(s_firmware)); display1.println(swversion);
                    display1.print(FPSTR(s_wlan));
                    if (wlanstate==wlansearching) { display1.println(FPSTR(s_wlansearch)); }
                    if (wlanstate==wlanconnected) { display1.print(FPSTR(s_wlancon)); display1.println(WiFi.SSID()); display1.println(WiFi.localIP()); }
                    if (wlanstate==wlanap) { display1.print(FPSTR(s_wlanap)); display1.println(WiFi.softAPIP()); display1.println( WiFi.softAPIP());}
                    if (wlanstate==wlanoff) { display1.println(FPSTR(s_off)); }
                    display1.print(FPSTR(s_bleoff));
                  break;
              #endif
              case stopsubscreen_alarms: //single/dual/same
                drawscreen_header(FPSTR(headline_alerts),subscreen,stopsubscreens);
                sprintf(val1buf,"%03d",alertcounter_bmstemp);
                sprintf(val2buf,"%03d",alertcounter_esctemp);
                sprintf(val3buf,"%03d",alertcounter_cellvoltage);
                sprintf(val4buf,"%03d",alertcounter_undervoltage);
                drawscreen_data(true, 4, false,
                    FPSTR(label_bmstemp),&val1buf[0],FPSTR(unit_percent),
                    FPSTR(label_esctemp),&val2buf[0],FPSTR(unit_mah),
                    FPSTR(label_cellvoltage),&val3buf[0],FPSTR(unit_mah),
                    FPSTR(label_undervoltage),&val4buf[0],FPSTR(unit_temp));
              break;
          }
    }
    if (screen == screen_configmenu) {
      
      lineitem=0;
      for (uint8_t curline=configstartindex;curline<=configendindex;curline++) {
        if (curline==subscreen) {
          display1.setFont(sp_fontdata);
          #if !defined useoled2
            cm_printValue(curline);
          #endif
          #ifdef useoled2
            configcurrentitem=curline;
          #endif
        } else {
          display1.setFont(sp_fontlabel);
        }
        //#if !defined useoled2
          display1.setCursor(5,baselineoffset + lineitem*(baselineoffset+linespace));
        //#else
          display1.setCursor(5,baselineoffset + lineitem*(baselineoffset+linespace));
        //#endif
        cm_printKey(curline);
        lineitem++;
      } //curline loop
    } //screen == screen_configmenu
    if (screen==screen_charging) {
          display1.clearDisplay();
          display1.setFont();
          display1.setCursor(0,0);
          display1.drawFastHLine(0,8,windowsubpos,WHITE);
          #if !defined useoled2
          switch (subscreen) {
            case 1: //Trip Info: Average Speed, Distance, Time, Average Speed 
                drawscreen_header(FPSTR(s_blank),subscreen,2);
          #endif
            display1.setTextSize(1);
            display1.setTextColor(WHITE);
            display1.setFont(&ARIALN9pt7b); 
            display1.setCursor(2,baselineoffset);
            display1.print(FPSTR(s_charging));
            display1.printf(" %2d%%", bmsparsed->remainingpercent);
            display1.drawRect(14,23,100,10,WHITE);
            display1.fillRect(14,24,bmsparsed->remainingpercent,8,WHITE);
            display1.setFont();
            display1.setCursor(14,39);
            display1.printf("%4.1f V   %4.1f A\r\n", (float)bmsparsed->voltage/100.0f,abs((float)bmsparsed->current/100.0f));
            display1.setCursor(14,48);
            display1.printf("%4.0f W  %5d mAh",abs(((float)(bmsparsed->voltage/100.0f)*(float)bmsparsed->current/100.0f)),bmsparsed->remainingcapacity);
            display1.setCursor(14,57);
            display1.print(FPSTR(s_charged));
            display1.printf(" %5d mAh",bmsparsed->remainingcapacity-capacitychargestart);
          #if !defined useoled2
              break;
            case 2: //Cell Infos
                drawscreen_header(FPSTR(headline_charging),subscreen,2);
                if (conf_battcells>=1) { display1.printf("01: %5.03f ",(float)bmsparsed->Cell1Voltage/1000.0f); }
                if (conf_battcells>=2) { display1.printf("02: %5.03f  ",(float)bmsparsed->Cell2Voltage/1000.0f); }
                if (conf_battcells>=3) { display1.printf("03: %5.03f ",(float)bmsparsed->Cell3Voltage/1000.0f); }
                if (conf_battcells>=4) { display1.printf("04: %5.03f  ",(float)bmsparsed->Cell4Voltage/1000.0f); }
                if (conf_battcells>=5) { display1.printf("05: %5.03f ",(float)bmsparsed->Cell5Voltage/1000.0f); }
                if (conf_battcells>=6) { display1.printf("06: %5.03f  ",(float)bmsparsed->Cell6Voltage/1000.0f); }
                if (conf_battcells>=7) { display1.printf("07: %5.03f ",(float)bmsparsed->Cell7Voltage/1000.0f); }
                if (conf_battcells>=8) { display1.printf("08: %5.03f  ",(float)bmsparsed->Cell8Voltage/1000.0f); }
                if (conf_battcells>=9) { display1.printf("09: %5.03f ",(float)bmsparsed->Cell9Voltage/1000.0f); }
                if (conf_battcells>=10) { display1.printf("10: %5.03f  ",(float)bmsparsed->Cell10Voltage/1000.0f); }
                if (conf_battcells>=11) { display1.printf("11: %5.03f ",(float)bmsparsed->Cell11Voltage/1000.0f); }
                if (conf_battcells>=12) { display1.printf("12: %5.03f  ",(float)bmsparsed->Cell12Voltage/1000.0f); }
                if (conf_battcells<11) { display1.printf("L:  %5.03f H:  %5.03f\r\n", (float)(lowest)/1000.0f,(float)(highest)/1000.0f); }
                display1.printf("T:  %5.02f D:  %5.03f", (float)bmsparsed->voltage/100.0f,(float)(highest-lowest)/1000.0f);
              break;
          }
          #endif
    }
    //if (screen==screen_timeout) {
    if (screen==screen_error) {
        #if !defined(useoled2)
          display1.setFont(&ARIALN9pt7b);
          display1.setCursor(30,20);
          display1.print(FPSTR(error_error));
          display1.setCursor(50,40);
          display1.printf("%02d",escparsed->error);
          display1.setFont();
          display1.setCursor(0,56);
          switch (escparsed->error) {
            case 10: display1.print(FPSTR(error_10)); break;
            case 11: display1.print(FPSTR(error_11)); break;
            case 12: display1.print(FPSTR(error_12)); break;
            case 13: display1.print(FPSTR(error_13)); break;
            case 14: display1.print(FPSTR(error_14)); break;
            case 15: display1.print(FPSTR(error_15)); break;
            case 18: display1.print(FPSTR(error_18)); break;
            case 21: display1.print(FPSTR(error_21)); break;
            case 22: display1.print(FPSTR(error_22)); break;
            case 23: display1.print(FPSTR(error_23)); break;
            case 24: display1.print(FPSTR(error_24)); break;
            case 26: display1.print(FPSTR(error_26)); break;
            case 27: display1.print(FPSTR(error_27)); break;
            case 28: display1.print(FPSTR(error_28)); break;
            case 29: display1.print(FPSTR(error_29)); break;
            case 31: display1.print(FPSTR(error_31)); break;
            case 35: display1.print(FPSTR(error_35)); break;
            case 36: display1.print(FPSTR(error_36)); break;
            case 39: display1.print(FPSTR(error_39)); break;
            case 40: display1.print(FPSTR(error_40)); break;
            default: display1.print(FPSTR(error_other)); break;
          }
        #else
          display1.setFont(&ARIALN9pt7b);
          display1.setCursor(50,20);
          display1.print(FPSTR(error_error));
          display1.setCursor(70,45);
          display1.printf("%02d",escparsed->error);
          display1.drawBitmap(0,0,  scooter, 64,64, 1);
        #endif
    }
    if (screen==screen_timeout) {
      #ifdef useoled2
        drawscreen_startscreen();
      #else
        display1.drawBitmap(0,0,  scooter, 64,64, 1);
        display1.setFont(&ARIALN9pt7b);
        display1.setCursor(74,15);
        display1.print(FPSTR(s_timeout1));
        display1.setCursor(64,35);
        display1.print(FPSTR(s_timeout2));
        display1.setFont();
        if (wlanstate==wlansearching) { display1.setCursor(64,42); display1.print(FPSTR(s_wlan)); display1.setCursor(64,55); display1.print(FPSTR(s_wlansearch)); }
        if (wlanstate==wlanconnected) { display1.setCursor(64,42); display1.print(WiFi.SSID()); display1.setCursor(34,57); display1.print(WiFi.localIP()); }
        if (wlanstate==wlanap) { display1.setCursor(64,42); display1.print(WiFi.softAPIP()); display1.setCursor(34,57); display1.print( WiFi.softAPIP()); }
        if (wlanstate==wlanoff) { display1.setCursor(64,56); display1.print(FPSTR(s_wlanoff)); }
      #endif
    }
    if (screen==screen_locked) {
      //display1.drawBitmap(0,0,  scooter, 64,64, 1);  
      display1.setFont();
      display1.setCursor(39,31);
      display1.print(FPSTR(s_locked));
    }
    if (screen==screen_alert) {  
      display1.setFont();
      display1.setCursor(39,31);
      display1.print(FPSTR(s_alert));
    }

    if ((screen==screen_stop)&(subscreen==0)) {
      //LIGHT ON/OFF
        if (x1parsed->light==0x64) { 
          display1.setFont();
          display1.setCursor(120,line1);
          display1.print("L");
        }
      //NORMAL/ECO MODE
        display1.setFont();
        display1.setCursor(112,line1);
        if (x1parsed->mode<2) { 
          display1.print("N"); //normal mode
        } else {
          display1.print("E"); //eco mode
        }
      //WLAN STATUS
        display1.setFont();
        display1.setCursor(96,line1);
        if (wlanstate==wlansearching) { display1.print(FPSTR(s_wlansearchshort)); }
        if (wlanstate==wlanconnected) { display1.print(FPSTR(s_wlanconshort)); }
        if (wlanstate==wlanap) { display1.print(FPSTR(s_wlanapshort)); }
    }

    if (showpopup) {
      drawscreen_popup();
    }

    #if !defined useoled2
      if (showdialog) {
        drawscreen_dialog();
      }
    #endif
    duration_oled1draw = micros()-timestamp_oled1draw;
    timestamp_oled1transfer=micros();
    display1.display();
    duration_oled1transfer = micros()-timestamp_oled1transfer;
    #ifdef useoled2
      updatescreen2 = true;
    #endif
  } //oled1_update
#endif //useoled1

#ifdef useoled2
  void oled2_init() {
    #ifdef usei2c
      display2.begin(SSD1306_SWITCHCAPVCC, oled2_address, oled_doreset,oled_sda, oled_scl, 800000UL);
    #else
      display2.begin(SSD1306_SWITCHCAPVCC);
    #endif
    display2.setRotation(OLED2_ROTATION);
    display2.setTextColor(WHITE);
    display2.setFont();
    display2.setTextSize(1);
    display2.clearDisplay();
  }

  void oled2_update() {
    uint8_t line;
    timestamp_oled2draw=micros();
    if (oledreinit) {
      oled2_init();
      oledreinit=false;
    }
    display2.clearDisplay();
    displaydraw = &display2;
    if (screen==screen_drive) {
        displaydraw->setFont(&ARIALNB18pt7b); displaydraw->setCursor(0,31); displaydraw->printf("%3d", int16_t((float)(bmsparsed->voltage/100.0f)*(float)bmsparsed->current/100.0f));
        displaydraw->setFont(&ARIALN9pt7b); displaydraw->setCursor(50,31); displaydraw->print(FPSTR(unit_power));
        displaydraw->setFont(&ARIALNB18pt7b); displaydraw->setCursor(65,31); displaydraw->printf("%3d",bmsparsed->remainingpercent);
        displaydraw->setFont(&ARIALN9pt7b); displaydraw->setCursor(116,31); displaydraw->print(FPSTR(unit_percent));
        displaydraw->setFont(&ARIALNB18pt7b); displaydraw->setCursor(0,63); displaydraw->printf("%2d",uint8_t((float)bmsparsed->voltage/100.0f));
        displaydraw->setFont(&ARIALNB9pt7b); displaydraw->setCursor(34,63); displaydraw->printf(".%1d",uint8_t((float)bmsparsed->voltage/10.0f) %10);
        displaydraw->setFont(&ARIALN9pt7b); displaydraw->setCursor(50,63); displaydraw->print(FPSTR(unit_volt));
        displaydraw->setFont(&ARIALNB18pt7b); displaydraw->setCursor(65,63); displaydraw->printf("%2d",int16_t((float)bmsparsed->current/100.0f));
        displaydraw->setFont(&ARIALNB9pt7b); displaydraw->setCursor(100,63); displaydraw->printf(".%1d",int16_t((float)bmsparsed->current/10.0f) %10);
        displaydraw->setFont(&ARIALN9pt7b); displaydraw->setCursor(116,63); displaydraw->print(FPSTR(unit_current));
    }
    if (screen==screen_stop) {
          display2.setFont();
          display2.setCursor(0,0);
          switch (subscreen) {
            case stopsubscreen_trip: //Temperatures - we have 2 Battery Temperatures, but we only display the higher one - both can be seen on detail screens
              sprintf(val1buf,"%4d",bmsparsed->health);
              if ((float)bmsparsed->temperature[0]>=(float)bmsparsed->temperature[1]) {
                  sprintf(val1buf,"%4.1f",(float)bmsparsed->temperature[0]-20.0f);  
                } else {
                  sprintf(val1buf,"%4.1f",(float)bmsparsed->temperature[1]-20.0f);  
                }
              sprintf(val2buf,"%4.1f",(float)escparsed->frametemp2/10.0f);
              sprintf(val3buf,"%4.1f",(float)bmsparsed->voltage/100.0f);
              sprintf(val4buf,"%5d",bmsparsed->remainingpercent);
              drawscreen_data(false, 4, true,
                FPSTR(label_bmstemp),&val1buf[0],FPSTR(unit_temp),
                FPSTR(label_esctemp),&val2buf[0],FPSTR(unit_temp),
                FPSTR(label_volt),&val3buf[0],FPSTR(unit_volt),
                FPSTR(label_percent),&val4buf[0],FPSTR(unit_percent));
              break;
            case stopsubscreen_minmax:
                display2.printf("FIX THIS SCREEN (%d/%d)\r\n\r\n",subscreen,stopsubscreens);
              break;
            case stopsubscreen_batt1:
                sprintf(val1buf,"%4d",bmsparsed->health);
                sprintf(val2buf,"%5d",bmsparsed->remainingcapacity);
                sprintf(val3buf,"%5d",bmsparsed->totalcapacity);
                sprintf(val4buf,"%02d/%02d",bmsparsed->temperature[0]-20,bmsparsed->temperature[1]-20);
                drawscreen_data(true, 4, true,
                    FPSTR(label_health),&val1buf[0],FPSTR(unit_percent),
                    FPSTR(label_capacityshort),&val2buf[0],FPSTR(unit_mah),
                    FPSTR(label_totalcapacityshort),&val3buf[0],FPSTR(unit_mah),
                    FPSTR(label_tempshort),&val4buf[0],FPSTR(unit_temp));
              break;
            case stopsubscreen_cells:
                display2.setFont(&ARIALN9pt7b); 
                display2.setCursor(0,12);
                if (conf_battcells>=9) { display2.printf(" 9: %5.03f ",(float)bmsparsed->Cell9Voltage/1000.0f); }
                if (conf_battcells>=10) { display2.printf("10: %5.03f",(float)bmsparsed->Cell10Voltage/1000.0f); }
                display2.setCursor(0,12+14);
                if (conf_battcells>=11) { display2.printf("11: %5.03f ",(float)bmsparsed->Cell11Voltage/1000.0f); }
                if (conf_battcells>=12) { display2.printf("12: %5.03f",(float)bmsparsed->Cell12Voltage/1000.0f); }
                display2.setCursor(0,12+14+14);
                display2.printf("Lo: %5.03f Hi: %5.03f", (float)(lowest)/1000.0f,(float)(highest)/1000.0f);
                display2.setCursor(0,12+14+14+14);
                display2.printf("T : %5.02f D : %5.03f", (float)bmsparsed->voltage/100.0f,(float)(highest-lowest)/1000.0f);
              break;
            case stopsubscreen_assets:
                display2.println(FPSTR(headline_espstate));
                display2.print(FPSTR(s_firmware)); 
                display2.println(swversion);
                display2.print(FPSTR(s_wlan));
                if (wlanstate==wlansearching) { display2.println(FPSTR(s_wlansearch)); }
                if (wlanstate==wlanconnected) { display2.print(FPSTR(s_wlancon)); display2.println(WiFi.SSID()); display2.println(WiFi.localIP()); }
                if (wlanstate==wlanap) { display2.print(FPSTR(s_wlanap)); display2.println(WiFi.softAPIP()); display2.println( WiFi.softAPIP()); }
                if (wlanstate==wlanoff) { display2.println(FPSTR(s_off)); }
                display2.print(FPSTR(s_bleoff));  
              break;
            case stopsubscreen_alarms:
                drawscreen_header(FPSTR(headline_alerts),subscreen,stopsubscreens);
                sprintf(val1buf,"%03d",alertcounter_escerror);
                sprintf(val1buf,"%03d",alertcounter_lockedalert);
                drawscreen_data(true, 2, false,
                    FPSTR(label_escerrorcounter),&val1buf[0],FPSTR(unit_percent),
                    FPSTR(label_alertcounter),&val2buf[0],FPSTR(unit_percent),
                    NULL,NULL,NULL,
                    NULL,NULL,NULL);
              break;
            default:
                display2.setFont();
                display2.setCursor(0,0);
                display2.print("OLED2 STOP DEFAULT");
              break;
          }
    }
    if (screen == screen_configmenu) {
        display2.setFont(sp_fontdata);
        cm_printValue(configcurrentitem);
    } //screen == screen_configmenu
    if (screen==screen_charging) {
                display2.setFont();
                display2.setCursor(0,0);
                if (conf_battcells>=1) { display2.printf("01: %5.03f ",(float)bmsparsed->Cell1Voltage/1000.0f); }
                if (conf_battcells>=2) { display2.printf("02: %5.03f  ",(float)bmsparsed->Cell2Voltage/1000.0f); }
                if (conf_battcells>=3) { display2.printf("03: %5.03f ",(float)bmsparsed->Cell3Voltage/1000.0f); }
                if (conf_battcells>=4) { display2.printf("04: %5.03f  ",(float)bmsparsed->Cell4Voltage/1000.0f); }
                if (conf_battcells>=5) { display2.printf("05: %5.03f ",(float)bmsparsed->Cell5Voltage/1000.0f); }
                if (conf_battcells>=6) { display2.printf("06: %5.03f  ",(float)bmsparsed->Cell6Voltage/1000.0f); }
                if (conf_battcells>=7) { display2.printf("07: %5.03f ",(float)bmsparsed->Cell7Voltage/1000.0f); }
                if (conf_battcells>=8) { display2.printf("08: %5.03f  ",(float)bmsparsed->Cell8Voltage/1000.0f); }
                if (conf_battcells>=9) { display2.printf("09: %5.03f ",(float)bmsparsed->Cell9Voltage/1000.0f); }
                if (conf_battcells>=10) { display2.printf("10: %5.03f  ",(float)bmsparsed->Cell10Voltage/1000.0f); }
                if (conf_battcells>=11) { display2.printf("11: %5.03f ",(float)bmsparsed->Cell11Voltage/1000.0f); }
                if (conf_battcells>=12) { display2.printf("12: %5.03f  ",(float)bmsparsed->Cell12Voltage/1000.0f); }
                display2.printf("L:  %5.03f H:  %5.03f\r\n", (float)(lowest)/1000.0f,(float)(highest)/1000.0f);
                display2.printf("T:  %5.02f D:  %5.03f", (float)bmsparsed->voltage/100.0f,(float)(highest-lowest)/1000.0f);
    }
    if (screen==screen_error) {
      display2.setFont(&ARIALN9pt7b);
      display2.setCursor(0,20);
      switch (escparsed->error) {
            case 10: display2.print(FPSTR(error_10)); break;
            case 11: display2.print(FPSTR(error_11)); break;
            case 12: display2.print(FPSTR(error_12)); break;
            case 13: display2.print(FPSTR(error_13)); break;
            case 14: display2.print(FPSTR(error_14)); break;
            case 15: display2.print(FPSTR(error_15)); break;
            case 18: display2.print(FPSTR(error_18)); break;
            case 21: display2.print(FPSTR(error_21)); break;
            case 22: display2.print(FPSTR(error_22)); break;
            case 23: display2.print(FPSTR(error_23)); break;
            case 24: display2.print(FPSTR(error_24)); break;
            case 26: display2.print(FPSTR(error_26)); break;
            case 27: display2.print(FPSTR(error_27)); break;
            case 28: display2.print(FPSTR(error_28)); break;
            case 29: display2.print(FPSTR(error_29)); break;
            case 31: display2.print(FPSTR(error_31)); break;
            case 35: display2.print(FPSTR(error_35)); break;
            case 36: display2.print(FPSTR(error_36)); break;
            case 39: display2.print(FPSTR(error_39)); break;
            case 40: display2.print(FPSTR(error_40)); break;
            default: display2.print(FPSTR(error_other)); break;
      }
    }
    if (screen==screen_timeout) {  
      display2.setFont(&ARIALN9pt7b);
      display2.setCursor(40,20);
      display2.print(FPSTR(s_timeout2));
      display2.setCursor(20,40);
      display2.print(FPSTR(s_timeout3));
      display2.setFont();
      if (wlanstate==wlansearching) { display2.setCursor(0,48); display2.print(FPSTR(s_wlan)); display2.print(FPSTR(s_wlansearch)); }
      if (wlanstate==wlanconnected) { display2.setCursor(0,48); display2.print("SSID: "); display2.print(WiFi.SSID()); display2.setCursor(0,57); display2.print("IP: "); display2.print(WiFi.localIP()); }
      if (wlanstate==wlanap) { display2.setCursor(0,48); display2.print("SSID: ");  display2.print(apssid); display2.setCursor(0,57); display2.print("IP: "); display2.print( WiFi.softAPIP()); }
      if (wlanstate==wlanoff) { display2.setCursor(40,57); display2.print(FPSTR(s_wlanoff)); }

    }
    if (screen==screen_locked) {  
      display2.setFont();
      display2.setCursor(39,31);
      display2.print(FPSTR(s_locked));
    }
    if (screen==screen_alert) {  
      display2.setFont();
      display2.setCursor(39,31);
      display2.print(FPSTR(s_alert));
    }
    if (showdialog) {
        drawscreen_dialog();
    }
    duration_oled2draw = micros()-timestamp_oled2draw;
    timestamp_oled2transfer=micros();
    display2.display();
    duration_oled2transfer = micros()-timestamp_oled2transfer;
  } //oled2_update
#endif //useoled2
