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

The LVGL library can be installed by the Arduino library manager, but
 it requires an extra step to copy the `lv_conf.h` file up to sit next
 to the directory name.
 ```
 lv_conf.h
 lvgl
``` 

These have to be installed "by hand". Find them here: 
https://github.com/ttlappalainen

```
 NMEA2000
 NMEA2000_Teensyx
```

