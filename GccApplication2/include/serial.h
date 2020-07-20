#ifndef SERIAL_H
#define SERIAL_H

#include <math.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#define FOSC 8000000UL// Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#include <stdlib.h>
#include <avr/io.h>

void USART_Init (unsigned int ubrr);
void USART_Transmit (unsigned char data);
void USART_Transmit_ar (char* data);
void USART_Transmit_ar_ln (char* data);
void USART_Transmit_int (int data);

#endif // SERIAL_H