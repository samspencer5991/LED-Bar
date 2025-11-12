#include <led_bar.h>

#define LED_CENTRE_DIMMING 4
#define LED_START_DIMMING 4

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
void ledBar_Init(LEDBar* ledBar, uint16_t firstLedIndex, uint16_t numLeds, PotMode mode, PotWrapStyle wrap,
	PotLedMode ledMode, uint8_t dataDirection, CHSV colour1, CHSV colour2)
{
	ledBar->ledArrayPtr = (CRGB*)malloc(sizeof(CRGB) * numLeds);
	memset(ledBar->ledArrayPtr, 0, sizeof(CRGB) * numLeds);
	ledBar->firstLedIndex = firstLedIndex;
	ledBar->numLeds = numLeds;
	ledBar->mode = mode;
	ledBar->wrap = wrap;
	ledBar->ledMode = ledMode;
	ledBar->dataDirection = dataDirection;
	ledBar->colours[0] = colour1;
	ledBar->colours[1] = colour2;
	ledBar->value = 0;
	ledBar->interFade = 1; // Enable interpolation fading
}
void ledBar_Update(LEDBar* ledBar)
{
	uint16_t ringCount;				 // Number of LEDs in the ring for value feedback
	uint16_t segmentSize;			 // Size of each segment in the pot value range
	uint16_t ringPos;				 // Current position in the ring
	CRGB interpolateLedColour;		 // Colour of the LED to be interpolated
	uint16_t interpolatePos;		 // Position of the LED to be interpolated
	uint8_t fadeAmount = 0;			 // Fade amount for interpolation
	uint16_t positionInSegment;		 // Position within the current segment
	uint8_t overflowMod;			 // Overflow from integer division of segments
	static uint16_t valueUpdate = 0; // For debugging value changes

	// If the numLeds is even, do not light the bottom centre LED (reduce the count by 1)
	ringCount = ledBar->numLeds;
	if (ledBar->numLeds % 2 == 0)
		ringCount -= 1;
	segmentSize = 1024 / ringCount;
	// Integer division gives segment index
	ringPos = ledBar->value / segmentSize;
	// Clamp to maximum position
	if (ringPos >= ringCount)
		ringPos = ringCount - 1;

	

	// The ledArrayPtr is a virtual mapping for the led bar.
	// This allows for different physical arrangements of the LEDs
	// The virtual mapping is always treated the same way regardless of physical layout
	// This is then mapped to the actual leds[] array based on firstLedIndex and dataDirection
	// The virtual mapping space always starts at index 0 to numLeds-1 in CW direction
	// An LED in the bottom centre position is always considered the last LED in the ring (numLeds-1)

	if (ledBar->mode == PotNormal)
	{
		if (ledBar->interFade)
		{
			positionInSegment = ledBar->value % segmentSize;
			// Clamp the positionInSegment to the max ring position to avoid index overflow
			// Get the overflow from the segment division
			overflowMod = 1024 % ringCount;
			if (ringPos == ringCount - 1 && ledBar->value >= 1024 - overflowMod)
				positionInSegment = segmentSize;
			if(ringPos == 0)
				fadeAmount = map(positionInSegment, 0, segmentSize, (255/LED_START_DIMMING)-1, 255);
			else
				fadeAmount = map(positionInSegment, 0, segmentSize, 0, 255);
		}
		// Apply the ring colour mapping
		if (ledBar->ledMode == PotLedFillGradient || ledBar->ledMode == PotLedDotGradient)
		{
			fill_gradient(ledBar->ledArrayPtr, 0, ledBar->colours[0],
				ringCount-1, ledBar->colours[1], SHORTEST_HUES);
		}
		else if(ledBar->ledMode == PotLedFill || ledBar->ledMode == PotLedDot)
		{
			for (uint8_t j = 0; j < ringCount; j++)
				ledBar->ledArrayPtr[j] = ledBar->colours[0];
		}

		// Fill modes
		if (ledBar->ledMode == PotLedFillGradient || ledBar->ledMode == PotLedFill)
		{
			// Capture the previous LED position colour for interpolation fade
			interpolatePos = ringPos;
			interpolateLedColour = ledBar->ledArrayPtr[interpolatePos];
			for (uint8_t j = ringPos+1; j < ringCount; j++)
				ledBar->ledArrayPtr[j] = 0;

			// Determine the faded brightness for interpolation
			// This is done by dividing the input value range by the number of LEDs to map the delta to the next position
			// Prevent the middle LED from being faded
			if (ledBar->interFade)
			{
				fadeAmount = fadeAmount;
				interpolateLedColour.nscale8(fadeAmount);
				ledBar->ledArrayPtr[interpolatePos] = interpolateLedColour;
			}
		}

		// Dot modes
		else if(ledBar->ledMode == PotLedDotGradient || ledBar->ledMode == PotLedDot)
		{
			// Capture the current position LED colour
			CRGB currentLedColour = ledBar->ledArrayPtr[ringPos];
			// Turn off all LEDs
			for (uint8_t j = 0; j < ledBar->numLeds; j++)
				ledBar->ledArrayPtr[j] = 0;
			// Light the current position LED
			ledBar->ledArrayPtr[ringPos] = currentLedColour;
		}
	}
	else if (ledBar->mode == PotCentred)
	{
		if (ledBar->interFade)
		{
			positionInSegment = ledBar->value % segmentSize;
			// Clamp the positionInSegment to the max ring position to avoid index overflow
			// Get the overflow from the segment division
			overflowMod = 1024 % ringCount;
			if (ringPos == ringCount - 1 && ledBar->value >= 1024 - overflowMod)
				positionInSegment = segmentSize;
			fadeAmount = map(positionInSegment, 0, segmentSize, 0, 255);
		}
		// Apply the ring colour mapping
		if (ledBar->ledMode == PotLedFillGradient || ledBar->ledMode == PotLedDotGradient)
		{
			fill_gradient(ledBar->ledArrayPtr, 0, ledBar->colours[0],
				ringCount - 1, ledBar->colours[1], SHORTEST_HUES);
		}
		else if(ledBar->ledMode == PotLedFill || ledBar->ledMode == PotLedDot)
		{
			for (uint8_t j = 0; j < ringCount; j++)
				ledBar->ledArrayPtr[j] = ledBar->colours[0];
		}

		// Fill modes
		if (ledBar->ledMode == PotLedFillGradient || ledBar->ledMode == PotLedFill)
		{
			// Capture the previous LED position colour for interpolation fade
			interpolatePos = ringPos;
			interpolateLedColour = ledBar->ledArrayPtr[interpolatePos];
			// Current position in the lower half
			if (ringPos < (ledBar->numLeds / 2 - 1))
			{
				// Turn off LEDs below current position
				for (uint8_t j = 0; j < ringPos; j++)
					ledBar->ledArrayPtr[j] = 0;

				// Turn off LEDs above centre
				for (uint8_t j = (ledBar->numLeds / 2); j < ledBar->numLeds; j++)
					ledBar->ledArrayPtr[j] = 0;
			}
			// Current position in the upper half
			else if (ringPos >= (ledBar->numLeds / 2 - 1))
			{
				// Turn off LEDs below centre
				for (uint8_t j = 0; j < (ledBar->numLeds / 2 - 1); j++)
					ledBar->ledArrayPtr[j] = 0;

				// Turn off LEDs above current position
				for (uint8_t j = ringPos + 1; j < ledBar->numLeds; j++)
					ledBar->ledArrayPtr[j] = 0;
			}
			// Determine the faded brightness for interpolation
			// This is done by dividing the input value range by the number of LEDs to map the delta to the next position
			// Prevent the middle LED from being faded
			if (ledBar->interFade && ringPos != (ledBar->numLeds / 2 - 1))
			{
				if (ringPos < (ledBar->numLeds / 2 - 1))
				{
					fadeAmount = 255 - fadeAmount;
				}
				else if (ringPos >= (ledBar->numLeds / 2 - 1))
				{
					fadeAmount = fadeAmount;
				}
				interpolateLedColour.nscale8(fadeAmount);
				ledBar->ledArrayPtr[interpolatePos] = interpolateLedColour;
			}
		}
		// Dot modes
		else if(ledBar->ledMode == PotLedDotGradient || ledBar->ledMode == PotLedDot)
		{
			// Capture the centre and current position LED colours
			CRGB centreLedColour = ledBar->ledArrayPtr[ledBar->numLeds / 2 - 1];
			CRGB currentLedColour = ledBar->ledArrayPtr[ringPos];
			// Turn off all LEDs
			for (uint8_t j = 0; j < ledBar->numLeds; j++)
				ledBar->ledArrayPtr[j] = 0;
			// Light the centre and current position LEDS
			// If the ringPos is not the centre LED, dim the centre
			if (ringPos != (ledBar->numLeds / 2 - 1))
			{
				centreLedColour.nscale8(255 / LED_CENTRE_DIMMING);
			}
			ledBar->ledArrayPtr[ledBar->numLeds / 2 - 1] = centreLedColour;
			ledBar->ledArrayPtr[ringPos] = currentLedColour;
		}
	}
	if (ledBar->value != valueUpdate)
	{
		valueUpdate = ledBar->value;
		char str[1024];
		sprintf(str, "Ring Pos: %d,  Value: %d  Segment Size: %d,  PosInSeg: %d,  FadeAmt: %d\r\n", ringPos, ledBar->value, segmentSize, positionInSegment, fadeAmount);
		Serial.print(str);
	}

	// Map the virtual ledArrayPtr to the actual leds[] array
	for (uint16_t i = 0; i < ledBar->numLeds; i++)
	{
		uint16_t targetIndex = 0;
		if (ledBar->dataDirection == 0)
		{
			targetIndex = ledBar->firstLedIndex + i;
		}
		else
		{
			targetIndex = ledBar->firstLedIndex + (ledBar->numLeds - 1) - i;
		}
		leds[targetIndex] = ledBar->ledArrayPtr[i];
	}

	/*
	if (ledBar->dataDirection == 0)
	{
		ledPos = ringPos + ledBar->firstLedIndex;
		ledEnd = ledBar->firstLedIndex + ledBar->numLeds - 1;
	}
	else
	{
		ledPos = ledBar->firstLedIndex + (ledBar->numLeds - 1) - ringPos;
		ledEnd = ledBar->firstLedIndex;
	}

	if (ledBar->mode == PotNormal)
	{
		if (ledBar->ledMode == PotLedDot)
		{
			if (ledBar->dataDirection == 0)
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

			if (ledBar->dataDirection == 0)
			{
				fill_gradient(leds, ledBar->firstLedIndex, ledBar->colours[0],
							  ledEnd, ledBar->colours[1], SHORTEST_HUES);
				for (uint8_t j = ledPos + 1; j <= ledEnd; j++)
					leds[j] = 0;
			}
			else
			{
				fill_gradient(leds, ledBar->firstLedIndex, ledBar->colours[1],
							  ledBar->firstLedIndex + ledBar->numLeds - 1, ledBar->colours[0], SHORTEST_HUES);
				for (uint8_t j = ledBar->firstLedIndex; j <= ledPos - 1; j++)
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
			// CW direction
			if (ledBar->dataDirection == 0)
			{
				fill_gradient(leds, ledBar->firstLedIndex, ledBar->colours[0],
							  ledEnd, ledBar->colours[1], SHORTEST_HUES);
				// Current position in the lower half
				if (ringPos < (ledBar->numLeds / 2 - 1))
				{
					// Turn off LEDs below current position
					for (uint8_t j = ledBar->firstLedIndex; j < ledPos; j++)
						leds[j] = 0;

					// Turn off LEDs above centre
					for (uint8_t j = ledBar->firstLedIndex + 11; j <= ledEnd; j++)
						leds[j] = 0;
				}
				// Current position in the upper half
				else if (ringPos > (ledBar->numLeds / 2 - 1))
				{
					// Turn off LEDs below centre
					for (uint8_t j = ledBar->firstLedIndex; j < ledBar->firstLedIndex + 10; j++)
						leds[j] = 0;

					// Turn off LEDs above current position
					for (uint8_t j = ledPos + 1; j <= ledEnd; j++)
						leds[j] = 0;
				}
			}
			// CCW direction
			else
			{
				fill_gradient(leds, ledBar->firstLedIndex, ledBar->colours[1],
							  ledBar->firstLedIndex + ledBar->numLeds - 1, ledBar->colours[0], SHORTEST_HUES);
				// Current position in the lower half
				if (ringPos <= (ledBar->numLeds / 2 - 1))
				{
					// Turn off LEDs below current position
					for (uint8_t j = (ledBar->firstLedIndex + ledBar->numLeds) - 1; j > ledPos; j--)
						leds[j] = 0;

					// Turn off LEDs above centre
					for (uint8_t j = ledBar->firstLedIndex; j <= ledBar->firstLedIndex + (ledBar->numLeds / 2 - 1); j++)
						leds[j] = 0;
				}
				// Current position in the upper half
				else if (ringPos > (ledBar->numLeds / 2 - 1))
				{
					// Turn off LEDs below centre
					for (uint8_t j = ledBar->firstLedIndex + ledBar->numLeds - 1; j > ledBar->firstLedIndex + (ledBar->numLeds / 2); j--)
						leds[j] = 0;

					// Turn off LEDs above current position
					for (uint8_t j = ledBar->firstLedIndex; j < ledPos + 1; j++)
						leds[j] = 0;
				}
			}
		}
	}
		*/
		// FastLED.show();
}