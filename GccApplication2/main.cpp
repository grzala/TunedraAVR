#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "config.h"

#include <util/delay.h>
#define DELAY _delay_ms
#include <avr/interrupt.h>

#include "noteDetection.h"
#include "Display.h"


#include <serial.h>

// --------------------------------------------------------------- SETUP ---------------------------------------------------------------------------------


Note currentNote;
Display displ;
void setup() {
	DDRC = 0b00000000; // SET C0 to iput
	//Serial.begin(115200);
	
	// (mid, up, upright, downright, down, leftdown, rightdown, sharp, led data pin)
	//displ.initialize();
	
	cli();//diable interrupts
	
	//set up continuous sampling of analog pin 0 at 38.5kHz
	
	//clear ADCSRA register
	ADCSRA = 0;
	
	// 0b01100000;
	ADMUX |= (1 << REFS0); //set reference voltage
	ADMUX |= (1 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only
	
	// 0b11101100
	ADCSRA |= (1 << ADPS2);// | (1 << ADPS0); //set ADC clock with 16 prescaler- 8mHz/16=500kHz
	ADCSRA |= (1 << ADFR); //enabble auto trigger
	ADCSRA |= (1 << ADIE); //enable interrupts when measurement complete
	ADCSRA |= (1 << ADEN); //enable ADC
	ADCSRA |= (1 << ADSC); //start ADC measurements
	
	sei();//enable interrupts
}
// --------------------------------------------------------------- END SETUP ---------------------------------------------------------------------------------



// --------------------------------------------------------------- PHYSICS ---------------------------------------------------------------------------------
//clipping indicator variables
static volatile bool clipping = 0;

//data storage variables
static volatile char newData = 0;
static volatile char prevData = 0;
static volatile unsigned int time = 0;//keeps time and sends vales to store in timer[] occasionally
static volatile int timer[10];//storage for timing of events
static volatile int slope[10];//storage for slope of events
static volatile unsigned int totalTimer;//used to calculate period
static volatile unsigned int period;//storage for period of wave
static volatile bool periodReady = false;
static volatile unsigned char index = 0;//current storage index
static volatile float frequency;//storage for frequency calculations
static volatile int maxSlope = 0;//used to calculate max slope as trigger point
static volatile int newSlope;//storage for incoming slope data

//variables for decided whether you have a match
static volatile char noMatch = 0;//counts how many non-matches you've received to reset variables if it's been too long
static volatile char slopeTol = 3;//slope tolerance- adjust this if you need
static volatile int timerTol = 10;//timer tolerance- adjust this if you need

//variables for amp detection
unsigned int ampTimer = 0;
static volatile char maxAmp = 0;
static volatile char checkMaxAmp;
static volatile char ampThreshold = 16;//raise if you have a very noisy signal

static volatile const char MID_POINT = 127; //2.5V

static volatile unsigned int ticks = 0;

void reset(){//clean out some variables
	index = 0;//reset index
	noMatch = 0;//reset match couner
	maxSlope = 0;//reset slope
}

ISR(ADC_vect) {//when new ADC value ready
	++ticks;
	//return; /////////////////////////////////// COMMENT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	prevData = newData;//store previous value
	newData = ADCH;//get value from A0
	//USART_Transmit(newData);
	//USART_Transmit(' ');
	
	if (prevData < MID_POINT && newData >= MID_POINT){//if increasing and crossing midpoint
		newSlope = newData - prevData;//calculate slope
		//Serial.println(newSlope);
		if (abs(newSlope-maxSlope)<slopeTol){//if slopes are ==
			//record new data and reset time
			slope[index] = newSlope;
			timer[index] = time;
			time = 0;
			if (index == 0){//new max slope just reset
				//Serial.println("I AM HER1 ");
				noMatch = 0;
				index++;//increment index
			}
			else if (abs(timer[0]-timer[index])<timerTol && abs(slope[0]-newSlope)<slopeTol){//if timer duration and slopes match
				//sum timer values
				totalTimer = 0;
				for (unsigned char i=0;i<index;i++){
					totalTimer+=timer[i];
				}
				period = totalTimer;//set period
				if (checkMaxAmp > ampThreshold) {
					USART_Transmit_int((int)(FREQ_SAMPLING_RATE/float(period)*100.0));
					USART_Println();
				}
				periodReady = true;
				//reset new zero index values to compare with
				timer[0] = timer[index];
				slope[0] = slope[index];
				index = 1;//set index to 1
				noMatch = 0;
			}
			else{//crossing midpoint but not match
				index++;//increment index
				if (index > 9){
					reset();
				}
			}
		}
		else if (newSlope>maxSlope){//if new slope is much larger than max slope
			maxSlope = newSlope;
			time = 0;//reset clock
			noMatch = 0;
			index = 0;//reset index
		}
		else{//slope not steep enough
			noMatch++;//increment no match counter
			if (noMatch>9){
				reset();
			}
		}
	}
	
	if (newData == 0 || newData == 1023){//if clipping
		clipping = 1;//currently clipping
	}
	
	time++;//increment timer at rate of 38.5kHz
	
	ampTimer++;//increment amplitude timer
	char newMaxAmp = abs(MID_POINT-newData);
	if (newMaxAmp > maxAmp) {
		maxAmp = newMaxAmp;
	}
	if (ampTimer==1000) {
		ampTimer = 0;
		checkMaxAmp = maxAmp;
		maxAmp = 0;
	}
	
}


void checkClipping(){//manage clipping indication
	if (clipping){//if currently clipping
		clipping = 0;
	}
}
// --------------------------------------------------------------- END PHYSICS -----------------------------------------------------------------------------

// For normalizing huge and short deviations
const int LONG_FREQ_AR_LEN = 25;
double long_last_frequencies[LONG_FREQ_AR_LEN];
int long_freq_ar_i = 0;
const double FREQ_MAX_DIFF = 0.2f;

// For normalizing small short deviations
const int SHORT_FREQ_AR_LEN = 10;
double short_last_frequencies[SHORT_FREQ_AR_LEN];
int short_freq_ar_i = 0;

// Get average from double array
double get_av(double* ar, int len) {
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (ar > 0) {
			sum += ar[i];
		}
	}

	return sum/(double)len;
}

