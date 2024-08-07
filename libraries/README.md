Libraries used here:

These are installed via the Arduino library manager. The FT6206 seems
to be the root and the rest are its dependencies. 

```
 Adafruit_FT6206_Library 
 Adafruit_BusIO
 Adafruit_GFX_Library
 Adafruit_ILI9341
 Adafruit_SH110X
 Adafruit_STMPE610
 Adafruit_TSC2007
 Adafruit_TouchScreen
```

These have to be installed "by hand". Find them here: 
https://github.com/ttlappalainen

```
 NMEA2000
 NMEA2000_Teensyx
```
The LVGL library can be installed by the Arduino library manager, but
 it requires an extra step to copy the `lv_conf.h` file up to sit next
 to the directory name.
 ```
 lv_conf.h
 lvgl
``` 
Note that as of 6 Jul 2024, the version of LVGL in use is 7.11 and
note also that this appears to be important.

You also need to edit a few lines of the `lv_conf.h` file so they read like this:
```
#if 1 /*Set it to "1" to enable content*/
...
#define LV_COLOR_DEPTH     16
...
#define LV_FONT_MONTSERRAT_8     0
#define LV_FONT_MONTSERRAT_10    1
#define LV_FONT_MONTSERRAT_12    1
#define LV_FONT_MONTSERRAT_14    1
#define LV_FONT_MONTSERRAT_16    1
#define LV_FONT_MONTSERRAT_18    1
#define LV_FONT_MONTSERRAT_20    0
#define LV_FONT_MONTSERRAT_22    0
#define LV_FONT_MONTSERRAT_24    0
#define LV_FONT_MONTSERRAT_26    0
#define LV_FONT_MONTSERRAT_28    0
#define LV_FONT_MONTSERRAT_30    1
#define LV_FONT_MONTSERRAT_32    1
#define LV_FONT_MONTSERRAT_34    0
#define LV_FONT_MONTSERRAT_36    0
#define LV_FONT_MONTSERRAT_38    0
#define LV_FONT_MONTSERRAT_40    1
#define LV_FONT_MONTSERRAT_42    0
#define LV_FONT_MONTSERRAT_44    0
#define LV_FONT_MONTSERRAT_46    0
#define LV_FONT_MONTSERRAT_48    1
```
This might be a little overkill on the fonts, feel free to audit.


