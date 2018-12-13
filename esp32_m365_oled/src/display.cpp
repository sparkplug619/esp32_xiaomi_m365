#include "display.h"
#include "strings.h"
#include "definitions.h"
#include "main.h"
#include "wlan.h"
#include "strings.h"
#include "config.h"    

#ifdef headunit
  #include "m365headunit.h"
#else
  #include "m365client.h"
#endif

//#define cmnav1 //navigation in config menu via throttle-swipe for menu-item-swipe
#define cmnav2  //navigation in config menu via throttle-press to advance one menu item

M365Display::M365Display() {
}    

void M365Display::begin(M365Client *mc) {
  _m365c = mc;
  #if (defined usei2c && defined useoled1)
      display1 = new Adafruit_SSD1306(oled_reset);
  #endif
  #if (defined usei2c && defined useoled2)
      display2 = new Adafruit_SSD1306(oled_reset);
  #endif

  #if (!defined usei2c && defined useoled1)
      display1 = new Adafruit_SSD1306(OLED_MISO, OLED_MOSI, OLED_CLK, OLED_DC, OLED1_RESET, OLED1_CS, oled_clock);
  #endif

  #if (!defined usei2c && defined useoled2)
      display2 = new Adafruit_SSD1306(OLED_MISO, OLED_MOSI, OLED_CLK, OLED_DC, OLED2_RESET, OLED2_CS, oled_clock);
  #endif
  #ifdef useoled1
    oled1_init();
    drawscreen_startscreen();
    display1->display();
    //delay(2000);
  #endif
  #ifdef useoled2
    oled2_init();
    display1->setRotation(0);
    drawscreen_startscreen();
    display1->setRotation(OLED1_ROTATION); //upside down
    display2->display();
  #endif
    timestamp_showsplashscreen= millis() + splashscreentimeout;
    screen = screen_splash;
  displaytask_start();
} //M365Display::begin

//screen layout/drawing

  void M365Display::drawscreen_startscreen() {
    display1->setFont();
    display1->setTextSize(1);
    display1->setTextColor(WHITE);
    display1->clearDisplay();
    display1->setCursor(35,5);
    #ifdef headunit
      display1->print("ESP32 HEADUNIT");
    #else
      #ifdef usengcode
        display1->print("ESP32 OLED NG");
      #else
        display1->print("ESP32 OLED");
      #endif
    #endif
    //display1->setCursor(64,20);
    //display1->print("for");
    display1->setCursor(35,25);
    display1->print("Xiaomi Mijia365");
    display1->setCursor(80,55);
    display1->print(swversion);
    display1->drawBitmap(0,0,  scooter, 64,64, 1);
  } //M365Display::drawscreen_startscreen()

  void M365Display::drawscreen_header(const char *h, uint8_t scrnum, uint8_t scrtotal) {
    //blestruct* _m365c->bleparsed = (blestruct*)bledata;
    displaydraw->setFont();
    displaydraw->setCursor(0,0);
    if (scrtotal!=0) {
      displaydraw->printf("%-15.15s (%d/%d)\r\n",FPSTR(h),scrnum,scrtotal);
    } else {
      displaydraw->printf("%-21.21s\r\n",FPSTR(h));
    }
  } //M365Display::drawscreen_header

  void M365Display::drawscreen_data(
      bool headline, uint8_t lines, bool showunits,
      const char *l1, char *v1, const char *u1,
      const char *l2, char* v2, const char *u2,
      const char *l3, char* v3, const char *u3,
      const char *l4, char* v4, const char *u4) {
    uint8_t i;
    uint8_t line=1;
    for (i=0;i<lines;i++) {
      switch(i) {
        case 0:
            if (headline & (lines==4)) { line = dataoffset+baselineoffset; }
            if (!headline & (lines==4)) { line = baselineoffset; }
            if (headline & (lines<=3)) { line = dataoffset+baselineoffset+5;}
            if (!headline & (lines<=3)) { line = baselineoffset; }
            displaydraw->setFont(sp_fontlabel); displaydraw->setCursor(sp_lx,line); displaydraw->print(FPSTR(l1));
            if (showunits) {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dx,line); displaydraw->printf(v1);
              displaydraw->setCursor(sp_ux,line); displaydraw->setFont(sp_fontunit); displaydraw->print(FPSTR(u1));
            } else {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dxnu,line); displaydraw->printf(v1);
            }
          break;
        case 1:
            if (headline & (lines==4)) { line = dataoffset+baselineoffset*2+linespace; }
            if (!headline & (lines==4)) { line = baselineoffset*2+linespace+2;}
            if (headline & (lines<=3)) { line = dataoffset+baselineoffset*2+10; }
            if (!headline & (lines<=3)) { line = baselineoffset*2+5+4; }
            displaydraw->setFont(sp_fontlabel); displaydraw->setCursor(sp_lx,line); displaydraw->print(FPSTR(l2));
            if (showunits) {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dx,line); displaydraw->printf(v2);
              displaydraw->setCursor(sp_ux,line); displaydraw->setFont(sp_fontunit); displaydraw->print(FPSTR(u2));
            } else {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dxnu,line); displaydraw->printf(v2);
            }
          break;
        case 2:
            if (headline & (lines==4)) { line = dataoffset+baselineoffset*3+linespace*2; }
            if (!headline & (lines==4)) { line = baselineoffset*3+linespace*2+5;}
            if (headline & (lines<=3)) { line = dataoffset+baselineoffset*3+15; }
            if (!headline & (lines<=3)) { line = baselineoffset*3+10+8; }
            displaydraw->setFont(sp_fontlabel); displaydraw->setCursor(sp_lx,line); displaydraw->print(FPSTR(l3));
            if (showunits) {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dx,line); displaydraw->printf(v3);
              displaydraw->setCursor(sp_ux,line); displaydraw->setFont(sp_fontunit); displaydraw->print(FPSTR(u3));
            } else {
              displaydraw->setFont(sp_fontdata); displaydraw->setCursor(sp_dxnu,line); displaydraw->printf(v3);
            }
          break;
        case 3:
            if (headline & (lines==4)) { line = dataoffset+baselineoffset*4+linespace*3; }
            if (!headline & (lines==4)) { line = baselineoffset*4+linespace*3+8;}
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
  } //M365Display::drawscreen_data

//popup "windows"
  void M365Display::infopopup (char *_popuptitle, char *_popuptext, uint16_t duration) {
    if (!showdialog) {
      sprintf(popuptitle, "%s", _popuptitle);
      sprintf(popuptext, "%s", _popuptext);
      showpopup = true;
      popuptimestamp = millis()+duration;
    }
  } // M365Display::infopopup

  void M365Display::alertpopup (char *_popuptitle, char *_popuptext, uint16_t duration) {
    if (!showdialog) {
      sprintf(popuptitle, "%s", _popuptitle);
      sprintf(popuptext, "%s", _popuptext);
      showpopup = true;
      popuptimestamp = millis()+duration;
      if (conf_beeponalert) {
        _m365c->sendcommand = cmd_beep5;
      }
    }
  } //M365Display::alertpopup

  void M365Display::drawscreen_popup() {
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
  } //M365Display::drawscreen_popup

  void M365Display::dialog_edit_int8(const char *_dialogtitle, uint8_t *_value, uint8_t _minvalue, uint8_t _maxvalue) {
    sprintf(diag_title, "%s", _dialogtitle);
    diag_value = _value;
    diag_min = _minvalue;
    diag_max = _maxvalue;
    showdialog = true;
  } //M365Display::dialog_edit_int8

  void M365Display::drawscreen_dialog() {
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
  } //M365Display::drawscreen_dialog

