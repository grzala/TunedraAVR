#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>

#define F_CPU 8000000UL // 8 MHz clock speed

#include <util/delay.h>
#define DELAY _delay_ms


#include <serial.h>

// --------------------------------------------------------------- SETUP ---------------------------------------------------------------------------------


//Note* currentNote = NULL;
//Display* displ = NULL;
void setup() {
	DDRC = 0b00000000; // SET C0 to iput
	//Serial.begin(115200);
	
	//currentNote = new Note;
	
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
	USART_Transmit_ar(str1);
	
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
				char str2[] = "I AM HERE 2 \r\n";
				USART_Transmit_ar(str2);
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
		//USART_Transmit_ar(maxAmpStr);
		//char mastr[8];
		//sprintf(mastr, "%i", (int)checkMaxAmp);
		//USART_Transmit_ar(mastr);
		//
		//USART_Transmit(' ');
//
		//USART_Transmit_ar(voltageStr);
		//char vstr[8];
		//sprintf(vstr, "%i", newData);
		//USART_Transmit_ar(vstr);
		//
		//USART_Transmit(' ');
		//
		
		float voltage = newData * (5.0 / 1023.0);
		
		//PORTD = 0;
		if (checkMaxAmp > ampThreshold) /* && checkMaxAmp < maxAmpThreshold) */ {
			frequency = 38462.0/float(period);//calculate frequency timer rate/period
			
			

			//USART_Transmit_ar(freqStr);
			//char fstr[15];
			//sprintf(fstr, "%f", (double)period);
			//USART_Transmit_ar(fstr);
			
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
		
		USART_Transmit_ar(newLine);
		
		DELAY(10);
	}
	
	return 0;
}