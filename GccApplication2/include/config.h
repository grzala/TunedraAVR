/*
 * config.h
 *
 * Created: 20/07/2020 20:15:29
 *  Author: mpana
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_


#define F_CPU 8000000UL // 8 MHz clock speed

#define ws2812_port B   // Data port
#define ws2812_pin  1   // Data out pin
#define DISPLAY_PORT_LED_OUTPUT PORTB
#define DISPLAY_PORT_LED_CONFIG DDRB
#define DISPLAY_PORT_OUTPUT PORTD
#define DISPLAY_PORT_CONFIG DDRD

#define midPin_ 1
#define upPin_ 4
#define upRPin_ 5
#define downRPin_ 6
#define downPin_ 2
#define downLPin_ 0
#define upLPin_ 3
#define sharpPin_ 7


#endif /* CONFIG_H_ */