double calculateSD(double* ar, int len)
{
	double sum = 0.0, mean, standardDeviation = 0.0;

	int i;

	for(i = 0; i < len; ++i)
	{
		sum += ar[i];
	}

	mean = sum/len;

	for(i = 0; i < len; ++i)
	standardDeviation += pow(ar[i] - mean, 2);

	return sqrt(standardDeviation / len);
}

int main() {
	setup();
	USART_Init ( MYUBRR );
	USART_Transmit('a');
	while(1) {
		
		if (checkMaxAmp > ampThreshold) /* && checkMaxAmp < maxAmpThreshold) */ {
			if (periodReady) { // prevent working twice with the same reading
				periodReady = false;
				
				frequency = FREQ_SAMPLING_RATE/float(period);//calculate frequency timer rate/period
				
				//USART_Transmit_int((int)frequency);
				//USART_Println();
			
				if (isFreqLegal(frequency)) {
			
					// Ignore noise and big swings
					long_last_frequencies[long_freq_ar_i++] = frequency;
					if (long_freq_ar_i >= LONG_FREQ_AR_LEN) long_freq_ar_i = 0;
					float long_average_freq = get_av(long_last_frequencies, LONG_FREQ_AR_LEN);
					float diff = abs(long_average_freq - frequency);
					float max_diff = long_average_freq * FREQ_MAX_DIFF;

					if (diff < max_diff){
						// get average freq
						short_last_frequencies[short_freq_ar_i++] = frequency;
						if (short_freq_ar_i >= SHORT_FREQ_AR_LEN) short_freq_ar_i = 0;
					}
			
					float short_average_freq = get_av(short_last_frequencies, SHORT_FREQ_AR_LEN);
					getNoteByFreq(&currentNote, short_average_freq); // RECOGNIZE NOTE
					if (currentNote.valid) {
						//USART_Transmit_int((int)short_average_freq);
						//USART_Println();
						//USART_Println();
						displ.displayNote(&currentNote, short_average_freq); // DISPLAY NOTE
						//USART_Transmit(currentNote.note);
						//USART_Println();
					}
				
				
					//double sd = calculateSD(long_last_frequencies, LONG_FREQ_AR_LEN);
					//USART_Transmit_int((int)(sd*100.0));
					//USART_Transmit(' ');
					//USART_Transmit_int((int)(frequency*100.0));
					//USART_Transmit(' ');
					//USART_Transmit_int((int)(short_average_freq*100.0));
					//USART_Transmit(' ');
					//USART_Transmit_int((int)(period));
					//USART_Transmit(' ');
					//USART_Println();
				}
			}
		} 
		
		_delay_ms(5);
	}
	
	return 0;
}

//int main(void)
//{
	//setup();
	//USART_Init ( MYUBRR );
	//USART_Transmit('a');
	//
	//OCR2 = 62;
	//TCCR2 |= (1 << WGM21);
	//// Set to CTC Mode
	//TIMSK |= (1 << OCIE2);
	////Set interrupt on compare match
	//TCCR2 |= (1 << CS21);
	//// set prescaler to 64 and starts PWM
	//sei();
	//// enable interrupts
	//while (1);
	//{
		//// we have a working timer
	//}
//}
//
//static volatile int x = 0;
//ISR (TIMER2_COMP_vect)
//{
	//++x;
	//if (x >= 16000) {
		//USART_Transmit_unsigned_int(ticks);
		//USART_Println();
		//x = 0;
		//ticks = 0;
	//}
//}
