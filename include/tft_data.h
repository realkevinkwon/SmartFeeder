
#ifndef TFT_DATA_H
#define TFT_DATA_H

#if	defined (__AVR__)
	#include <avr/pgmspace.h>
#else
	#if !defined(PROGMEM)
		#define PROGMEM
	#endif
#endif

extern const uint8_t logo[239] PROGMEM;
extern const uint8_t pic[3844] PROGMEM;
extern const uint8_t flash[12753] PROGMEM;
extern const uint8_t pic_wifi_32[30000] PROGMEM;
extern const uint8_t pic_wifi_48[30000] PROGMEM;
extern const uint8_t pic_wifi_64[30000] PROGMEM;

#endif /* TFT_DATA_H */