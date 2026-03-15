#ifndef dbg_simave_h
#define dbg_simave_h

#ifdef SIMAVR
#include "system.h"
#include "avr/io.h"
#include "avr/interrupt.h"
#include "simavr/avr_mcu_section.h"
AVR_MCU(F_CPU, MCU);
#endif

#endif