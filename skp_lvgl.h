

#ifndef skp_lvgl_h
#define skp_lvgl_h

// Make this 1 to enable a debug display on the screen, or not 1 to
// disable.
#define SHOWDEBUG 0

// When this is 1, touching the screen will toggle display in roman
// numerals.
#define ENABLEROMAN 1

// For fun, should implement this flag as a live thing, like the roman
// numerals. For now, just a compile-time thing.
#define ENGLISHUNITS 1

#ifdef __cplusplus
extern "C" {
#endif
void skp_lvgl_init(void);



#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
