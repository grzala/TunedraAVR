#include "Display.h"


// IMPLEMENT CLASS DISPLAY
Display::Display(int midPin, int upPin, int upRPin, int downRPin,
				int downPin, int downLPin, int UpLPin, int sharpPin,
				int rLED0, int rLED1, int gLED, int rLED2, int rLED3) {
	//pinMode(midPin, OUTPUT);
	//pinMode(upPin, OUTPUT);
	//pinMode(upRPin, OUTPUT);
	//pinMode(downRPin, OUTPUT);
	//pinMode(downPin, OUTPUT);
	//pinMode(downLPin, OUTPUT);
	//pinMode(UpLPin, OUTPUT);
	//
	//pinMode(sharpPin, OUTPUT);
	//
	//pinMode(rLED0, OUTPUT);
	//pinMode(gLED, OUTPUT);
	//pinMode(rLED1, OUTPUT);
	
	this->pin_array[0] = midPin;
	this->pin_array[1] = upPin;
	this->pin_array[2] = upRPin;
	this->pin_array[3] = downRPin;
	this->pin_array[4] = downPin;
	this->pin_array[5] = downLPin;
	this->pin_array[6] = UpLPin;
	
	this->sharpPin = sharpPin;

	this->indicatorBar[0] = rLED0;
	this->indicatorBar[1] = rLED1;
	this->indicatorBar[2] = gLED;
	this->indicatorBar[3] = rLED2;
	this->indicatorBar[4] = rLED3;

	this->clean();
	this->lightSharp(false);
	this->cleanIndicator();
}

void Display::clean() {
	//digitalWrite(this->pin_array[0], LOW);
	//digitalWrite(this->pin_array[1], LOW);
	//digitalWrite(this->pin_array[2], LOW);
	//digitalWrite(this->pin_array[3], LOW);
	//digitalWrite(this->pin_array[4], LOW);
	//digitalWrite(this->pin_array[5], LOW);
	//digitalWrite(this->pin_array[6], LOW);
	this->currentlyDisplaying = 0;
}

void Display::cleanIndicator() {
	//analogWrite(this->indicatorBar[0], 0);
	//analogWrite(this->indicatorBar[1], 0);
	//analogWrite(this->indicatorBar[2], 0);
	//analogWrite(this->indicatorBar[3], 0);
	//analogWrite(this->indicatorBar[4], 0);
}

void Display::do_sth1() {
	this->clean();
	
	//digitalWrite(this->pin_array[DBN::mid], HIGH);
	//digitalWrite(this->pin_array[DBN::up], HIGH);
	//digitalWrite(this->pin_array[DBN::down], HIGH);
}

void Display::do_sth2() {
	this->clean();
	
	//digitalWrite(this->pin_array[DBN::upR], HIGH);
	//digitalWrite(this->pin_array[DBN::downR], HIGH);
	//digitalWrite(this->pin_array[DBN::upL], HIGH);
	//digitalWrite(this->pin_array[DBN::downL], HIGH);
}

void Display::write(DBN pin) {
	this->write(static_cast<unsigned int>(pin));
}

void Display::write(unsigned int pin) {
	//digitalWrite(this->pin_array[pin], HIGH);
}

void Display::light(DI instruction) {
	this->light(static_cast<unsigned int>(instruction));
}

void Display::light(unsigned int instruction) {
	if (instruction == this->currentlyDisplaying) return;
	
	this->clean();

	if (instruction & (1 << DBN::mid)) {
		this->write(DBN::mid);
	}
	if (instruction & (1 << DBN::up)) {
		this->write(DBN::up);
	}
	if (instruction & (1 << DBN::upR)) {
		this->write(DBN::upR);
	}
	if (instruction & (1 << DBN::downR)) {
		this->write(DBN::downR);
	}
	if (instruction & (1 << DBN::down)) {
		this->write(DBN::down);
	}
	if (instruction & (1 << DBN::downL)) {
		this->write(DBN::downL);
	}
	if (instruction & (1 << DBN::upL)) {
		this->write(DBN::upL);
	}
	
	this->currentlyDisplaying = instruction;
}

void Display::lightSharp(bool light) {
	//if (light) {
		//digitalWrite (this->sharpPin, HIGH);
		//} else {
		//digitalWrite (this->sharpPin, LOW);
	//}
	//this->currentSharpPinStatus = light;
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
	ledFCache.xBounds[0] = xBoundFactors[0] * max_distance;
	ledFCache.xBounds[1] = xBoundFactors[1] * max_distance;
	ledFCache.xBounds[2] = xBoundFactors[2] * max_distance;

	ledFCache.As[0] = (yBounds[0] - MAX_ANALOG) / (ledFCache.xBounds[0]);
	ledFCache.Bs[0] = MAX_ANALOG;
	
	ledFCache.As[1] = (yBounds[1] - yBounds[0]) / (ledFCache.xBounds[1] - ledFCache.xBounds[0]);
	ledFCache.Bs[1] = yBounds[1] - (ledFCache.As[1] * ledFCache.xBounds[1]);
	
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
void Display::lightIndicator(double currentFreq, const Note* note) {
	if (currentFreq < note->min_freq || currentFreq > note->max_freq) {
		return;
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
	
	for (int i = 0; i < 5; i++) {
		int val = this->getIndicatorValByDistance(dists[i], max_dist);
		//analogWrite(this->indicatorBar[i], val);
	}
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
	this->lightIndicator(frequency, note);
	//this->time_at_last_display = millis();
}

void Display::resetIfTime() {
	//unsigned long currentTime = millis();
	//if (currentTime - this->time_at_last_display > this->time_to_reset) {
		//this->clean();
		//this->lightSharp(false);
		//this->cleanIndicator();
	//}
}