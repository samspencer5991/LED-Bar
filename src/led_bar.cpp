#include <led_bar.h>

#define LED_CENTRE_DIMMING 2

#define POT_WRAP_THRESHOLD 50

// CRGB leds[NUM_LEDS];

// HSV Colours
// Neon
CHSV csvNeonLightBlue = CHSV(130, 240, 255);
CHSV csvNeonLime = CHSV(111, 235, 255);
CHSV csvNeonPink = CHSV(240, 204, 250);
CHSV csvNeonSeafoam = CHSV(107, 247, 250);
CHSV csvNeonRed = CHSV(2, 196, 255);

int controlFocusToggle = -1;

//------------------ Input Aquisition and Processing ----------------------
void ledBar_Update(LEDBar *ledBar)
{
	uint8_t ringPos = map(ledBar->value, 0, 1023, 0, ledBar->numLeds - 1);
	uint16_t ledPos = 0;
	uint16_t ledEnd = 0;

	if(ledBar->dataDirection == 0)
	{
		ledPos = ringPos + ledBar->firstLedIndex;
		ledEnd = ledBar->firstLedIndex + ledBar->numLeds-1;
	}
	else
	{
		ledPos = ledBar->firstLedIndex + (ledBar->numLeds-1) - ringPos;
		ledEnd = ledBar->firstLedIndex;
	}


	if (ledBar->mode == PotNormal)
	{
		if (ledBar->ledMode == PotLedDot)
		{
			if(ledBar->dataDirection == 0)
			{
				for (uint8_t j = ledBar->firstLedIndex; j <= ledEnd; j++)
					leds[j] = 0;
			}
			else
			{
				for (uint8_t j = ledBar->firstLedIndex; j <= ledBar->firstLedIndex + ledBar->numLeds; j++)
					leds[j] = 0;
			}

			leds[ledPos] = ledBar->colours[0];
		}
		else if (ledBar->ledMode == PotLedDotGradient)
		{
			fill_gradient(leds, ledBar->firstLedIndex, ledBar->colours[0],
							  ledEnd, ledBar->colours[1], SHORTEST_HUES);
			for (uint8_t j = ledPos + 1; j <= ledEnd; j++)
				leds[j] = 0;

			for (uint8_t j = ledBar->firstLedIndex; j < ledPos; j++)
				leds[j] = 0;

		}
		else if (ledBar->ledMode == PotLedFill)
		{
			for (uint8_t j = ledBar->firstLedIndex; j <= ledPos; j++)
				leds[j] = ledBar->colours[0];

			for (uint8_t j = ledPos + 1; j <= ledEnd; j++)
				leds[j] = 0;

		}
		else if (ledBar->ledMode == PotLedFillGradient)
		{
			
			if(ledBar->dataDirection == 0)
			{
				fill_gradient(leds, ledBar->firstLedIndex, ledBar->colours[0],
					ledEnd, ledBar->colours[1], SHORTEST_HUES);
				for (uint8_t j = ledPos + 1; j <= ledEnd; j++)
					leds[j] = 0;
			}
			else
			{
				fill_gradient(leds, ledBar->firstLedIndex, ledBar->colours[1],
					ledBar->firstLedIndex + ledBar->numLeds-1, ledBar->colours[0], SHORTEST_HUES);
				for(uint8_t j = ledBar->firstLedIndex; j <= ledPos-1; j++)
					leds[j] = 0;
			}

		}
	}
	else if (ledBar->mode == PotCentred)
	{
		if (ledBar->ledMode == PotLedDot)
		{
			for (uint8_t j = ledBar->firstLedIndex; j <= ledEnd; j++)
				leds[j] = 0;

			leds[ledPos] = ledBar->colours[0];
			CHSV dimColour = ledBar->colours[0];
			dimColour.value = dimColour.value / LED_CENTRE_DIMMING;
			leds[ledBar->firstLedIndex + 10] = dimColour;
		}
		else if (ledBar->ledMode == PotLedDotGradient)
		{
			fill_gradient(leds, ledBar->firstLedIndex, ledBar->colours[0],
							  ledEnd, ledBar->colours[1], SHORTEST_HUES);
			for (uint8_t j = ledPos + 1; j <= ledEnd; j++)
				leds[j] = 0;

			for (uint8_t j = ledBar->firstLedIndex; j < ledPos; j++)
				leds[j] = 0;

			// Find blended gradient colour for centre point
			uint8_t hue = (ledBar->colours[0].hue + ledBar->colours[1].hue) / 2;
			uint8_t sat = (ledBar->colours[0].hue + ledBar->colours[1].sat) / 2;
			uint8_t val = (ledBar->colours[0].hue + ledBar->colours[1].val) / (2 * LED_CENTRE_DIMMING);
			CHSV dimColour = CHSV(hue, sat, val);
			leds[ledBar->firstLedIndex + 10] = dimColour;
		}
		else if (ledBar->ledMode == PotLedFill)
		{
			for (uint8_t j = ledBar->firstLedIndex; j <= ledEnd; j++)
				leds[j] = 0;

			if (ringPos <= 10)
			{
				for (uint8_t j = ledPos; j <= ledBar->firstLedIndex + 10; j++)
					leds[j] = ledBar->colours[0];

			}
			else if (ringPos > 10)
			{
				for (uint8_t j = ledBar->firstLedIndex + 10; j <= ledPos; j++)
					leds[j] = ledBar->colours[0];

			}
		}
		else if (ledBar->ledMode == PotLedFillGradient)
		{
			fill_gradient(leds, ledBar->firstLedIndex, ledBar->colours[0],
							  ledEnd, ledBar->colours[1], SHORTEST_HUES);
			if (ringPos <= 10)
			{
				for (uint8_t j = ledBar->firstLedIndex; j < ledPos; j++)
					leds[j] = 0;

				for (uint8_t j = ledBar->firstLedIndex + 11; j <= ledEnd; j++)
					leds[j] = 0;

			}
			else if (ringPos > 10)
			{

				for (uint8_t j = ledPos + 1; j <= ledEnd; j++)
					leds[j] = 0;

				for (uint8_t j = ledBar->firstLedIndex; j < ledBar->firstLedIndex + 10; j++)
					leds[j] = 0;

			}
		}
	}
	//FastLED.show();
}