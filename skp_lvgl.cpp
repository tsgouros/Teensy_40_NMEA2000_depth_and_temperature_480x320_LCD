

#include <Arduino.h>
#include "skp_lvgl.h"
#include <lvgl.h>
#include <Wire.h>
#include <ILI9488_t3.h>
#include <SPI.h>
#include <Adafruit_FT6206.h>

LV_IMG_DECLARE(needle2);
LV_IMG_DECLARE(depthScreen);

int LCD_BL = 14;
int LCD_RST = 15;

#define LVGL_TICK_PERIOD 60

#define TFT_CS 10
#define TFT_DC  9

ILI9488_t3 display = ILI9488_t3(&SPI, TFT_CS, TFT_DC);
Adafruit_FT6206 touchScreen = Adafruit_FT6206();

lv_obj_t * label_water_depth; 
lv_obj_t * gauge1;
lv_obj_t * label_temperature;
int oldTouchX = 0;
int oldTouchY = 0;
lv_indev_t * touchDevice;

int screenWidth = 480;
int screenHeight = 320;
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * LV_VER_RES_MAX / 10];   

IntervalTimer tick;
lv_obj_t * lvneedle;

static void lv_tick_handler(void)
{

  lv_tick_inc(LVGL_TICK_PERIOD);
}


/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{ 
   uint16_t width = (area->x2 - area->x1 +1);
   uint16_t height = (area->y2 - area->y1+1);

   display.writeRect(area->x1, area->y1, width, height, (uint16_t *)color_p);

   lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
  
}


bool my_touchpad_read(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
    uint16_t touchX, touchY;
    Serial.println("in my_touchpad_read()");
  
    // If there's no data, get out and go home.
    if (!data) {
      Serial.println("null data pointer");
      return 0;
    }

    // Check if touchpad has been touched.
    if (touchScreen.touched()) {   
      // Retrieve a point  
      TS_Point p = touchScreen.getPoint(); 
    
        touchX = p.y;         // Rotate the co-ordinates
        touchY = p.x;
        touchY = 320-touchY;
  
         if ((touchX != oldTouchX) || (touchY != oldTouchY))
         {
              Serial.print("x = ");
              Serial.print(touchX, DEC);
              Serial.print("y = ");
              Serial.println(touchY, DEC);
              
              oldTouchY = touchY;
              oldTouchX = touchX;
              data->state = LV_INDEV_STATE_PR; 
              data->point.x = touchX;
              data->point.y = touchY;
         
         }
    } else {
        data->point.x = oldTouchX;
        data->point.y = oldTouchY;
        data->state =LV_INDEV_STATE_REL;
     }

    Serial.println("at end of my_touchpad_read()");
           
    return 0;
}

uint16_t vres = 320;
uint16_t hres = 320;
uint16_t pos_x = 0;
uint16_t pos_y = 0;

void skp_lvgl_init(void)
{

    pinMode(LCD_BL,OUTPUT);
    digitalWrite(LCD_BL, HIGH); 

    display.begin();
    display.fillScreen(ILI9488_BLUE);
    display.setRotation(1); 

    lv_init();
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX / 10);
  
   /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t indev_drv;
    // Descriptor of a input device driver
    lv_indev_drv_init(&indev_drv);
    // Specify that a touch screen is a pointer-like device
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    // Set the driver function.
    indev_drv.read_cb = my_touchpad_read;
    // Finally register the driver
    touchDevice = lv_indev_drv_register(&indev_drv);


    lv_obj_set_style_local_bg_color (lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_bg_opa( lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);  

    lv_obj_t *central = lv_page_create(lv_scr_act(), NULL);
    lv_obj_set_size(central, vres , hres);
    lv_obj_set_pos(central,  0, 0);
    lv_page_set_scrlbar_mode(central, LV_SCRLBAR_MODE_OFF);


    lv_obj_t * img = lv_img_create(central,NULL);
    lv_img_set_src(img, &depthScreen);
    lv_obj_set_size(img, 320, 320);
    lv_obj_set_pos(img,pos_x-14,pos_y-14);
  
    lvneedle = lv_img_create(lv_scr_act(),NULL);
    lv_img_set_src(lvneedle, &needle2);
    lv_obj_set_pos(lvneedle,60,142);

    lv_img_set_angle(lvneedle, 900); // Units of .1 degree

    static lv_style_t style_text;
    lv_style_init(&style_text);
    lv_style_set_text_color(&style_text, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&style_text, LV_STATE_DEFAULT, &lv_font_montserrat_16);

    lv_obj_t * label_text_wd = lv_label_create(lv_disp_get_scr_act(NULL), NULL);
    lv_obj_set_pos(label_text_wd, 330, 10);
    lv_obj_add_style(label_text_wd, LV_OBJ_PART_MAIN,&style_text);
    lv_label_set_text(label_text_wd, "Water Depth");
  
    lv_obj_t * label_text_lb = lv_label_create(lv_disp_get_scr_act(NULL), NULL);
    lv_obj_set_pos(label_text_lb, 435, 77);
    lv_obj_add_style(label_text_lb, LV_OBJ_PART_MAIN,&style_text);
    lv_label_set_text(label_text_lb, "Ft");

    lv_obj_t * label_text_temp = lv_label_create(lv_disp_get_scr_act(NULL), NULL);
    lv_obj_set_pos(label_text_temp, 330, 120);
    lv_obj_add_style(label_text_temp, LV_OBJ_PART_MAIN,&style_text);
    lv_label_set_text(label_text_temp, "Temperature");

    lv_obj_t * label_text_degree = lv_label_create(lv_disp_get_scr_act(NULL), NULL);
    lv_obj_set_pos(label_text_degree, 450, 166);
    lv_obj_add_style(label_text_degree, LV_OBJ_PART_MAIN,&style_text);
    //lv_label_set_text(label_text_degree, "°C");
    lv_label_set_text(label_text_degree, "°F");

    static lv_style_t style_mps;
    lv_style_init(&style_mps);
    lv_style_set_text_color(&style_mps, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&style_mps, LV_STATE_DEFAULT, &lv_font_montserrat_48);

    label_water_depth = lv_label_create(lv_disp_get_scr_act(NULL), NULL);
    lv_label_set_long_mode(label_water_depth, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label_water_depth, 150);
    lv_label_set_align(label_water_depth, LV_LABEL_ALIGN_RIGHT); 
    lv_obj_set_pos(label_water_depth, 320, 30);
    lv_obj_add_style(label_water_depth, LV_OBJ_PART_MAIN, &style_mps);
    lv_label_set_text(label_water_depth, "0.00");

    label_temperature = lv_label_create(lv_disp_get_scr_act(NULL), NULL);
    lv_label_set_long_mode(label_temperature, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label_temperature, 150);
    lv_label_set_align(label_temperature, LV_LABEL_ALIGN_RIGHT); 
    lv_obj_set_pos(label_temperature, 290, 140);
    lv_obj_add_style(label_temperature, LV_OBJ_PART_MAIN, &style_mps);
    lv_label_set_text(label_temperature, "-.-");

    Serial.println("tick.begin");
    tick.begin(lv_tick_handler, LVGL_TICK_PERIOD * 1000);  // Start ticker

    if (touchScreen.begin(40)) { 
      Serial.println("Touchscreen started."); 
    } else { 
      Serial.println("Unable to start touchscreen.");
    }
}
