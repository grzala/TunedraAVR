#include "Display.h"


// IMPLEMENT CLASS DISPLAY
void Display::initialize() {
					
	DISPLAY_PORT_LED_CONFIG |=_BV(ws2812_pin);
	DISPLAY_PORT_CONFIG |= 0xFF;


	this->clean();
	this->lightSharp(false);
	this->cleanIndicator();
}

void Display::clean() {
	DISPLAY_PORT_OUTPUT = 0x00;
	this->currentlyDisplaying = 0;
}

void Display::cleanIndicator() {
	for (int i = 0; i < 5; i++) {
		this->indicatorBar[i].r = 0;
		this->indicatorBar[i].g = 0;
		this->indicatorBar[i].b = 0;
	}
	ws2812_sendarray((uint8_t *)this->indicatorBar, INDICATOR_BAR_LEN*3);
}

void Display::write(unsigned int pin) {
	DISPLAY_PORT_OUTPUT |= _BV(pin);
}

void Display::light(DI instruction) {
	this->light(static_cast<unsigned int>(instruction));
}

void Display::light(unsigned int instruction) {
	if (instruction == this->currentlyDisplaying) return;
	
	this->clean();

	if (instruction & (1 << DBN::mid)) {
		this->write(midPin_);
	}
	if (instruction & (1 << DBN::up)) {
		this->write(upPin_);
	}
	if (instruction & (1 << DBN::upR)) {
		this->write(upRPin_);
	}
	if (instruction & (1 << DBN::downR)) {
		this->write(downRPin_);
	}
	if (instruction & (1 << DBN::down)) {
		this->write(downPin_);
	}
	if (instruction & (1 << DBN::downL)) {
		this->write(downLPin_);
	}
	if (instruction & (1 << DBN::upL)) {
		this->write(upLPin_);
	}
	
	this->currentlyDisplaying = instruction;
}

void Display::lightSharp(bool light) {
	if (light) {
		DISPLAY_PORT_OUTPUT |= _BV(sharpPin_);
	} else {
		DISPLAY_PORT_OUTPUT &= ~_BV(sharpPin_);
	}
	this->currentSharpPinStatus = light;
}


void Display::printCacheInfo() {
	//Serial.print("f(x) = ");
	//Serial.print(ledFCache.As[0]);
	//Serial.print("x + ");
	//Serial.print(ledFCache.Bs[0]);
	//Serial.println();
	//Serial.print("f(x) = ");
	//Serial.print(ledFCache.As[1]);
	//Serial.print("x + ");
	//Serial.print(ledFCache.Bs[1]);
	//Serial.println();
	//Serial.print("f(x) = ");
	//Serial.print(ledFCache.As[2]);
	//Serial.print("x + ");
	//Serial.print(ledFCache.Bs[2]);
	//Serial.println();
}

// build a and b coefficients for linear functions
void Display::rebuildCache(double max_distance) {
	this->ledFCache.xBounds[0] = this->xBoundFactors[0] * max_distance;
	this->ledFCache.xBounds[1] = this->xBoundFactors[1] * max_distance;
	this->ledFCache.xBounds[2] = this->xBoundFactors[2] * max_distance; 
	
	this->ledFCache.As[0] = (this->yBounds[0] - this->MAX_ANALOG) / (this->ledFCache.xBounds[0]);
	this->ledFCache.Bs[0] = this->MAX_ANALOG;
	
	this->ledFCache.As[1] = (this->yBounds[1] - this->yBounds[0]) / (this->ledFCache.xBounds[1] - this->ledFCache.xBounds[0]);
	this->ledFCache.Bs[1] = this->yBounds[1] - (this->ledFCache.As[1] * this->ledFCache.xBounds[1]);
	
	ledFCache.As[2] = (yBounds[2] - yBounds[1]) / (ledFCache.xBounds[2] - ledFCache.xBounds[1]);
	ledFCache.Bs[2] = yBounds[2] - (ledFCache.As[2] * ledFCache.xBounds[2]);
}

int Display::getIndicatorValByDistance(double distance, double max_distance) {
	this->rebuildCache(max_distance); // find new functions
	
	if (distance < 0) return MAX_ANALOG; // distance should never be negative - in case it is, return max
	if (distance > ledFCache.xBounds[2]) return 0; // always no light if distance too high

	// Find which function to use
	int i = 0;
	for(; i < LEDFunctions_Len; i++)
	if (distance <= ledFCache.xBounds[i])
	break;
	
	int val = (int)((ledFCache.As[i] * distance) + ledFCache.Bs[i]);
	return val;
}

// currentFreq must be beterrn min and max freq of note
void Display::lightIndicator(const Note* note, double currentFreq) {
	if (currentFreq < note->min_freq || currentFreq > note->max_freq) {
		//return;
	}
	double max_dist = (note->max_freq - note->min_freq);

	double bound_1 = note->freq - (note->freq  - note->min_freq)/2.0;
	double bound_3 = note->freq + (note->max_freq - note->freq)/2.0;
	
	double dists[] = {
		fmin(max_dist, currentFreq - note->min_freq),
		fmin(max_dist, abs(currentFreq - bound_1)),
		fmin(max_dist, abs(currentFreq - note->freq)),
		fmin(max_dist, abs(bound_3 - currentFreq)),
		fmin(max_dist, note->max_freq - currentFreq)
	};
	
	for (int i = 0; i < INDICATOR_BAR_LEN; i++) {
		int val = this->getIndicatorValByDistance(dists[i], max_dist);
		if (i != 2) {
			this->indicatorBar[i].r = val;
		} else { 
			this->indicatorBar[i].g = val;
		}
	}
	

	ws2812_sendarray((uint8_t *)this->indicatorBar, INDICATOR_BAR_LEN*3);
}

void Display::displayNote(const Note* note, double frequency) {
	DI di = DI::A;
	switch(note->note) {
		case 'A':
		di = DI::A;
		break;
		case 'B':
		di = DI::B;
		break;
		case 'C':
		di = DI::C;
		break;
		case 'D':
		di = DI::D;
		break;
		case 'E':
		di = DI::E;
		break;
		case 'F':
		di = DI::F;
		break;
		case 'G':
		di = DI::G;
		break;
	}
	
	this->light(di);
	this->lightSharp(note->sharp);
	this->lightIndicator(note, frequency);
}

void Display::resetIfTime(unsigned int* timer) {
	if (*timer > this->time_to_reset) {
		this->clean();
		this->lightSharp(false);
		this->cleanIndicator();
		*timer = 0;
	}
}
