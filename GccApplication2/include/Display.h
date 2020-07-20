/*
 * Display.h
 *
 * Created: 20/07/2020 13:23:36
 *  Author: mpana
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_


#include <math.h>
#include "freqDetect.h"

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

// Display class handles LED 7 segment display, LED indicating sharp note and a LED bar indicating deviation
class Display {
	public:
	static const int LEDFunctions_Len = 3; // 3 linear functions will help determine LED brightness
	
	Display(int midPin, int upPin, int upRPin, int downRPin,
	int downPin, int downLPin, int UpLPin, int sharpPin,
	int rLED0, int rLED1, int gLED, int rLED2, int rLED3);
	void clean();
	void cleanIndicator();
	void do_sth1();
	void do_sth2();
	void light(DI instruction);
	void light(unsigned int instruction);
	void lightSharp(bool light);
	void lightIndicator(double currentFreq, const Note* note);
	void write(DBN pin);
	void write(unsigned int pin);
	void displayNote(const Note* note, double frequency);
	void resetIfTime();

	private:
	int pin_array[7];
	unsigned int currentlyDisplaying = 0;
	int sharpPin;
	bool currentSharpPinStatus = false;
	int indicatorBar[5];
	
	unsigned long time_at_last_display = 0;
	const unsigned int time_to_reset = 5000;

	
	// Adjust those to alter light. LEDs are lighted according to three linear functions intersecting
	constexpr static double MAX_ANALOG = 200.0; // LEDS do not need to go any higher
	
	constexpr static double xBoundFactors[LEDFunctions_Len] = { 0.035, 0.15, 0.35 };
	constexpr static double yBoundFactors[LEDFunctions_Len] = { 0.4, 0.05, 0.0 };
	constexpr static double yBounds[LEDFunctions_Len] = {
		yBoundFactors[0] * MAX_ANALOG,
		yBoundFactors[1] * MAX_ANALOG,
		yBoundFactors[2] * MAX_ANALOG
	};
	
	typedef struct {
		double max_distance;
		double As[Display::LEDFunctions_Len];
		double Bs[Display::LEDFunctions_Len];
		double xBounds[Display::LEDFunctions_Len];
	} LEDFunctionCache;
	LEDFunctionCache ledFCache {-1};

	
	int getIndicatorValByDistance(double distance, double max_distance);
	void rebuildCache(double max_distance);
	void printCacheInfo();
};
// END CLASS DISPLAY DECLARATION



#endif /* DISPLAY_H_ */