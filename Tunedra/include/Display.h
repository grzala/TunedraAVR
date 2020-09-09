/*
 * Display.h
 *
 * Created: 20/07/2020 13:23:36
 *  Author: mpana
 */ 

// REMEMBER TO USE SOME DELAY IN LOOP WHERE DISPLAY IS CALLED


#ifndef DISPLAY_H_
#define DISPLAY_H_


#include "config.h"
#include <math.h>
#include "noteDetection.h"

// Map display segments to int
typedef enum {
	mid = 0,
	up = 1,
	upR = 2,
	downR = 3,
	down = 4,
	downL = 5,
	upL = 6,
} DisplBarName;
typedef DisplBarName DBN;

// Instructions for displaying letters
typedef enum {
	A = 0b1101111,
	B = 0b1111001,
	C = 0b1110010,
	D = 0b0111101,
	E = 0b1110011,
	F = 0b1100011,
	G = 0b1111010,
	H = 0b1101101,
	J = 0b0011100,
	I = 0b0001100,
	U = 0b1111100,
	S = 0b1011011,
	Z = 0b0110111,
} DisplInstruction;
typedef DisplInstruction DI;

#ifndef DISPLAY_PORT_LED_OUTPUT
# warning "DISPLAY PORT LED  NOT CONFIGURED"
#define DISPLAY_PORT_LED_OUTPUT PORTB
#endif
#ifndef DISPLAY_PORT_LED_CONFIG
# warning "DISPLAY PORT LED  NOT CONFIGURED"
#define DISPLAY_PORT_LED_CONFIG DDRB
#endif

#ifndef DISPLAY_PORT_OUTPUT
# warning "DISPLAY PORT NOT CONFIGURED"
#define DISPLAY_PORT_LED_OUTPUT PORTD
#endif
#ifndef DISPLAY_PORT_CONFIG
# warning "DISPLAY PORT NOT CONFIGURED"
#define DISPLAY_PORT_LED_CONFIG DDRD
#endif


#define INDICATOR_BAR_LEN 5

extern "C" {
	#include "light_ws2812.h"
};


typedef struct {
	double max_distance;
	double As[3];
	double Bs[3];
	double xBounds[3];
} LEDFunctionCache;

// Display class handles LED 7 segment display, LED indicating sharp note and a LED bar indicating deviation
class Display {
	public:
	static const int LEDFunctions_Len = 3; // 3 linear functions will help determine LED brightness
	
	void initialize();
	void clean();
	void light(DI instruction);
	void light(unsigned int instruction);
	void lightSharp(bool light);
	void lightIndicator(const Note* note, double currentFreq);
	void displayNote(const Note* note, double frequency);
	void resetIfTime(unsigned int* timer);

	private:
	void cleanIndicator();
	void write(unsigned int pin);
	
	
	unsigned int currentlyDisplaying = 0;
	bool currentSharpPinStatus = false;
	cRGB indicatorBar[INDICATOR_BAR_LEN];
	
	static const unsigned int time_to_reset = 5000;

	
	// Adjust those to alter light. LEDs are lighted according to three linear functions intersecting
	static constexpr double MAX_ANALOG {MAX_ANALOG_LED_VAL}; // LEDS do not need to go any higher
	
	static constexpr double xBoundFactors[LEDFunctions_Len] = { 0.08, 0.15, 0.35 };
	static constexpr double yBoundFactors[LEDFunctions_Len] = { 0.45, 0.15, 0.0 };
	static constexpr double yBounds[LEDFunctions_Len] = {
		yBoundFactors[0] * MAX_ANALOG,
		yBoundFactors[1] * MAX_ANALOG,
		yBoundFactors[2] * MAX_ANALOG
	};
	
	LEDFunctionCache ledFCache;

	
	int getIndicatorValByDistance(double distance, double max_distance);
	void rebuildCache(double max_distance);
	void printCacheInfo();
};

// END CLASS DISPLAY DECLARATION


#endif /* DISPLAY_H_ */