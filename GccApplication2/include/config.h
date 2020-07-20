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
#define DISPLAY_PORT_OUTPUT PORTB
#define DISPLAY_PORT_CONFIG DDRB


#endif /* CONFIG_H_ */