/*
 * freqDetect.cpp
 *
 * Created: 20/07/2020 13:26:48
 *  Author: mpana
 */ 

#include "noteDetection.h"


bool isFreqLegal(double freq) {
	return freq >= MIN_FREQ && freq < MAX_FREQ;
}

// Determine in which octave lies the frequency and get a multiplier required to work with that octave
double get_octave_multiplier(double freq) {
	double multiplier = 1.0;
	// double last_octave_freq = 0;
	while (freq > FIRST_OCT_MAX_FREQ * multiplier) {
		multiplier *= 2.0;
		// last_octave_freq = FIRST_OCT_MAX_FREQ * multiplier;
	}

	return multiplier;
}

void getNoteByFreq(Note* note, double freq) {
	if (!isFreqLegal(freq)) {
		note->valid = false;
		return;
	}
	
	double multiplier = get_octave_multiplier(freq);

	// Find closest note in O(n) time
	double min_distance = MAX_FREQ;
	int closest_i = -1;
	for (int note_i = 0; note_i < NOTES_IN_OCTAVE; note_i++) {
		double distance = abs(freq - (firstOctaveFreqs[note_i]*multiplier));
		if (distance < min_distance) {
			min_distance = distance;
			closest_i = note_i;
			} else {
			break;
		}
	}

	// Fill note struct
	note->note = noteNames[closest_i];
	note->sharp = noteSharps[closest_i];
	note->freq = firstOctaveFreqs[closest_i]*multiplier;
	note->valid = true;

	if (closest_i == NOTES_IN_OCTAVE-1) {
		note->max_freq = FIRST_OCT_MAX_FREQ*multiplier;
		} else {
		note->max_freq = note->freq + (((firstOctaveFreqs[closest_i+1]*multiplier) - note->freq)/2.0);
	}

	if (closest_i == 0) {
		note->min_freq = MIN_FREQ*multiplier;
		} else {
		note->min_freq = note->freq - ((note->freq - (firstOctaveFreqs[closest_i-1]*multiplier))/2.0);
	}
}
