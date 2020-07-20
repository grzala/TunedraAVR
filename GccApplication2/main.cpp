#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>

#define F_CPU 8000000UL // 8 MHz clock speed

#include <util/delay.h>
#define DELAY _delay_ms
#include <avr/interrupt.h>

extern "C" { 
	#include "light_ws2812.h" 
};
#include "freqDetect.h"


#include <serial.h>

// --------------------------------------------------------------- SETUP ---------------------------------------------------------------------------------


Note currentNote;
//Display* displ = NULL;
void setup() {
	DDRC = 0b00000000; // SET C0 to iput
	//Serial.begin(115200);
	
	// (mid, up, upright, downright, down, leftdown, rightdown, sharp, red0, red1, green, red2, red3)
	//displ = new Display(3, 19, 18, 17, 7, 4, 2, 16, 6, 9, 5, 10, 11);
	
	cli();//diable interrupts
	
	//set up continuous sampling of analog pin 0 at 38.5kHz
	
	//clear ADCSRA register
	ADCSRA = 0;
	
	// 0b01100000;
	ADMUX |= (1 << REFS0); //set reference voltage
	ADMUX |= (1 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only
	
	// 0b11101100
	ADCSRA |= (1 << ADPS2); //set ADC clock with 32 prescaler- 8mHz/16=500kHz
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

int vala = -1;

void reset(){//clean out some variables
	index = 0;//reset index
	noMatch = 0;//reset match couner
	maxSlope = 0;//reset slope
}

ISR(ADC_vect) {//when new ADC value ready
	//return; /////////////////////////////////// COMMENT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	prevData = newData;//store previous value
	newData = ADCH;//get value from A0
	//
	//  Serial.print("oldData: ");
	//  Serial.print(prevData);
	//  Serial.print("newData: ");
	//  Serial.println(newData);
	char str1[] = "I AM HERE 1 \r\n";
	char str2[] = "I AM HERE 2 \r\n";
	char str3[] = "I AM HERE 2 \r\n";
	
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
	
	//PORTD = 0;
	//if (checkMaxAmp > ampThreshold) /* && checkMaxAmp < maxAmpThreshold) */ {
		//frequency = 38462.0/float(period);//calculate frequency timer rate/period
		//if (frequency > 40) {
			//PORTD |= (1 << 0);
		//}
		//if (frequency > 50) {
			//PORTD |= (1 << 1);
		//}
		//if (frequency > 170) {
			//PORTD |= (1 << 2);
		//}
		//if (frequency > 190) {
			//PORTD |= (1 << 3);
		//}
	//}
}


void checkClipping(){//manage clipping indication
	if (clipping){//if currently clipping
		clipping = 0;
	}
}
// --------------------------------------------------------------- END PHYSICS -----------------------------------------------------------------------------

// For normalizing huge and short deviations
const int LONG_FREQ_AR_LEN = 60;
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


int main() {
	//DDRD = 0xFF;
	//PORTD = 0x00;
	
	setup();
	
	char newLine[] = "\r\n";
	char maxAmpStr[] = "MaxAmp: ";
	char voltageStr[] = "New Data: ";
	char freqStr[] = "Frequency: ";
	
	USART_Init ( MYUBRR );
	while(1) {
		float voltage = newData * (5.0 / 1023.0);
		
		//PORTD = 0;
		if (checkMaxAmp > ampThreshold) /* && checkMaxAmp < maxAmpThreshold) */ {
			frequency = 38462.0/float(period);//calculate frequency timer rate/period
			
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
				float short_average_freq = get_av(short_last_frequencies, SHORT_FREQ_AR_LEN);

				getNoteByFreq(&currentNote, short_average_freq); // RECOGNIZE NOTE
				if (currentNote.valid) {
					//printFreqNote(short_average_freq, currentNote);
					//displ->displayNote(currentNote, short_average_freq); // DISPLAY NOTE
					
					USART_Transmit(currentNote.note);
					USART_Transmit_ar(newLine);
						  
				}
			}
			
			//USART_Transmit_ar(str, strlen(str));
	
			//if (frequency > 40) {
				//PORTD |= (1 << 0);
			//}
			//if (frequency > 50) {
				//PORTD |= (1 << 1);
			//}
			//if (frequency > 70) {
				//PORTD |= (1 << 2);
			//}
			//if (frequency > 900) {
				//PORTD |= (1 << 3);
			//}
			
			
		}
		
		DELAY(10);
	}
	
	return 0;
}

//#define MAXPIX 30
//#define COLORLENGTH (MAXPIX/2)
//#define FADE (256/COLORLENGTH)
//
//struct cRGB colors[8];
//struct cRGB led[MAXPIX];
//
//#define fullLedPower 120
//#define smallLedPower 55
//
//int main(void)
//{
	//
	//uint8_t j = 1;
	//uint8_t k = 1;
//
	//DDRB|=_BV(ws2812_pin);
	//
	//uint8_t i;
	//for(i=MAXPIX; i>0; i--)
	//{
		//led[i-1].r=0;led[i-1].g=0;led[i-1].b=0;
	//}
	//
	////Rainbowcolors
	//colors[0].r=150; colors[0].g=150; colors[0].b=150;
	//colors[1].r=255; colors[1].g=000; colors[1].b=000;//red
	//colors[2].r=255; colors[2].g=100; colors[2].b=000;//orange
	//colors[3].r=100; colors[3].g=255; colors[3].b=000;//yellow
	//colors[4].r=000; colors[4].g=255; colors[4].b=000;//green
	//colors[5].r=000; colors[5].g=100; colors[5].b=255;//light blue (türkis)
	//colors[6].r=000; colors[6].g=000; colors[6].b=255;//blue
	//colors[7].r=100; colors[7].g=000; colors[7].b=255;//violet
	//
	//while(1)
	//{
		////shift all vallues by one led
		//uint8_t i=0;
		//for(i=MAXPIX; i>1; i--)
		//led[i-1]=led[i-2];
		////change colour when colourlength is reached
		//if(k>COLORLENGTH)
		//{
			//j++;
			//if(j>7)
			//{
				//j=0;
			//}
//
			//k=0;
		//}
		//k++;
		////loop colouers
		//
		////fade red
		//if(led[0].r<(colors[j].r-FADE))
		//led[0].r+=FADE;
		//
		//if(led[0].r>(colors[j].r+FADE))
		//led[0].r-=FADE;
//
		//if(led[0].g<(colors[j].g-FADE))
		//led[0].g+=FADE;
		//
		//if(led[0].g>(colors[j].g+FADE))
		//led[0].g-=FADE;
//
		//if(led[0].b<(colors[j].b-FADE))
		//led[0].b+=FADE;
		//
		//if(led[0].b>(colors[j].b+FADE))
		//led[0].b-=FADE;
//
		//_delay_ms(10);
		//ws2812_sendarray((uint8_t *)led,MAXPIX*3);
	//}
//}