//configmenu helpers:
  void M365Display::cm_handleactions() {
    switch(configcurrentitem) {
      //menu navigation
      case cms_exitscooter:
      case cms_exitalerts:
      case cms_exitesp:
          configcurrentmenu = cmm_root;
          subscreen = 0;
          configwindowsize = (uint8_t)((throttlemax-throttlemin)/(menuitemcount[configcurrentmenu>>8]-1));   
        break;  
      case cms_exitroot:
          screen = screen_stop;
          subscreen = 0;
          if (configchanged) { saveconfig(); applyconfig(); }
        break;
      case cms_smscooter:
          configcurrentmenu =  cmm_scooter;
          subscreen = 0;
          configwindowsize = (uint8_t)((throttlemax-throttlemin)/(menuitemcount[configcurrentmenu>>8]-1));   
        break;
      case cms_smalerts:
          configcurrentmenu =  cmm_alerts;
          subscreen = 0;
          configwindowsize = (uint8_t)((throttlemax-throttlemin)/(menuitemcount[configcurrentmenu>>8]-1));   
        break;
      case cms_smesp:
          configcurrentmenu =  cmm_esp;
          subscreen = 0;
          configwindowsize = (uint8_t)((throttlemax-throttlemin)/(menuitemcount[configcurrentmenu>>8]-1));   
        break;        
      
      //menu items
      case cms_light:
          switch(_m365c->escparsed->taillight) {
            case 0: _m365c->sendcommand = cmd_light_on; break;
            case 2: _m365c->sendcommand = cmd_light_off; break;
          }
        break;
      case cms_cruise:
          switch(_m365c->escparsed->cruisemode) {
            case 0: _m365c->sendcommand = cmd_cruise_on; break;
            case 1: _m365c->sendcommand = cmd_cruise_off; break;
          }
        break;
      case cms_kers:
          switch(_m365c->escparsed->kers) {
            case 0: _m365c->sendcommand = cmd_kers_medium; break;
            case 1: _m365c->sendcommand = cmd_kers_strong; break;
            case 2: _m365c->sendcommand = cmd_kers_weak; break;
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
      case cms_beeponalert:
          conf_beeponalert = !conf_beeponalert;
          _m365c->sendcommand = cmd_beep5; //just for testing purposes, remove later...
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
      case cms_wifionstartup:
          conf_espwifionstart = !conf_espwifionstart;
          configchanged = true;
        break;

      case cms_changelock:
          if (_m365c->escparsed->lockstate==0x00) { 
            _m365c->sendcommand = cmd_lock;
          } else {
            _m365c->sendcommand = cmd_unlock;
          }
          screen = screen_stop;
          subscreen = 0;
          if (configchanged) { saveconfig(); applyconfig(); }
        break;
      case cms_turnoff:
          _m365c->sendcommand = cmd_turnoff;
          if (configchanged) { saveconfig(); applyconfig(); }
        break;
    } //switch curline
  } //M365Display::handle_configmenuactions

  void M365Display::cm_printKey(uint16_t entryid) {
    //displaydraw->printf("%d ",entryid);
    switch(entryid) {
      case cms_light: displaydraw->print(FPSTR(menu_light));
        break;
      case cms_cruise: 
          displaydraw->print(FPSTR(menu_cruise));
        break;
      case cms_kers: 
          displaydraw->print(FPSTR(menu_kers));
        break;
      case cms_ws: displaydraw->print(FPSTR(menu_wheelsize));
        break;
      case cms_unit: displaydraw->print(FPSTR(menu_unit));
        break;
      case cms_buv: displaydraw->print(FPSTR(menu_battalertlowvoltage));
        break;
      case cms_bc: displaydraw->print(FPSTR(menu_battcells));
        break;
      case cms_bac: displaydraw->print(FPSTR(menu_battalertcell));
        break;
      case cms_bat: displaydraw->print(FPSTR(menu_battalerttemp));
        break;
      case cms_eat: displaydraw->print(FPSTR(menu_escalerttemp));
        break;
      /*case cms_flashprotection: display1->print("***shprot");
        break;
      case cms_navigation: display1->print("***igation");
        break;*/
      case cms_beeponalert: displaydraw->print(FPSTR(menu_beeponalert));
        break;
      case cms_busmode: displaydraw->print(FPSTR(menu_espbusmode));
        break;
      case cms_wifirestart: displaydraw->print(FPSTR(menu_espwifirestart));
        break;
      case cms_wifionstartup: displaydraw->print(FPSTR(menu_espwifionstart));
        break;
      case cms_changelock: 
            if (_m365c->escparsed->lockstate==0x00) {
              displaydraw->print(FPSTR(menu_m365lock));
            } else {
              displaydraw->print(FPSTR(menu_m365unlock));
            }
        break;
      case cms_turnoff: displaydraw->print(FPSTR(menu_m365turnoff));
        break;
      case cms_exitroot:
      case cms_exitscooter:
      case cms_exitalerts:
      case cms_exitesp:
          displaydraw->print(FPSTR(menu_exit));
        break;
      case cms_smscooter:
          displaydraw->print(FPSTR(menu_scooter));
        break;
      case cms_smalerts:
          displaydraw->print(FPSTR(menu_alerts));
        break;
      case cms_smesp:
          displaydraw->print(FPSTR(menu_esp));
        break;
    } //switch curline
  } //M365Display::cm_printKey

  void M365Display::cm_printValue(uint16_t entryid) {
    #if!defined useoled2
      displaydraw->setCursor(5,dataoffset+baselineoffset + 3*(baselineoffset+linespace));
      displaydraw->drawFastHLine(0,dataoffset+baselineoffset + 2*(baselineoffset+linespace)-linespace-linespace,128,WHITE);
    #endif
    #ifdef useoled2
      displaydraw->setCursor(5,31-baselineoffset/2);
    #endif
    switch(entryid) {
      case cms_light:
            switch(_m365c->escparsed->taillight) {
              case 0: displaydraw->print(FPSTR(menu_off)); break;
              case 2: displaydraw->print(FPSTR(menu_on)); break;
            }
        break;
      case cms_cruise: 
            switch(_m365c->escparsed->cruisemode) {
              case 0: displaydraw->print(FPSTR(menu_off)); break;
              case 1: displaydraw->print(FPSTR(menu_on)); break;
            }
        break;
      case cms_kers: 
            switch(_m365c->escparsed->kers) {
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
      case cms_wifionstartup:
          if (conf_espwifionstart) { 
            displaydraw->print(FPSTR(menu_on));
          } else { 
            displaydraw->print(FPSTR(menu_off)); 
          }
        break;
    } //switch curline
  } //M365Display::cm_printValue

//handle screen switching, throttle & brake user interactions, toggle screen update,...  
  void M365Display::loop() {
    timestamp_oledstart=micros();
    //### start of screen switching
    uint8_t oldscreen = screen;
  
    //splashscreen shown? do nothing...
    if ((screen==screen_splash) & (millis()>timestamp_showsplashscreen)) {
      screen = screen_stop;
      updatescreen=true;
    } else {
      //no splashscreen, go on with normal screen-switching:
        //1st. prio: Data/Bus Timeout
          //switch to timeout screen
          if (((_m365c->m365packettimestamp+m365packettimeout)<millis()) & (screen!=screen_timeout)) {
            screen=screen_timeout;
            updatescreen=true;
            return;
          }
          //leave timeout screen
          if ((screen==screen_timeout) & ((_m365c->m365packettimestamp+m365packettimeout)>millis())) {
            if (_m365c->escparsed->speed>0) {
              screen=screen_drive;  
            } else {
              screen=screen_stop;  
            }
            updatescreen=true; 
          }

        //2nd prio - locked or alert in lockmode?
          if ((_m365c->escparsed->lockstate==0x02) & (screen!=screen_configmenu)) {
            screen=screen_locked;
          }
          if ((_m365c->escparsed->lockstate==0x09) & (screen!=screen_configmenu)) {
            screen=screen_alert;
          }
        
        //"button" handling
          if (_m365c->newdata & (_m365c->bleparsed->brake>=buttonbrakepressed1) & !brakebuttonpressed) {
            buttonbrakepressedtimestamp = millis();
            brakebuttonpressed=true;
            brakebuttonstate=0;
          }
          if (_m365c->newdata & (_m365c->bleparsed->brake<buttonbrakepressed1) & brakebuttonpressed) {
            if (millis()>buttonbrakepressedtimestamp+buttonbrakelongpressedduration) { 
              brakebuttonstate=2;
            } else {
              if (millis()>buttonbrakepressedtimestamp+buttonbrakeshortpressedduration) { 
                brakebuttonstate=1;
              }
            }
            if (brakebuttonstate!=0) { brakebuttonpressed=false; }
          }
#ifdef cmnav1
          if (_m365c->newdata & (_m365c->bleparsed->throttle>=buttonthrottlepressed1) & !throttlebuttonpressed) {
            buttonthrottlepressedtimestamp = millis();
            throttlebuttonpressed=true;
            throttlebuttonstate=0;
          }
          if (_m365c->newdata & (_m365c->bleparsed->throttle<buttonthrottlepressed1) & throttlebuttonpressed) {
            if (millis()>buttonthrottlepressedtimestamp+buttonthrottlelongpressedduration) { 
              throttlebuttonstate=2;
            } else {
              if (millis()>buttonthrottlepressedtimestamp+buttonthrottleshortpressedduration) { 
                throttlebuttonstate=1;
              }
            }
            if (throttlebuttonstate!=0) { throttlebuttonpressed=false; }
          }
#endif
#ifdef cmnav2
          if (_m365c->newdata & (_m365c->bleparsed->throttle>=buttonthrottlepressed2) & !throttlebuttonpressed) {
            buttonthrottlepressedtimestamp = millis();
            throttlebuttonpressed=true;
            throttlebuttonstate=0;
          }
          if (_m365c->newdata & (_m365c->bleparsed->throttle<buttonthrottlepressed2) & throttlebuttonpressed) {
            if (millis()>buttonthrottlepressedtimestamp+buttonthrottlelongpressedduration) { 
              throttlebuttonstate=2;
            } else {
              if (millis()>buttonthrottlepressedtimestamp+buttonthrottleshortpressedduration) { 
                throttlebuttonstate=1;
              }
            }
            if (throttlebuttonstate!=0) { throttlebuttonpressed=false; }
          }
#endif
          if (_m365c->newdata & (_m365c->bleparsed->brake>=buttonbrakepressed1) & (_m365c->bleparsed->throttle>=buttonthrottlepressed1) & !bothbuttonpressed & (screen!=screen_configmenu)) {
            buttonbothpressedtimestamp = millis();
            bothbuttonpressed=true;
            bothbuttonstate=0;
          }
          if (_m365c->newdata & (_m365c->bleparsed->brake<buttonbrakepressed1) & (_m365c->bleparsed->throttle<buttonthrottlepressed1) & bothbuttonpressed) {
            if (millis()>buttonbothpressedtimestamp+buttonbothlongpressedduration) { 
              bothbuttonstate=2;
            } else {
              if (millis()>buttonbothpressedtimestamp+buttonbothshortpressedduration) { 
                bothbuttonstate=1;
              }
            }
            if (bothbuttonstate!=0) { bothbuttonpressed=false; }
          }
        
        //enter configmenu
          if ((bothbuttonstate!=0) & (_m365c->escparsed->speed==0) & (screen!=screen_configmenu)) {
            screen = screen_configmenu;
            subscreen = 0;
            configcurrentmenu = cmm_root;
            configstartindex = _max(subscreen - configlinesabove,0);
            configendindex = _min(configstartindex + confignumlines-1,menuitemcount[configcurrentmenu>>8]-1);
            if ((configendindex-confignumlines+1)<configstartindex) {
              configstartindex = configendindex - confignumlines+1;
            }
            configchanged = false;
            updatescreen = true;
            infopopup((char*)"MENU", (char*)"THR <>, BRK ok", 500);
          }

        //exit from configmenu if speed > 5km/h
          if ((screen==screen_configmenu) & (abs((float)_m365c->escparsed->speed/1000.0f)>5.0f)) {
          //if (brakebuttonstate==1 & screen==screen_stop & subscreen == stopsubscreens) {
            if (configchanged) { saveconfig(); }
            screen = screen_drive;
            subscreen = 0;
            updatescreen = true;
          }

        //handle configscreen & responses to brake/throttle actions by user:
          if (screen==screen_configmenu) {
            if (showdialog) {
              //1st prio: handle value-change dialogs
              updatescreen = true;
              *diag_value = map(_m365c->bleparsed->throttle,throttlemin,throttlemax,diag_min,diag_max);
              if (brakebuttonstate!=0) { //brake pressed, exit dialog?
                showdialog = false;
              }
            } else {
              //2nd prio: handle brake-button actions
              //if (screen==screen_configmenu & brakebuttonstate!=0 & !showdialog) {
              if ((brakebuttonstate!=0) & !showpopup) {  
                cm_handleactions();
                configstartindex = _max(subscreen - configlinesabove,0);
                configendindex = _min(configstartindex + confignumlines-1,menuitemcount[configcurrentmenu>>8]-1);
                if ((configendindex-confignumlines+1)<configstartindex) {
                  configstartindex = configendindex - confignumlines+1;
                }
                updatescreen = true;
              }
            }
          }

        #ifdef cmnav1
        //configmenu navigaton version 1 via throttle swipe:
          if (_m365c->newdata & (screen==screen_configmenu) & !(showdialog|showpopup)) {
            uint8_t oldsubscreen = subscreen;
            if (_m365c->bleparsed->throttle>(throttlemin+5)) {
              subscreen = ((_m365c->bleparsed->throttle-throttlemin) / configwindowsize)+1;
              //subscreen=_min(subscreen,menuitemcount[configcurrentmenu>>8]-1);
              windowsubpos = (uint8_t)((float)((_m365c->bleparsed->throttle-throttlemin) % configwindowsize)*(float)oledwidth/(float)configwindowsize);
            } else {
              subscreen=0;
              windowsubpos=0;
            }
            //if ((subscreen)>menuitemcount[configcurrentmenu>>8]) { subscreen=menuitemcount[configcurrentmenu>>8]; }
            subscreen=_min(subscreen,menuitemcount[configcurrentmenu>>8]-1);
            if (subscreen!=oldsubscreen) { 
              configstartindex = _max(subscreen - configlinesabove,0);
              configendindex = _min(configstartindex + confignumlines-1,menuitemcount[configcurrentmenu>>8]-1);
              if ((configendindex-confignumlines+1)<configstartindex) {
                configstartindex = configendindex - confignumlines+1;
              }
              updatescreen = true; 
            }
          }
        #endif
        #ifdef cmnav2
        //configmenu navigaton version 2 via throttle press:
          if (_m365c->newdata & (screen==screen_configmenu) & !(showdialog|showpopup)) {
            uint8_t oldsubscreen = subscreen;
            if (throttlebuttonstate==1) {
              subscreen++;
              windowsubpos=0;
            }
            if (subscreen==menuitemcount[configcurrentmenu>>8]) {
              subscreen=0;  
            }
            if (subscreen!=oldsubscreen) { 
              configstartindex = _max(subscreen - configlinesabove,0);
              configendindex = _min(configstartindex + confignumlines-1,menuitemcount[configcurrentmenu>>8]-1);
              if ((configendindex-confignumlines+1)<configstartindex) {
                configstartindex = configendindex - confignumlines+1;
              }
              updatescreen = true; 
            }
          }
        #endif       
        //charging screens: 
          if (_m365c->newdata & (_m365c->escparsed->speed==0) & (_m365c->bmsparsed->current<0) & (screen!=screen_charging)) { 
            if  (screen!=screen_locked) {
              //only display if not locked...
              screen=screen_charging; 
            }
            //timeout_oled=millis()+oledchargestarttimeout;
            _m365c->capacitychargestart = _m365c->bmsparsed->remainingcapacity;
            updatescreen=true;
          }
          if (_m365c->newdata & (screen==screen_charging) & (abs((float)_m365c->escparsed->speed/1000.0f)>2.0f)) { 
            if (abs((float)_m365c->escparsed->speed/1000.0f)>0.9f) {
              screen=screen_drive;  
            } else {
              screen=screen_stop;  
            }
            updatescreen=true;
          }

          //switching of subscreens via throttle while we are in CHARGE mode, only when single-screen setup
          #if !defined useoled2
            if (_m365c->newdata & (screen==screen_charging)) {
              uint8_t oldsubscreen = subscreen;
              if (_m365c->bleparsed->throttle>=throttlemin) {
                subscreen = ((_m365c->bleparsed->throttle-throttlemin) / chargewindowsize)+1;
                windowsubpos = (uint8_t)((float)((_m365c->bleparsed->throttle-throttlemin) % chargewindowsize)*(float)oledwidth/(float)chargewindowsize);
              } else {
                subscreen=1;
                windowsubpos=0;
              }
              if ((subscreen)>chargesubscreens) { subscreen=chargesubscreens; }
              if (subscreen!=oldsubscreen) { updatescreen = true; }
            }
          #endif

        //switch between driving/stop screens:
          if (_m365c->newdata & (screen==screen_drive) & (abs((float)_m365c->escparsed->speed/1000.0f)<0.5f)) {
            screen=screen_stop;
            updatescreen=true;
          }
          if (_m365c->newdata & (screen==screen_stop) & (abs((float)_m365c->escparsed->speed/1000.0f)>0.9f)) {
            screen=screen_drive;
            updatescreen=true;
          }

        //stop_screen: switching of subscreens via throttle-swipe
          if (_m365c->newdata & (screen==screen_stop)) {
            uint8_t oldsubscreen = subscreen;
            if (_m365c->bleparsed->throttle>throttlemin+5) {
              subscreen = ((_m365c->bleparsed->throttle-throttlemin) / stopwindowsize)+1;
              windowsubpos = (uint8_t)((float)((_m365c->bleparsed->throttle-throttlemin) % stopwindowsize)*(float)oledwidth/(float)stopwindowsize);
            } else {
              subscreen=0;
              windowsubpos=0;
            }
            if ((subscreen)>stopsubscreens) { subscreen=stopsubscreens; }
            if (subscreen!=oldsubscreen) { updatescreen = true; }
          }
        /* old version without displaytask
        //update with new data, but honor refresh-rate
          if (_m365c->newdata & (olednextrefreshtimestamp<millis())) { 
          //if ((olednextrefreshtimestamp<millis())) { 
            updatescreen=true;
            _m365c->newdata=false; 
          }
        */
       //also update screen when no screenswitching/user interaction, but when we have new data:
          if (_m365c->newdata) { 
            updatescreen=true;
          }

        //update subscriptions if screen has been changed
          if (oldscreen!=screen) { 
            //DebugSerial.printf("---screen: %d\r\n",screen);
            _m365c->subscribedrequests=_m365c->rqsarray[screen];
          }

        //reset _m365c->newdata flag if we consumed it
          if (updatescreen) { _m365c->newdata=false; }
        
        //DebugSerial.printf("### Br: %03d Thr: %03d B: %01d T: %01d BT: %01d Scr: %01d\r\n",_m365c->escparsed->brake, _m365c->escparsed->throttle, brakebuttonstate, throttlebuttonstate, bothbuttonstate, screen);
        //reset buttonstates
          brakebuttonstate=0;
          throttlebuttonstate=0;
          bothbuttonstate=0;

        //popup timeout handling
        if ((showpopup) & (millis() > popuptimestamp)) {
          showpopup = false;
        }
    } //if else screen==screen_splash
    //### end of screen switching

    //check if oledreinitduration has passed and toggle flags for re-initialization of displays
      if ((millis() > oledreinittimestamp) & (screen!=screen_splash)) {
          if (oledreinittimestamp!=0) {
            oledreinit = true;
          }
          oledreinittimestamp = millis() + oledreinitduration;
      }

    //screendrawing in a task: update both screens at the same time as we are not blocking other code
      if ((updatescreen) & (millis()>olednextrefreshtimestamp))  {
        olednextrefreshtimestamp=millis()+oledrefreshanyscreen;
        oled1_update();
        #ifdef useoled2
          oled2_update();
        #endif
        updatescreen=false;
        //DebugSerial.printf("---OLED1----- %02d %d\r\n",screen, millis());
      }

    duration_oled = micros()-timestamp_oledstart;
  } //M365Display::loop

//oled init & screen-drawing
#ifdef useoled1
  void M365Display::oled1_init() {
    #ifdef usei2c
      display1->begin(SSD1306_SWITCHCAPVCC, oled1_address, oled_doreset, oled_sda, oled_scl, oled_clock);
    #else
      display1->begin(SSD1306_SWITCHCAPVCC);
    #endif
    display1->setRotation(OLED1_ROTATION);
    display1->setTextColor(WHITE);
    display1->setFont();
    display1->setTextSize(1);
    display1->clearDisplay();
  } //M365Display::oled1_init

  void M365Display::oled1_update() {
    //uint8_t line;
    uint8_t lineitem;
    timestamp_oled1draw=micros();

    if (oledreinit) {
      oled1_init();
      #if !defined useoled2
        oledreinit=false;
      #endif
    }
    if (screen!=screen_splash) {
      display1->clearDisplay();
    }
    displaydraw = display1;
    
    if (screen==screen_drive) {
        displaydraw->setFont(&ariblk42pt7b);
        displaydraw->setCursor(0,62);
        displaydraw->printf("%02d", uint8_t(abs((float)_m365c->escparsed->speed/1000.0f*wheelfact)));
        displaydraw->setFont(&ARIALNB18pt7b);
        displaydraw->setCursor(108,28);
        displaydraw->printf("%01d", uint16_t((float)_m365c->escparsed->speed/100.0f*wheelfact) %10); 
        displaydraw->setFont(&ARIALNB9pt7b);
        if (_m365c->x1parsed->light==0x64) { 
          displaydraw->setCursor(118,45);
          displaydraw->print("L");
        }
        displaydraw->setCursor(118,63);
        if (_m365c->x1parsed->mode<2) { 
          //display1->print("N"); //normal mode
        } else {
          displaydraw->print("E"); //eco mode
        }
    }
    if (screen==screen_stop) {
          displaydraw->setFont();
          displaydraw->setCursor(0,0);
          switch (subscreen) {
            case stopsubscreen_trip: //Trip Info: Average Speed, Distance, Time, Average Speed 
                drawscreen_header(FPSTR(headline_tripinfo),0,0);
                sprintf(val1buf,"%05.2f",(float)_m365c->escparsed->averagespeed/1000.0f*wheelfact);
                sprintf(val2buf,"%05.2f",(float)_m365c->escparsed->tripdistance/100.0f*wheelfact);
                sprintf(val3buf,"%02d:%02d",_m365c->escparsed->triptime/60,_m365c->escparsed->triptime % 60);
                sprintf(val4buf,"%05.2f",(float)_m365c->escparsed->remainingdistance/100.0f);
                  drawscreen_data(true, 4, true,
                      FPSTR(label_averageshort),&val1buf[0],(conf_unit==0?FPSTR(unit_speed_km):FPSTR(unit_speed_miles)),
                      FPSTR(label_distanceshort),&val2buf[0],(conf_unit==0?FPSTR(unit_distance_km):FPSTR(unit_distance_miles)),
                      FPSTR(label_time),&val3buf[0],FPSTR(label_seconds),
                      FPSTR(label_remainingshort),&val4buf[0],(conf_unit==0?FPSTR(unit_distance_km):FPSTR(unit_distance_miles)));
              break;
              #if !defined useoled2
                case stopsubscreen_temp: //Single
                    drawscreen_header(FPSTR(headline_temperature),subscreen,stopsubscreens);
                    sprintf(val1buf,"%4.1f",(float)_m365c->bmsparsed->temperature[0]-20.0f);
                    sprintf(val2buf,"%4.1f",(float)_m365c->bmsparsed->temperature[1]-20.0f);
                    sprintf(val3buf,"%4.1f",(float)_m365c->escparsed->frametemp2/10.0f);
                    drawscreen_data(true, 3, true,
                      FPSTR(label_batt1),&val1buf[0],FPSTR(unit_temp),
                      FPSTR(label_batt2),&val2buf[0],FPSTR(unit_temp),
                      FPSTR(label_esc),&val3buf[0],FPSTR(unit_temp),
                      NULL,NULL,NULL);
                  break;
              #endif
              case stopsubscreen_minmax: //Single/Dual/Same
              //fix this screen - single and dual version...
                displaydraw->printf("TRIP Min/Max    (%d/%d)\r\n\r\n",subscreen,stopsubscreens);
                displaydraw->printf("Speed : %5.1f %5.1f\r\n",(float)_m365c->speed_min/1000.0f,(float)_m365c->speed_max/1000.0f);
                displaydraw->printf("Ampere: %5.1f %5.1f A\r\n",(float)_m365c->current_min/100.0f,(float)_m365c->current_max/100.0f);
                displaydraw->printf("Watt  : %5d %5d W\r\n",_m365c->watt_min,_m365c->watt_max);
                displaydraw->printf("BattT1: %3d.0 %3d.0 C\r\n",_m365c->tb1_min-20,_m365c->tb1_max-20); display1->drawCircle(117,42,1,WHITE);
                displaydraw->printf("BattT2: %3d.0 %3d.0 C\r\n",_m365c->tb2_min-20,_m365c->tb2_max-20); display1->drawCircle(117,50,1,WHITE);
                displaydraw->printf("ESC T : %5.1f %5.1f C\r\n",(float)_m365c->te_min/10.0f,(float)_m365c->te_max/10.0f); display1->drawCircle(117,58,1,WHITE);
              break;
              case stopsubscreen_batt1: //Single/Dual/Same
                #if !defined useoled2
                  drawscreen_header(FPSTR(headline_battery1),subscreen,stopsubscreens);
                #else
                  drawscreen_header(FPSTR(headline_batterystatus),subscreen,stopsubscreens);
                #endif
                sprintf(val1buf,"%5.2f",(float)_m365c->bmsparsed->voltage/100.0f);
                sprintf(val2buf,"%4d%%",_m365c->bmsparsed->remainingpercent);
                sprintf(val3buf,"%4d",_m365c->bmsparsed->cycles);
                sprintf(val4buf,"%4d",_m365c->bmsparsed->chargingtimes);
                drawscreen_data(true, 4, true,
                    FPSTR(label_volt),&val1buf[0],FPSTR(unit_volt),
                    FPSTR(label_percent),&val2buf[0],FPSTR(unit_percent),
                    FPSTR(label_cycles),&val3buf[0],FPSTR(unit_count),
                    FPSTR(label_charges),&val4buf[0],FPSTR(unit_count));
              break;
              #if !defined useoled2  
                case stopsubscreen_batt2: //Single Screen Batt Info 2: Health/Cycles/Charge Num/Prod Date
                    drawscreen_header(FPSTR(headline_battery2),subscreen,stopsubscreens);
                    sprintf(val1buf,"%4d",_m365c->bmsparsed->health);
                    sprintf(val2buf,"%5d",_m365c->bmsparsed->remainingcapacity);
                    sprintf(val3buf,"%5d",_m365c->bmsparsed->totalcapacity);
                    sprintf(val4buf,"%02d/%02d",_m365c->bmsparsed->temperature[0]-20,_m365c->bmsparsed->temperature[1]-20);
                    drawscreen_data(true, 4, true,
                        FPSTR(label_health),&val1buf[0],FPSTR(unit_percent),
                        FPSTR(label_capacityshort),&val2buf[0],FPSTR(unit_mah),
                        FPSTR(label_totalcapacityshort),&val3buf[0],FPSTR(unit_mah),
                        FPSTR(label_tempshort),&val4buf[0],FPSTR(unit_temp));
                  break;
              #endif
              case stopsubscreen_cells: //single/dual/different
                drawscreen_header(FPSTR(headline_cellvolt),subscreen,stopsubscreens);
                #if !defined useoled2
                  if (conf_battcells>=1) { displaydraw->printf("01: %5.03f ",(float)_m365c->bmsparsed->Cell1Voltage/1000.0f); }
                  if (conf_battcells>=2) { displaydraw->printf("02: %5.03f  ",(float)_m365c->bmsparsed->Cell2Voltage/1000.0f); }
                  if (conf_battcells>=3) { displaydraw->printf("03: %5.03f ",(float)_m365c->bmsparsed->Cell3Voltage/1000.0f); }
                  if (conf_battcells>=4) { displaydraw->printf("04: %5.03f  ",(float)_m365c->bmsparsed->Cell4Voltage/1000.0f); }
                  if (conf_battcells>=5) { displaydraw->printf("05: %5.03f ",(float)_m365c->bmsparsed->Cell5Voltage/1000.0f); }
                  if (conf_battcells>=6) { displaydraw->printf("06: %5.03f  ",(float)_m365c->bmsparsed->Cell6Voltage/1000.0f); }
                  if (conf_battcells>=7) { displaydraw->printf("07: %5.03f ",(float)_m365c->bmsparsed->Cell7Voltage/1000.0f); }
                  if (conf_battcells>=8) { displaydraw->printf("08: %5.03f  ",(float)_m365c->bmsparsed->Cell8Voltage/1000.0f); }
                  if (conf_battcells>=9) { displaydraw->printf("09: %5.03f  ",(float)_m365c->bmsparsed->Cell9Voltage/1000.0f); }
                  if (conf_battcells>=10) { displaydraw->printf("10: %5.03f ",(float)_m365c->bmsparsed->Cell10Voltage/1000.0f); }
                  if (conf_battcells>=11) { displaydraw->printf("11: %5.03f ",(float)_m365c->bmsparsed->Cell11Voltage/1000.0f); }
                  if (conf_battcells>=12) { displaydraw->printf("12: %5.03f  ",(float)_m365c->bmsparsed->Cell12Voltage/1000.0f); }
                  displaydraw->printf("%s %5.03f", FPSTR(label_maxdiff), (float)(_m365c->highest-_m365c->lowest)/1000.0f);
                #else
                  displaydraw->setFont(&ARIALN9pt7b); 
                  displaydraw->setCursor(0,21);
                  if (conf_battcells>=1) { displaydraw->printf(" 1: %5.03f ",(float)_m365c->bmsparsed->Cell1Voltage/1000.0f); }
                  if (conf_battcells>=2) { displaydraw->printf(" 2: %5.03f",(float)_m365c->bmsparsed->Cell2Voltage/1000.0f); }
                  displaydraw->setCursor(0,35);
                  if (conf_battcells>=3) { displaydraw->printf(" 3: %5.03f ",(float)_m365c->bmsparsed->Cell3Voltage/1000.0f); }
                  if (conf_battcells>=4) { displaydraw->printf(" 4: %5.03f",(float)_m365c->bmsparsed->Cell4Voltage/1000.0f); }
                  displaydraw->setCursor(0,49);
                  if (conf_battcells>=5) { displaydraw->printf(" 5: %5.03f ",(float)_m365c->bmsparsed->Cell5Voltage/1000.0f); }
                  if (conf_battcells>=6) { displaydraw->printf(" 6: %5.03f",(float)_m365c->bmsparsed->Cell6Voltage/1000.0f); }
                  displaydraw->setCursor(0,63);
                  if (conf_battcells>=7) { displaydraw->printf(" 7: %5.03f ",(float)_m365c->bmsparsed->Cell7Voltage/1000.0f); }
                  if (conf_battcells>=8) { displaydraw->printf(" 8: %5.03f",(float)_m365c->bmsparsed->Cell8Voltage/1000.0f); }
                #endif  
              break;
              case stopsubscreen_assets: //Single/Dual/Same
                drawscreen_header(FPSTR(headline_assests),subscreen,stopsubscreens);
                sprintf(tmp1,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c",_m365c->bmsparsed->serial[0],_m365c->bmsparsed->serial[1],_m365c->bmsparsed->serial[2],_m365c->bmsparsed->serial[3],_m365c->bmsparsed->serial[4],_m365c->bmsparsed->serial[5],_m365c->bmsparsed->serial[6],_m365c->bmsparsed->serial[7],_m365c->bmsparsed->serial[8],_m365c->bmsparsed->serial[9],_m365c->bmsparsed->serial[10],_m365c->bmsparsed->serial[11],_m365c->bmsparsed->serial[12],_m365c->bmsparsed->serial[13]);
                displaydraw->printf(FPSTR(s_bmsfw), _m365c->bmsparsed->fwversion[1],(_m365c->bmsparsed->fwversion[0]&0xf0)>>4,_m365c->bmsparsed->fwversion[0]&0x0f,tmp1);
                sprintf(tmp1,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c",_m365c->escparsed->serial[0],_m365c->escparsed->serial[1],_m365c->escparsed->serial[2],_m365c->escparsed->serial[3],_m365c->escparsed->serial[4],_m365c->escparsed->serial[5],_m365c->escparsed->serial[6],_m365c->escparsed->serial[7],_m365c->escparsed->serial[8],_m365c->escparsed->serial[9],_m365c->escparsed->serial[10],_m365c->escparsed->serial[11],_m365c->escparsed->serial[12],_m365c->escparsed->serial[13]);
                displaydraw->printf(FPSTR(s_escfw), _m365c->escparsed->fwversion[1],(_m365c->escparsed->fwversion[0]&0xf0)>>4,_m365c->escparsed->fwversion[0]&0x0f,tmp1);
                sprintf(tmp1,"%c%c%c%c%c%c",_m365c->escparsed->pin[0],_m365c->escparsed->pin[1],_m365c->escparsed->pin[2],_m365c->escparsed->pin[3],_m365c->escparsed->pin[4],_m365c->escparsed->pin[5]);
                displaydraw->printf(FPSTR(s_pin), tmp1);
                displaydraw->printf(FPSTR(s_miles),(float)_m365c->escparsed->totaldistance/1000.0f);
                displaydraw->printf(FPSTR(s_battdate),((_m365c->bmsparsed->proddate)&0xFE00)>>9,((_m365c->bmsparsed->proddate)&0x1E0)>>5,(_m365c->bmsparsed->proddate)&0x1f);
              break;
              #if !defined useoled2  
                case stopsubscreen_espstate: //single
                    drawscreen_header(FPSTR(headline_espstate),subscreen,stopsubscreens);
                    displaydraw->print(FPSTR(s_firmware)); displaydraw->println(swversion);
                    displaydraw->print(FPSTR(s_wlan));
                    if (wlanstate==wlansearching) { displaydraw->println(FPSTR(s_wlansearch)); }
                    if (wlanstate==wlanconnected) { displaydraw->print(FPSTR(s_wlancon)); displaydraw->println(WiFi.SSID()); displaydraw->println(WiFi.localIP()); }
                    if (wlanstate==wlanap) { displaydraw->print(FPSTR(s_wlanap)); displaydraw->println(WiFi.softAPIP()); displaydraw->println( WiFi.softAPIP());}
                    if (wlanstate==wlanoff) { displaydraw->println(FPSTR(s_off)); }
                    displaydraw->print(FPSTR(s_bleoff));
                  break;
              #endif
              case stopsubscreen_alarms: //single/dual/same
                drawscreen_header(FPSTR(headline_alerts),subscreen,stopsubscreens);
                sprintf(val1buf,"%03d",_m365c->alertcounter_bmstemp);
                sprintf(val2buf,"%03d",_m365c->alertcounter_esctemp);
                sprintf(val3buf,"%03d",_m365c->alertcounter_cellvoltage);
                sprintf(val4buf,"%03d",_m365c->alertcounter_undervoltage);
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
      for (uint16_t curline=configstartindex;curline<=configendindex;curline++) {
        if (curline==subscreen) {
          configcurrentitem=configcurrentmenu+curline;
          displaydraw->setFont(sp_fontdata);
          displaydraw->setCursor(0,baselineoffset + lineitem*(baselineoffset+linespace));
          displaydraw->print(">");
          #if !defined useoled2
            cm_printValue(configcurrentmenu+curline);
          #endif
        } else {
          displaydraw->setFont(sp_fontlabel);
        }
        //#if !defined useoled2
          displaydraw->setCursor(8,baselineoffset + lineitem*(baselineoffset+linespace));
        //#else
        //  displaydraw->setCursor(5,baselineoffset + lineitem*(baselineoffset+linespace));
        //#endif
        cm_printKey(configcurrentmenu+curline);
        lineitem++;
      } //curline loop
    } //screen == screen_configmenu
    if (screen==screen_charging) {
          displaydraw->clearDisplay();
          displaydraw->setFont();
          displaydraw->setCursor(0,0);
          displaydraw->drawFastHLine(0,8,windowsubpos,WHITE);
          #if !defined useoled2
          switch (subscreen) {
            case 1: //Trip Info: Average Speed, Distance, Time, Average Speed 
                drawscreen_header(FPSTR(s_blank),subscreen,2);
          #endif
            displaydraw->setTextSize(1);
            displaydraw->setTextColor(WHITE);
            displaydraw->setFont(&ARIALN9pt7b); 
            displaydraw->setCursor(2,baselineoffset);
            displaydraw->print(FPSTR(s_charging));
            displaydraw->printf(" %2d%%", _m365c->bmsparsed->remainingpercent);
            displaydraw->drawRect(14,23,100,10,WHITE);
            displaydraw->fillRect(14,24,_m365c->bmsparsed->remainingpercent,8,WHITE);
            displaydraw->setFont();
            displaydraw->setCursor(14,39);
            displaydraw->printf("%4.1f V   %4.1f A\r\n", (float)_m365c->bmsparsed->voltage/100.0f,abs((float)_m365c->bmsparsed->current/100.0f));
            displaydraw->setCursor(14,48);
            displaydraw->printf("%4.0f W  %5d mAh",abs(((float)(_m365c->bmsparsed->voltage/100.0f)*(float)_m365c->bmsparsed->current/100.0f)),_m365c->bmsparsed->remainingcapacity);
            displaydraw->setCursor(14,57);
            displaydraw->print(FPSTR(s_charged));
            displaydraw->printf(" %5d mAh",_m365c->bmsparsed->remainingcapacity-_m365c->capacitychargestart);
          #if !defined useoled2
              break;
            case 2: //Cell Infos
                drawscreen_header(FPSTR(headline_charging),subscreen,2);
                if (conf_battcells>=1) { displaydraw->printf("01: %5.03f ",(float)_m365c->bmsparsed->Cell1Voltage/1000.0f); }
                if (conf_battcells>=2) { displaydraw->printf("02: %5.03f  ",(float)_m365c->bmsparsed->Cell2Voltage/1000.0f); }
                if (conf_battcells>=3) { displaydraw->printf("03: %5.03f ",(float)_m365c->bmsparsed->Cell3Voltage/1000.0f); }
                if (conf_battcells>=4) { displaydraw->printf("04: %5.03f  ",(float)_m365c->bmsparsed->Cell4Voltage/1000.0f); }
                if (conf_battcells>=5) { displaydraw->printf("05: %5.03f ",(float)_m365c->bmsparsed->Cell5Voltage/1000.0f); }
                if (conf_battcells>=6) { displaydraw->printf("06: %5.03f  ",(float)_m365c->bmsparsed->Cell6Voltage/1000.0f); }
                if (conf_battcells>=7) { displaydraw->printf("07: %5.03f ",(float)_m365c->bmsparsed->Cell7Voltage/1000.0f); }
                if (conf_battcells>=8) { displaydraw->printf("08: %5.03f  ",(float)_m365c->bmsparsed->Cell8Voltage/1000.0f); }
                if (conf_battcells>=9) { displaydraw->printf("09: %5.03f ",(float)_m365c->bmsparsed->Cell9Voltage/1000.0f); }
                if (conf_battcells>=10) { displaydraw->printf("10: %5.03f  ",(float)_m365c->bmsparsed->Cell10Voltage/1000.0f); }
                if (conf_battcells>=11) { displaydraw->printf("11: %5.03f ",(float)_m365c->bmsparsed->Cell11Voltage/1000.0f); }
                if (conf_battcells>=12) { displaydraw->printf("12: %5.03f  ",(float)_m365c->bmsparsed->Cell12Voltage/1000.0f); }
                if (conf_battcells<11) { displaydraw->printf("L:  %5.03f H:  %5.03f\r\n", (float)(_m365c->lowest)/1000.0f,(float)(_m365c->highest)/1000.0f); }
                displaydraw->printf("T:  %5.02f D:  %5.03f", (float)_m365c->bmsparsed->voltage/100.0f,(float)(_m365c->highest-_m365c->lowest)/1000.0f);
              break;
          }
          #endif
    }
    //if (screen==screen_timeout) {
    if (screen==screen_error) {
        #if !defined(useoled2)
          displaydraw->setFont(&ARIALN9pt7b);
          displaydraw->setCursor(30,20);
          displaydraw->print(FPSTR(error_error));
          displaydraw->setCursor(50,40);
          displaydraw->printf("%02d",_m365c->escparsed->error);
          displaydraw->setFont();
          displaydraw->setCursor(0,56);
          switch (_m365c->escparsed->error) {
            case 10: displaydraw->print(FPSTR(error_10)); break;
            case 11: displaydraw->print(FPSTR(error_11)); break;
            case 12: displaydraw->print(FPSTR(error_12)); break;
            case 13: displaydraw->print(FPSTR(error_13)); break;
            case 14: displaydraw->print(FPSTR(error_14)); break;
            case 15: displaydraw->print(FPSTR(error_15)); break;
            case 18: displaydraw->print(FPSTR(error_18)); break;
            case 21: displaydraw->print(FPSTR(error_21)); break;
            case 22: displaydraw->print(FPSTR(error_22)); break;
            case 23: displaydraw->print(FPSTR(error_23)); break;
            case 24: displaydraw->print(FPSTR(error_24)); break;
            case 26: displaydraw->print(FPSTR(error_26)); break;
            case 27: displaydraw->print(FPSTR(error_27)); break;
            case 28: displaydraw->print(FPSTR(error_28)); break;
            case 29: displaydraw->print(FPSTR(error_29)); break;
            case 31: displaydraw->print(FPSTR(error_31)); break;
            case 35: displaydraw->print(FPSTR(error_35)); break;
            case 36: displaydraw->print(FPSTR(error_36)); break;
            case 39: displaydraw->print(FPSTR(error_39)); break;
            case 40: displaydraw->print(FPSTR(error_40)); break;
            default: displaydraw->print(FPSTR(error_other)); break;
          }
        #else
          displaydraw->setFont(&ARIALN9pt7b);
          displaydraw->setCursor(50,20);
          displaydraw->print(FPSTR(error_error));
          displaydraw->setCursor(70,45);
          displaydraw->printf("%02d",_m365c->escparsed->error);
          displaydraw->drawBitmap(0,0,  scooter, 64,64, 1);
        #endif
    }
    if (screen==screen_timeout) {
      #ifdef useoled2
        drawscreen_startscreen();
      #else
        displaydraw->drawBitmap(0,0,  scooter, 64,64, 1);
        displaydraw->setFont(&ARIALN9pt7b);
        displaydraw->setCursor(74,15);
        displaydraw->print(FPSTR(s_timeout1));
        displaydraw->setCursor(64,35);
        displaydraw->print(FPSTR(s_timeout2));
        displaydraw->setFont();
        if (wlanstate==wlansearching) { displaydraw->setCursor(64,42); displaydraw->print(FPSTR(s_wlan)); displaydraw->setCursor(64,55); displaydraw->print(FPSTR(s_wlansearch)); }
        if (wlanstate==wlanconnected) { displaydraw->setCursor(64,42); displaydraw->print(WiFi.SSID()); displaydraw->setCursor(34,57); displaydraw->print(WiFi.localIP()); }
        if (wlanstate==wlanap) { displaydraw->setCursor(64,42); displaydraw->print(apssid); displaydraw->setCursor(34,57); displaydraw->print( WiFi.softAPIP()); }
        if (wlanstate==wlanoff) { displaydraw->setCursor(64,56); displaydraw->print(FPSTR(s_wlanoff)); }
      #endif
    }
    if (screen==screen_locked) {
      //display1->drawBitmap(0,0,  scooter, 64,64, 1);  
      displaydraw->setFont();
      displaydraw->setCursor(39,31);
      displaydraw->print(FPSTR(s_locked));
    }
    if (screen==screen_alert) {  
      displaydraw->setFont();
      displaydraw->setCursor(39,31);
      displaydraw->print(FPSTR(s_alert));
    }

    if ((screen==screen_stop)&(subscreen==0)) {
      //LIGHT ON/OFF
        if (_m365c->x1parsed->light==0x64) { 
          displaydraw->setFont();
          displaydraw->setCursor(120,line1);
          displaydraw->print("L");
        }
      //NORMAL/ECO MODE
        display1->setFont();
        displaydraw->setCursor(112,line1);
        if (_m365c->x1parsed->mode<2) { 
          displaydraw->print("N"); //normal mode
        } else {
          displaydraw->print("E"); //eco mode
        }
      //WLAN STATUS
        displaydraw->setFont();
        displaydraw->setCursor(96,line1);
        if (wlanstate==wlansearching) { displaydraw->print(FPSTR(s_wlansearchshort)); }
        if (wlanstate==wlanconnected) { displaydraw->print(FPSTR(s_wlanconshort)); }
        if (wlanstate==wlanap) { displaydraw->print(FPSTR(s_wlanapshort)); }
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
    displaydraw->display();
    duration_oled1transfer = micros()-timestamp_oled1transfer;

/*    #ifdef useoled2
      updatescreen2 = true;
    #endif*/
  } //M365Display::oled1_update
#endif //useoled1

#ifdef useoled2
  void M365Display::oled2_init() {
    #ifdef usei2c
      display2->begin(SSD1306_SWITCHCAPVCC, oled2_address, oled_doreset,oled_sda, oled_scl, oled_clock);
    #else
      display2->begin(SSD1306_SWITCHCAPVCC);
    #endif
    display2->setRotation(OLED2_ROTATION);
    display2->setTextColor(WHITE);
    display2->setFont();
    display2->setTextSize(1);
    display2->clearDisplay();
  } //M365Display::oled2_init

  void M365Display::oled2_update() {
    //uint8_t line;
    timestamp_oled2draw=micros();
    if (oledreinit) {
      oled2_init();
      oledreinit=false;
    }
    if (screen!=screen_splash) {
      display2->clearDisplay();
    }
    displaydraw = display2;
    if (screen==screen_drive) {
        displaydraw->setFont(&ARIALNB18pt7b); displaydraw->setCursor(0,31); displaydraw->printf("%3d", int16_t((float)(_m365c->bmsparsed->voltage/100.0f)*(float)_m365c->bmsparsed->current/100.0f));
        displaydraw->setFont(&ARIALN9pt7b); displaydraw->setCursor(50,31); displaydraw->print(FPSTR(unit_power));
        displaydraw->setFont(&ARIALNB18pt7b); displaydraw->setCursor(65,31); displaydraw->printf("%3d",_m365c->bmsparsed->remainingpercent);
        displaydraw->setFont(&ARIALN9pt7b); displaydraw->setCursor(116,31); displaydraw->print(FPSTR(unit_percent));
        displaydraw->setFont(&ARIALNB18pt7b); displaydraw->setCursor(0,63); displaydraw->printf("%2d",uint8_t((float)_m365c->bmsparsed->voltage/100.0f));
        displaydraw->setFont(&ARIALNB9pt7b); displaydraw->setCursor(34,63); displaydraw->printf(".%1d",uint8_t((float)_m365c->bmsparsed->voltage/10.0f) %10);
        displaydraw->setFont(&ARIALN9pt7b); displaydraw->setCursor(50,63); displaydraw->print(FPSTR(unit_volt));
        displaydraw->setFont(&ARIALNB18pt7b); displaydraw->setCursor(65,63); displaydraw->printf("%2d",int16_t((float)_m365c->bmsparsed->current/100.0f));
        displaydraw->setFont(&ARIALNB9pt7b); displaydraw->setCursor(100,63); displaydraw->printf(".%1d",int16_t((float)_m365c->bmsparsed->current/10.0f) %10);
        displaydraw->setFont(&ARIALN9pt7b); displaydraw->setCursor(116,63); displaydraw->print(FPSTR(unit_current));
    }
    if (screen==screen_stop) {
          displaydraw->setFont();
          displaydraw->setCursor(0,0);
          switch (subscreen) {
            case stopsubscreen_trip: //Temperatures - we have 2 Battery Temperatures, but we only display the higher one - both can be seen on detail screens
              sprintf(val1buf,"%4d",_m365c->bmsparsed->health);
              if ((float)_m365c->bmsparsed->temperature[0]>=(float)_m365c->bmsparsed->temperature[1]) {
                  sprintf(val1buf,"%4.1f",(float)_m365c->bmsparsed->temperature[0]-20.0f);  
                } else {
                  sprintf(val1buf,"%4.1f",(float)_m365c->bmsparsed->temperature[1]-20.0f);  
                }
              sprintf(val2buf,"%4.1f",(float)_m365c->escparsed->frametemp2/10.0f);
              sprintf(val3buf,"%4.1f",(float)_m365c->bmsparsed->voltage/100.0f);
              sprintf(val4buf,"%5d",_m365c->bmsparsed->remainingpercent);
              drawscreen_data(false, 4, true,
                FPSTR(label_bmstemp),&val1buf[0],FPSTR(unit_temp),
                FPSTR(label_esctemp),&val2buf[0],FPSTR(unit_temp),
                FPSTR(label_volt),&val3buf[0],FPSTR(unit_volt),
                FPSTR(label_percent),&val4buf[0],FPSTR(unit_percent));
              break;
            case stopsubscreen_minmax:
                displaydraw->printf("FIX THIS SCREEN (%d/%d)\r\n\r\n",subscreen,stopsubscreens);
              break;
            case stopsubscreen_batt1:
                sprintf(val1buf,"%4d",_m365c->bmsparsed->health);
                sprintf(val2buf,"%5d",_m365c->bmsparsed->remainingcapacity);
                sprintf(val3buf,"%5d",_m365c->bmsparsed->totalcapacity);
                sprintf(val4buf,"%02d/%02d",_m365c->bmsparsed->temperature[0]-20,_m365c->bmsparsed->temperature[1]-20);
                drawscreen_data(true, 4, true,
                    FPSTR(label_health),&val1buf[0],FPSTR(unit_percent),
                    FPSTR(label_capacityshort),&val2buf[0],FPSTR(unit_mah),
                    FPSTR(label_totalcapacityshort),&val3buf[0],FPSTR(unit_mah),
                    FPSTR(label_tempshort),&val4buf[0],FPSTR(unit_temp));
              break;
            case stopsubscreen_cells:
                displaydraw->setFont(&ARIALN9pt7b); 
                displaydraw->setCursor(0,12);
                if (conf_battcells>=9) { displaydraw->printf(" 9: %5.03f ",(float)_m365c->bmsparsed->Cell9Voltage/1000.0f); }
                if (conf_battcells>=10) { displaydraw->printf("10: %5.03f",(float)_m365c->bmsparsed->Cell10Voltage/1000.0f); }
                displaydraw->setCursor(0,12+14);
                if (conf_battcells>=11) { displaydraw->printf("11: %5.03f ",(float)_m365c->bmsparsed->Cell11Voltage/1000.0f); }
                if (conf_battcells>=12) { displaydraw->printf("12: %5.03f",(float)_m365c->bmsparsed->Cell12Voltage/1000.0f); }
                displaydraw->setCursor(0,12+14+14);
                displaydraw->printf("Lo: %5.03f Hi: %5.03f", (float)(_m365c->lowest)/1000.0f,(float)(_m365c->highest)/1000.0f);
                displaydraw->setCursor(0,12+14+14+14);
                displaydraw->printf("T : %5.02f D : %5.03f", (float)_m365c->bmsparsed->voltage/100.0f,(float)(_m365c->highest-_m365c->lowest)/1000.0f);
              break;
            case stopsubscreen_assets:
                displaydraw->println(FPSTR(headline_espstate));
                displaydraw->print(FPSTR(s_firmware)); 
                displaydraw->println(swversion);
                displaydraw->print(FPSTR(s_wlan));
                if (wlanstate==wlansearching) { displaydraw->println(FPSTR(s_wlansearch)); }
                if (wlanstate==wlanconnected) { displaydraw->print(FPSTR(s_wlancon)); displaydraw->println(WiFi.SSID()); displaydraw->println(WiFi.localIP()); }
                if (wlanstate==wlanap) { displaydraw->print(FPSTR(s_wlanap)); displaydraw->println(WiFi.softAPIP()); displaydraw->println( WiFi.softAPIP()); }
                if (wlanstate==wlanoff) { displaydraw->println(FPSTR(s_off)); }
                displaydraw->print(FPSTR(s_bleoff));  
              break;
            case stopsubscreen_alarms:
                drawscreen_header(FPSTR(headline_alerts),subscreen,stopsubscreens);
                sprintf(val1buf,"%03d",_m365c->alertcounter_escerror);
                sprintf(val1buf,"%03d",_m365c->alertcounter_lockedalarm);
                drawscreen_data(true, 2, false,
                    FPSTR(label_escerrorcounter),&val1buf[0],FPSTR(unit_percent),
                    FPSTR(label_alertcounter),&val2buf[0],FPSTR(unit_percent),
                    NULL,NULL,NULL,
                    NULL,NULL,NULL);
              break;
            default:
                displaydraw->setFont();
                displaydraw->setCursor(0,0);
                displaydraw->print("OLED2 STOP DEFAULT");
              break;
          }
    }
    if (screen == screen_configmenu) {
        displaydraw->setFont(sp_fontdata);
        cm_printValue(configcurrentitem);
    } //screen == screen_configmenu
    if (screen==screen_charging) {
                displaydraw->setFont();
                displaydraw->setCursor(0,0);
                if (conf_battcells>=1) { displaydraw->printf("01: %5.03f ",(float)_m365c->bmsparsed->Cell1Voltage/1000.0f); }
                if (conf_battcells>=2) { displaydraw->printf("02: %5.03f  ",(float)_m365c->bmsparsed->Cell2Voltage/1000.0f); }
                if (conf_battcells>=3) { displaydraw->printf("03: %5.03f ",(float)_m365c->bmsparsed->Cell3Voltage/1000.0f); }
                if (conf_battcells>=4) { displaydraw->printf("04: %5.03f  ",(float)_m365c->bmsparsed->Cell4Voltage/1000.0f); }
                if (conf_battcells>=5) { displaydraw->printf("05: %5.03f ",(float)_m365c->bmsparsed->Cell5Voltage/1000.0f); }
                if (conf_battcells>=6) { displaydraw->printf("06: %5.03f  ",(float)_m365c->bmsparsed->Cell6Voltage/1000.0f); }
                if (conf_battcells>=7) { displaydraw->printf("07: %5.03f ",(float)_m365c->bmsparsed->Cell7Voltage/1000.0f); }
                if (conf_battcells>=8) { displaydraw->printf("08: %5.03f  ",(float)_m365c->bmsparsed->Cell8Voltage/1000.0f); }
                if (conf_battcells>=9) { displaydraw->printf("09: %5.03f ",(float)_m365c->bmsparsed->Cell9Voltage/1000.0f); }
                if (conf_battcells>=10) { displaydraw->printf("10: %5.03f  ",(float)_m365c->bmsparsed->Cell10Voltage/1000.0f); }
                if (conf_battcells>=11) { displaydraw->printf("11: %5.03f ",(float)_m365c->bmsparsed->Cell11Voltage/1000.0f); }
                if (conf_battcells>=12) { displaydraw->printf("12: %5.03f  ",(float)_m365c->bmsparsed->Cell12Voltage/1000.0f); }
                displaydraw->printf("L:  %5.03f H:  %5.03f\r\n", (float)(_m365c->lowest)/1000.0f,(float)(_m365c->highest)/1000.0f);
                displaydraw->printf("T:  %5.02f D:  %5.03f", (float)_m365c->bmsparsed->voltage/100.0f,(float)(_m365c->highest-_m365c->lowest)/1000.0f);
    }
    if (screen==screen_error) {
      displaydraw->setFont(&ARIALN9pt7b);
      displaydraw->setCursor(0,20);
      switch (_m365c->escparsed->error) {
            case 10: displaydraw->print(FPSTR(error_10)); break;
            case 11: displaydraw->print(FPSTR(error_11)); break;
            case 12: displaydraw->print(FPSTR(error_12)); break;
            case 13: displaydraw->print(FPSTR(error_13)); break;
            case 14: displaydraw->print(FPSTR(error_14)); break;
            case 15: displaydraw->print(FPSTR(error_15)); break;
            case 18: displaydraw->print(FPSTR(error_18)); break;
            case 21: displaydraw->print(FPSTR(error_21)); break;
            case 22: displaydraw->print(FPSTR(error_22)); break;
            case 23: displaydraw->print(FPSTR(error_23)); break;
            case 24: displaydraw->print(FPSTR(error_24)); break;
            case 26: displaydraw->print(FPSTR(error_26)); break;
            case 27: displaydraw->print(FPSTR(error_27)); break;
            case 28: displaydraw->print(FPSTR(error_28)); break;
            case 29: displaydraw->print(FPSTR(error_29)); break;
            case 31: displaydraw->print(FPSTR(error_31)); break;
            case 35: displaydraw->print(FPSTR(error_35)); break;
            case 36: displaydraw->print(FPSTR(error_36)); break;
            case 39: displaydraw->print(FPSTR(error_39)); break;
            case 40: displaydraw->print(FPSTR(error_40)); break;
            default: displaydraw->print(FPSTR(error_other)); break;
      }
    }
    if (screen==screen_timeout) {  
      displaydraw->setFont(&ARIALN9pt7b);
      displaydraw->setCursor(40,20);
      displaydraw->print(FPSTR(s_timeout2));
      displaydraw->setCursor(20,40);
      displaydraw->print(FPSTR(s_timeout3));
      displaydraw->setFont();
      if (wlanstate==wlansearching) { displaydraw->setCursor(0,48); displaydraw->print(FPSTR(s_wlan)); displaydraw->print(FPSTR(s_wlansearch)); }
      if (wlanstate==wlanconnected) { displaydraw->setCursor(0,48); displaydraw->print("SSID: "); displaydraw->print(WiFi.SSID()); displaydraw->setCursor(0,57); displaydraw->print("IP: "); displaydraw->print(WiFi.localIP()); }
      if (wlanstate==wlanap) { displaydraw->setCursor(0,48); displaydraw->print("SSID: ");  displaydraw->print(apssid); displaydraw->setCursor(0,57); displaydraw->print("IP: "); displaydraw->print( WiFi.softAPIP()); }
      if (wlanstate==wlanoff) { displaydraw->setCursor(40,57); displaydraw->print(FPSTR(s_wlanoff)); }

    }
    if (screen==screen_locked) {  
      displaydraw->setFont();
      displaydraw->setCursor(39,31);
      displaydraw->print(FPSTR(s_locked));
    }
    if (screen==screen_alert) {  
      displaydraw->setFont();
      displaydraw->setCursor(39,31);
      displaydraw->print(FPSTR(s_alert));
    }
    if (showdialog) {
        drawscreen_dialog();
    }
    duration_oled2draw = micros()-timestamp_oled2draw;
    timestamp_oled2transfer=micros();
    displaydraw->display();
    duration_oled2transfer = micros()-timestamp_oled2transfer;
  } //M365Display::oled2_update
#endif //useoled2


//display task and headunit task
  TaskHandle_t dTaskHandle;
  uint32_t display_timestamp_start=0;
  uint16_t display_time_duration=0;
  uint16_t display_time_idle=0;
  uint32_t display_iterations=0;

//debugdisplay for the headunit
#ifdef hudisplaydebug
  boolean doupdatehudisplay = false;

  void hudisplay()
    {
      ptrm365d->display1->clearDisplay();
      ptrm365d->display1->setFont();
      ptrm365d->display1->setTextSize(1);
      ptrm365d->display1->setTextColor(WHITE);
      ptrm365d->display1->setCursor(0,0);
      ptrm365d->display1->printf("T:%03d[%03d] B:%03d[%03d]\r\n", ptrm365bus->value_throttle,ptrm365bus->adc_throttle,ptrm365bus->value_brake, ptrm365bus->adc_brake);
      //display1->printf("Brake:    %03d (%03d)\r\n", value_brake, adc_brake);
      ptrm365d->display1->printf("escpackets: %d\r\n", ptrm365bus->escpacketcounter);
      ptrm365d->display1->printf("c: %02d/%02d d: %02d/%02d\r\n", ptrm365bus->controller_time_duration,ptrm365bus->controller_time_idle,display_time_duration,display_time_idle);
//      display1->printf("\r\n", display_time_duration,display_time_idle);
      ptrm365d->display1->printf("s:%d/%d > %d/%d\r\n", ptrm365bus->sendqueuein_ok,ptrm365bus->sendqueuein_fail,ptrm365bus->sendqueueout_ok,ptrm365bus->sendqueueout_fail);
      ptrm365d->display1->printf("r:%d/%d > %d/%d\r\n", ptrm365bus->recqueuein_ok,ptrm365bus->recqueuein_fail,ptrm365bus->recqueueout_ok,ptrm365bus->recqueueout_fail);
      ptrm365d->display1->printf("mrs: %d\r\n", ptrm365bus->maxpacketsizereceived);
      if (wlanstate==wlansearching) { ptrm365d->display1->print(FPSTR(s_wlan)); ptrm365d->display1->print(FPSTR(s_wlansearch)); }
      if (wlanstate==wlanconnected) { ptrm365d->display1->print(WiFi.SSID()); ptrm365d->display1->print(" "); ptrm365d->display1->print(WiFi.localIP()); }
      if (wlanstate==wlanap) { ptrm365d->display1->print(WiFi.softAPIP()); }
      if (wlanstate==wlanoff) { ptrm365d->display1->print(FPSTR(s_wlanoff)); }
      ptrm365d->display1->display();  
    } //hudisplay
#endif
#ifdef usedisplay  
  void displaytask( void * pvParameters ) {
    DebugSerial.printf("DisplayTask started on core %d, prio %d\r\n", xPortGetCoreID(), uxTaskPriorityGet(NULL));
    while(true) {
      display_time_idle = millis()-display_timestamp_start;
      display_timestamp_start = millis();
      display_iterations++;
      #ifdef hudisplaydebug
        if (doupdatehudisplay) {
            hudisplay();
            doupdatehudisplay = false;
          }
      #else
        ptrm365d->loop();
      #endif
      display_time_duration= millis()-display_timestamp_start;
      vTaskDelay(1); //10ms -> 100 Iterations per second (display-framerate is still defined via ) display updates per second are enough
    } //while
  } //displaytask

  void displaytask_start() {
    xTaskCreatePinnedToCore(displaytask, "DisplayTask", 10000, NULL, prio_display, &dTaskHandle, core_display);
    #ifdef hudisplaydebug
      doupdatehudisplay = true;
    #endif
  } //displaytask_start
#endif