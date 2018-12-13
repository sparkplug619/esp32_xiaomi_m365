#ifndef DISPLAY_h
#define DISPLAY_h

    #include "definitions.h"
    
    #ifdef headunit
      #include "m365headunit.h"
    #else
      #include "m365client.h"
    #endif

    #include <Adafruit_SSD1306.h>
    #include <Adafruit_GFX.h>
    #include <ARIALNB9pt7b.h> //Arial Narrow Bold Size 9 "33" = 15x13px
    #include <ARIALN9pt7b.h> //Arial Narrow Size 9 "33" = 15x13px
    #include <ariblk42pt7b.h> //Arial Black Size 42 "33" = 101x61px
    #include <ARIALNB18pt7b.h> //Arial Narrow Bold Size 18 "33" = 30x25px

  class M365Display {
    public:
      M365Display(void); //constructor
      
      //i2c variants:
        #if (defined usei2c && defined useoled1)
            ///Adafruit_SSD1306 display1(oled_reset);
            Adafruit_SSD1306 *display1;
            Adafruit_SSD1306 *displaydraw;
        #endif
        #if (defined usei2c && defined useoled2)
            //Adafruit_SSD1306 display2(oled_reset);
            Adafruit_SSD1306 *display2;
        #endif

      //spi variants
        #if (!defined usei2c && defined useoled1)
            //software spi Adafruit_SSD1306 display1(OLED_MOSI, OLED_CLK, OLED_DC, OLED1_RESET, OLED1_CS);
            //hardware spi with patched Adafruit_SSD1306 Library:
            //Adafruit_SSD1306 display1(OLED_MISO, OLED_MOSI, OLED_CLK, OLED_DC, OLED1_RESET, OLED1_CS, 4000000UL);
            Adafruit_SSD1306 *display1;
            Adafruit_SSD1306 *displaydraw;
        #endif
        #if (!defined usei2c && defined useoled2)
            //software spi Adafruit_SSD1306 display2(OLED_MOSI, OLED_CLK, OLED_DC, OLED1_RESET, OLED2_CS);
            //hardware spi
            //Adafruit_SSD1306 display2(OLED_MISO, OLED_MOSI, OLED_CLK, OLED_DC, OLED2_RESET, OLED2_CS, 4000000UL);
            Adafruit_SSD1306 *display2;
        #endif

      //main methods
        void begin(M365Client *mc);
        void loop(void);

      //display/screen "templates"
        void drawscreen_startscreen(void);
        void drawscreen_header(const char *h, uint8_t scrnum, uint8_t scrtotal);
        void drawscreen_data(bool headline, uint8_t lines, bool showunits,
                      const char *l1, char *v1, const char *u1,
                      const char *l2, char* v2, const char *u2,
                      const char *l3, char* v3, const char *u3,
                      const char *l4, char* v4, const char *u4);
        void infopopup (char *_popuptitle, char *_popuptext, uint16_t duration);
        void alertpopup (char *_popuptitle, char *_popuptext, uint16_t duration);
        void drawscreen_popup(void);
        void dialog_edit_int8(const char *_dialogtitle, uint8_t *_value, uint8_t _minvalue, uint8_t _maxvalue);
        void drawscreen_dialog(void);

      //configmenu methods
        void cm_handleactions(void);
        void cm_printKey(uint16_t entryid);
        void cm_printValue(uint16_t entryid);

      //screenswitching/drawing methods
        void oled_switchscreens(void);
        void oled1_init(void);
        void oled1_update(void);

        #ifdef useoled2
          void oled2_init(void);
          void oled2_update(void);
        #endif

    protected:
      M365Client *_m365c;

      #define line1 0
      #define line2 8
      #define line3 16
      #define line4 24
      #define line5 32
      #define line6 40
      #define line7 48
      #define line8 56
      boolean updatescreen = false;
      //boolean updatescreen2 = false; //workaround for i2c/uart timing for 2 displays
      #define oledrefreshanyscreen 100 //refresh oled screen every xx ms (if there is new data or user interaction)
      unsigned long olednextrefreshtimestamp = 0;
      unsigned long timeout_oled = 0;
      //boolean oled_blink=true;
      #define oledwidth 128
      #define oledheight 64
      #define baselineoffset 13
      #define linespace 1
      #define dataoffset 8
      
      boolean oledreinit = false;
      #define oledreinitduration 30000
      unsigned long oledreinittimestamp = 0;

      unsigned long popuptimestamp = 0;
      boolean showpopup = false;
      #define popupalertduration 5000

      boolean showdialog = false;
      char diag_title[30];
      uint8_t *diag_value;
      uint8_t diag_min;
      uint8_t diag_max;


      //buffers for drawscreen methods
        #define dsvalbuflen 10
        char val1buf[dsvalbuflen];
        char val2buf[dsvalbuflen];
        char val3buf[dsvalbuflen];
        char val4buf[dsvalbuflen];
        char popuptitle[30];
        char popuptext[100];

      //offsets & font definitions
        #define sp_lx 0
        #define sp_fontlabel &ARIALN9pt7b
        #define sp_dx 72 //64 for 4 ltter units, 72 for 3 letter units
        #define sp_dxnu 80
        #define sp_fontdata &ARIALNB9pt7b
        #define sp_ux 110 //104 for 4 letters, 112 for 3
        #define sp_fontunit

        #define menu_x 1
        #define menu_fontlabel &ARIALN9pt7b
        #define menu_fontlabelselected &ARIALNB9pt7b
        #define menu_fontdata &ARIALNB9pt7b

        #define popup_fontheader &ARIALNB9pt7b
        #define popup_header_x 13
        #define popup_header_y 22
        #define popup_fonttext &ARIALN9pt7b
        #define popup_text_x 13
        #define popup_text_y 42

      //Screen Switching
        #define screen_stop 0
        #define screen_drive 1
        #define screen_error 2
        #define screen_timeout 3
        #define screen_charging 4
        #define screen_configmenu 5
        #define screen_alarm 6 //error counters & state
        #define screen_locked 7
        #define screen_alert 8 //alert screen when scooter is locked and moved
        #define screen_splash 9 //splashscreen on startup
        uint8_t screen = screen_splash;

        uint8_t subscreen = 0;
        uint8_t windowsubpos=0;

        #if !defined useoled2 //Single Screen Stopscreens
          #define stopsubscreens 8
          #define stopsubscreen_trip 0 //single/dual/same
          #define stopsubscreen_temp 1 //single
          #define stopsubscreen_minmax 2 //single/dual/same
          #define stopsubscreen_batt1 3 //single/dual/same
          #define stopsubscreen_batt2 4 //single
          #define stopsubscreen_cells 5 //single/dual/different
          #define stopsubscreen_assets 6 //single/dual/same
          #define stopsubscreen_espstate 7 //single
          #define stopsubscreen_alarms 8 //single/dual/same
        #else //Dual Screen Stopscreens
          #define stopsubscreens 5
          #define stopsubscreen_trip 0
          #define stopsubscreen_minmax 1 //single/dual/same
          #define stopsubscreen_batt1 2 //single/dual/same
          #define stopsubscreen_cells 3
          #define stopsubscreen_assets 4 //single/dual/same
          #define stopsubscreen_alarms 5 //single/dual/same
        #endif
        #define chargesubscreens 2
        
      //throttle value learning/"window" definitions
        #define throttlemindefault 40
        #define throttlemaxdefault 190
        uint8_t throttlemin = throttlemindefault;
        uint8_t throttlemax = throttlemaxdefault;
        uint8_t stopwindowsize = (uint8_t)((throttlemax-throttlemin)/stopsubscreens);
        uint8_t chargewindowsize = (uint8_t)((throttlemax-throttlemin)/chargesubscreens);

      //config menu
        //cmm -> Configmenu "main" prefixes
          #define cmm_root 0x0000
          #define cmm_scooter 0x0100
          #define cmm_alerts 0x0200
          #define cmm_esp 0x0300
          #define nummenus 4
        //menu itams
          //root
            #define cms_smscooter cmm_root+0
            #define cms_smalerts cmm_root+1
            #define cms_smesp cmm_root+2
            #define cms_changelock cmm_root+3 //WORKING
            #define cms_turnoff cmm_root+4 //WORKING
            #define cms_exitroot cmm_root+5 //exitmenu //WORKING
          //submenu scooter
            #define cms_light cmm_scooter+0 //tail ligth on/off //WORKING
            #define cms_cruise cmm_scooter+1 //cruise mode on/off //WORKING
            #define cms_kers cmm_scooter+2 //set kers //WORKING
            #define cms_ws cmm_scooter+3 //set wheelsize //WORKING
            #define cms_unit cmm_scooter+4 //kilometers or miles?
            #define cms_bc cmm_scooter+5 //set number of cells (10/12s) //WORKING
            #define cms_exitscooter cmm_scooter+6 //exitmenu //WORKING
          //submenu alerts
            #define cms_buv cmm_alerts+0 //set Battery undervoltage alarm
            #define cms_bac cmm_alerts+1 //set Battery Alert CellVoltage Difference //WORKING
            #define cms_bat cmm_alerts+2 //set Battery Alert Temperature //NOT IMPLEMENTED
            #define cms_eat cmm_alerts+3 //set ESC Alert Temperature //NOT IMPLEMENTED
            #define cms_beeponalert cmm_alerts+4
            #define cms_exitalerts cmm_alerts+5 //exitmenu //WORKING
          //submenu esp
            #define cms_busmode cmm_esp+0 //busmode active/passive (request data from m365 or not...?) //WORKING
            #define cms_wifirestart cmm_esp+1 //restart wifi //WORKING
            #define cms_wifionstartup cmm_esp+2 //restart wifi //WORKING
            #define cms_exitesp cmm_esp+3 //exitmenu //WORKING

        uint8_t menuitemcount[nummenus] = { 6,7,6,4 };
        uint16_t configcurrentmenu = cmm_root;
        uint16_t configcurrentitem = 0;
        uint8_t configstartindex = 0;
        uint8_t configendindex = 0;
        bool configchanged = false;

        uint8_t configwindowsize = (uint8_t)((throttlemax-throttlemin)/(menuitemcount[configcurrentmenu>>8]-1));
        #if !defined useoled2
          #define configlinesabove 1
          #define confignumlines 3
        #else
          #define configlinesabove 1
          #define confignumlines 4
        #endif

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
        #define buttonthrottlepressed2 60 //treshold for "button pressed"
        #define buttonthrottleshortpressedduration 50 //millis needed for short press
        #define buttonthrottlelongpressedduration 500 //millis needed for long press
        unsigned long buttonthrottlepressedtimestamp = 0;

        uint8_t bothbuttonstate = 0;  //1 = short, 2 = long press
        boolean bothbuttonpressed = false; //helper for key-detection
        #define buttonbothshortpressedduration 50 //millis needed for short press
        #define buttonbothlongpressedduration 500 //millis needed for long press
        unsigned long buttonbothpressedtimestamp = 0;
  }; //M365Display
  
  static const uint8_t unit_ticks[] PROGMEM = "ticks";
  #define core_display 1
  #define prio_display 1

  extern TaskHandle_t dTaskHandle;
  void displaytask( void * pvParameters );
  void displaytask_start(void);
  #ifdef hudisplaydebug
    extern boolean doupdatehudisplay;
    void hudisplay();
  #endif

  static const unsigned char PROGMEM scooter [] PROGMEM = {
    0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x30, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9c, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x76, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x01, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x0a, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x13, 0x80, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x30, 0xe0, 
    0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x70, 0x30, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x98, 0x78, 
    0x00, 0x22, 0x80, 0x00, 0x00, 0x01, 0x27, 0x18, 0x00, 0x20, 0x40, 0x00, 0x00, 0x1e, 0x59, 0xd8, 
    0x00, 0x21, 0x20, 0x00, 0x07, 0xc0, 0xb6, 0x10, 0x00, 0x66, 0x90, 0x01, 0xf0, 0x09, 0xbd, 0x30, 
    0x01, 0xc5, 0x08, 0x7c, 0x00, 0x06, 0xd3, 0xb0, 0x02, 0x02, 0x87, 0x00, 0x00, 0x03, 0x79, 0x90, 
    0x04, 0x61, 0x40, 0x00, 0x00, 0xec, 0x2f, 0xf0, 0x09, 0x20, 0x80, 0x00, 0x32, 0x21, 0x0a, 0xa0, 
    0x08, 0xc5, 0x8c, 0x04, 0x80, 0xfc, 0x90, 0x40, 0x10, 0x4c, 0x73, 0x60, 0x3f, 0x50, 0x61, 0x80, 
    0x10, 0x0a, 0x73, 0x0f, 0xc0, 0x50, 0x1e, 0x00, 0x13, 0x62, 0xcf, 0xf0, 0x00, 0x50, 0x00, 0x00, 
    0x14, 0x13, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x11, 0x2d, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 
    0x10, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xa6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

#endif