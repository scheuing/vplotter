#ifndef system_h
#define system_h

// default configuration, potentially overwritten by Makefile
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUD
#define BAUD 9600
#endif

#ifndef MCU
#define MCU "atmega328p"
#endif

#include "avr/io.h"

#endif