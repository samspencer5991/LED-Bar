#ifndef INTERFACE_H
#define INTERFACE_H

#include <stdint.h>
#include <FastLED.h>
#include <Arduino.h>

extern CRGB leds[];

typedef enum
{
    PotNormal,      // Pot is normalised to 0-1023
    PotCentred,     // Pot is centred around 0 with +- 512 values either side
    PotButtonOnly   // Pot is a button only
} PotMode;

typedef enum
{
    PotLedFill,
    PotLedDot,
    PotLedFillGradient,
    PotLedDotGradient
} PotLedMode;

typedef enum
{
    PotNoWrap,
    PotWrap
} PotWrapStyle;

typedef struct
{
    PotMode mode;                                       // Control UI mode
    PotWrapStyle wrap;                                  // Control wrap style 
    PotLedMode ledMode;                                 // Control LED UI mode
    CHSV colours[2];                                    // Ring display colours
    uint16_t value;                                     // Current value
	 uint16_t numLeds;						// Number of LEDs in the bar
	 uint16_t firstLedIndex;				// Position of the first LED in the bar
	 uint8_t dataDirection;					// 0 = bottom to top or CW, 1 = top to bottom or CCW	
	 uint8_t interFade;						// Interpolation fade on or off		
} LEDBar;

// Public function prototypes
void ledBar_Update(LEDBar* ledBar);


// HSV Colours
// Neon
extern CHSV csvNeonLightBlue;
extern CHSV csvNeonLime;
extern CHSV csvNeonPink;
extern CHSV csvNeonSeafoam;
extern CHSV csvNeonRed;

#endif /* INTERFACE_H */