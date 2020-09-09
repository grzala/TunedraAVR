*/
 * freqDetect.h
 *
 * Created: 20/07/2020 13:26:41
 *  Author: mpana
 */ 


#ifndef NOTEDETECTION_H_
#define NOTEDETECTION_H_

#include <math.h>
#include <stdlib.h>

// constants for note detection
const double MIN_FREQ = 15.8927255;
const double FIRST_OCT_MAX_FREQ = 31.7854510;
const double MAX_FREQ = 508.565;
const int NOTES_IN_OCTAVE = 12;

const double firstOctaveFreqs[] = {
	16.3515978,
	17.3239144,
	18.3540480,
	19.4554365,
	20.6017223,
	21.8267644,
	23.1246514,
	24.4997147,
	25.9565435,
	27.5000000,
	29.1352351,
	30.8677063
};

const char noteNames[] = {'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B'};
const bool noteSharps[] = {false, true, false, true, false, false, true, false, true, false, true, false};

typedef struct  {
	char note;
	bool sharp;
	double freq;
	double min_freq;
	double max_freq;
	bool valid = false;
} Note;

bool isFreqLegal(double freq);
double get_octave_multiplier(double freq);
void getNoteByFreq(Note* note, double freq);


#endif /* NOTEDETECTION_H_ */