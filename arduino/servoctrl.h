// provide basic servo control functions
// control the lift-off servo using Timer1 PWM output on OC1A (ATMEGA328P PB1, 28SPDIP Pin 15, UNO R3 Pin 9)
// servo shall have a pwm period of approx 20ms and pulse width in the range of 1ms to 2ms in 100us steps

#ifndef servoctrl_h
#define servoctrl_h

#include "system.h"
#include "avr/interrupt.h"
#include <stdint.h>

#include "dbg_servoctrl.h"

// configuration
// SERVOCTRL is fixed to Pin PB1
#define SERVOCTRL_INVERTSIGNAL 0
#define SERVOCTRL_PWMPERIOD_S 0.02
#define SERVOCTRL_ISR_PRESCALER 256
#define SERVOCTRL_PULSE_WIDTH_S_DOWN 0.0012
#define SERVOCTRL_PULSE_WIDTH_S_UP 0.0019
#define SERVOCTRL_RAMP_STEPS 10

constexpr int16_t SERVOCTRL_ISR_TICKS_PERIOD = static_cast<int16_t>((F_CPU / SERVOCTRL_ISR_PRESCALER * SERVOCTRL_PWMPERIOD_S) - 1);
constexpr int16_t SERVOCTRL_ISR_TICKS_DOWN = static_cast<int16_t>(F_CPU / SERVOCTRL_ISR_PRESCALER * SERVOCTRL_PULSE_WIDTH_S_DOWN);
constexpr int16_t SERVOCTRL_ISR_TICKS_UP = static_cast<int16_t>(F_CPU / SERVOCTRL_ISR_PRESCALER * SERVOCTRL_PULSE_WIDTH_S_UP);
constexpr int8_t SERVOCTRL_ISR_RAMP_LIFT = static_cast<int8_t>((SERVOCTRL_ISR_TICKS_UP - SERVOCTRL_ISR_TICKS_DOWN) / SERVOCTRL_RAMP_STEPS);
constexpr int8_t SERVOCTRL_ISR_RAMP_LOWER = static_cast<int8_t>((SERVOCTRL_ISR_TICKS_DOWN - SERVOCTRL_ISR_TICKS_UP) / SERVOCTRL_RAMP_STEPS);
constexpr bool SERVOCTRL_ISR_RAMPEDDOWN(const int16_t x) { return ((SERVOCTRL_ISR_TICKS_DOWN < SERVOCTRL_ISR_TICKS_UP) ? (x <= SERVOCTRL_ISR_TICKS_DOWN) : (x >= SERVOCTRL_ISR_TICKS_DOWN)); };
constexpr bool SERVOCTRL_ISR_RAMPEDUP(const int16_t x) { return ((SERVOCTRL_ISR_TICKS_DOWN < SERVOCTRL_ISR_TICKS_UP) ? (x >= SERVOCTRL_ISR_TICKS_UP) : (x <= SERVOCTRL_ISR_TICKS_UP)); };

static_assert(F_CPU > 0);
static_assert((SERVOCTRL_INVERTSIGNAL == 0) || (SERVOCTRL_INVERTSIGNAL == 1));
static_assert((SERVOCTRL_ISR_PRESCALER == 1) || (SERVOCTRL_ISR_PRESCALER == 8) || (SERVOCTRL_ISR_PRESCALER == 64) || (SERVOCTRL_ISR_PRESCALER == 256) || (SERVOCTRL_ISR_PRESCALER == 1024));
static_assert(SERVOCTRL_PWMPERIOD_S > 0);
static_assert((SERVOCTRL_PULSE_WIDTH_S_DOWN > 0) && (SERVOCTRL_PULSE_WIDTH_S_DOWN < SERVOCTRL_PWMPERIOD_S));
static_assert((SERVOCTRL_PULSE_WIDTH_S_UP > 0) && (SERVOCTRL_PULSE_WIDTH_S_UP < SERVOCTRL_PWMPERIOD_S));
static_assert(SERVOCTRL_RAMP_STEPS > 0);
static_assert(F_CPU / SERVOCTRL_ISR_PRESCALER * SERVOCTRL_PWMPERIOD_S < 32768, "pwm period too long for prescaler");
static_assert(((SERVOCTRL_ISR_TICKS_UP - SERVOCTRL_ISR_TICKS_DOWN) / SERVOCTRL_RAMP_STEPS > -128) && ((SERVOCTRL_ISR_TICKS_UP - SERVOCTRL_ISR_TICKS_DOWN) / SERVOCTRL_RAMP_STEPS < 128), "ramp too steep");
static_assert(SERVOCTRL_ISR_RAMP_LIFT != 0, "ramp too flat");

class ServoCtrl
{
public:
  static void init();
  static bool lower();
  static bool lift();
  static bool is_down();
  static bool is_up();
};

/// @brief state enum for servo control state maschine
typedef enum
{
  SERVOCTRL_OFF,
  SERVOCTRL_LOWERING,
  SERVOCTRL_DOWN,
  SERVOCTRL_LIFTING,
  SERVOCTRL_UP
} servoctrl_state_t;

/// @brief state variable for the servo control state machine, should be modified only within the ISR and the ServoCtrl methods
volatile servoctrl_state_t servoctrl_state{SERVOCTRL_OFF};
/// @brief servo pwm value, should be modified only within the ISR
volatile int16_t servoctrl_pwm{SERVOCTRL_ISR_TICKS_UP};

/// @brief interrupt service routine for Timer1 Compare Match A, used to ramp the pulse width up and down for smooth lifting and lowering of the pen
ISR(TIMER1_COMPA_vect)
{
  if (servoctrl_state == SERVOCTRL_LOWERING)
  {
    // ramp the pulse width in steps until the ramped down pulse width is reached
    servoctrl_pwm += SERVOCTRL_ISR_RAMP_LOWER;
    if (SERVOCTRL_ISR_RAMPEDDOWN(servoctrl_pwm))
    {
      servoctrl_state = SERVOCTRL_DOWN;
      servoctrl_pwm = SERVOCTRL_ISR_TICKS_DOWN;
    }
    OCR1A = static_cast<uint16_t>(servoctrl_pwm);
  }
  else if (servoctrl_state == SERVOCTRL_LIFTING)
  {
    // ramp the pulse width in steps until the ramped up pulse width is reached
    servoctrl_pwm += SERVOCTRL_ISR_RAMP_LIFT;
    if (SERVOCTRL_ISR_RAMPEDUP(servoctrl_pwm))
    {
      servoctrl_state = SERVOCTRL_UP;
      servoctrl_pwm = SERVOCTRL_ISR_TICKS_UP;
    }
    OCR1A = static_cast<uint16_t>(servoctrl_pwm);
  }
}

/// @brief prepare the servo control by configuring 16 bit Timer1
void ServoCtrl::init()
{
  // Waveform Generation Mode (WGM13:10) = 14 (fast PWM, TOP = ICR1, update OCR1A at BOTTOM, TOV1 flag set on TOP)
  // Clock Select (CS12:10) = 4 (use prescaler 256 -> 16MHz/256 = 62.5kHz timer clock)
  // ICR1 = 62500/50 - 1 = 1249 (20ms period)
  // COM1A1=1, COM1A0=0: clear OC1A on compare match, set at BOTTOM (active high pulse)
  // COM1A1=1, COM1A0=1: set OC1A on compare match, clear at BOTTOM (active low pulse)
  constexpr uint8_t CS1_VALUE = ((((SERVOCTRL_ISR_PRESCALER == 256) || (SERVOCTRL_ISR_PRESCALER == 1024)) ? 1 : 0) << CS12) |
                                ((((SERVOCTRL_ISR_PRESCALER == 8) || (SERVOCTRL_ISR_PRESCALER == 64)) ? 1 : 0) << CS11) |
                                ((((SERVOCTRL_ISR_PRESCALER == 1) || (SERVOCTRL_ISR_PRESCALER == 64) || (SERVOCTRL_ISR_PRESCALER == 1024)) ? 1 : 0) << CS10);
  constexpr uint8_t TCCR1B_INIT = (1 << WGM13) | (1 << WGM12) | CS1_VALUE;
  constexpr uint8_t TCCR1A_INIT = (1 << WGM11) | (1 << COM1A1) | (SERVOCTRL_INVERTSIGNAL << COM1A0);
  constexpr uint8_t TIMSK1_INIT = (1 << OCIE1A);
  constexpr uint8_t PB1_MASK = (1 << PB1);
  constexpr uint8_t DDB1_MASK = (1 << DDB1);

  TCCR1B = TCCR1B_INIT;
  TCCR1A = TCCR1A_INIT;
  DDRB |= DDB1_MASK;

  servoctrl_pwm = SERVOCTRL_ISR_TICKS_UP;
  OCR1A = static_cast<uint16_t>(servoctrl_pwm);
  ICR1 = static_cast<uint16_t>(SERVOCTRL_ISR_TICKS_PERIOD);
  ICR1 = static_cast<uint16_t>(SERVOCTRL_ISR_TICKS_PERIOD);
  servoctrl_state = SERVOCTRL_UP;
  TIMSK1 |= TIMSK1_INIT;
}

/// @brief start the pen lowering process
/// @return success of starting the lowering process
bool ServoCtrl::lower()
{
  if (servoctrl_state == SERVOCTRL_UP)
  {
    servoctrl_state = SERVOCTRL_LOWERING;
    return true;
  }
  else
  {
    return false;
  }
}

/// @brief start the pen lifting process
/// @return success of starting the lifting process
bool ServoCtrl::lift()
{
  if (servoctrl_state == SERVOCTRL_DOWN)
  {
    servoctrl_state = SERVOCTRL_LIFTING;
    return true;
  }
  else
  {
    return false;
  }
}

/// @brief check if the pen is in the down position
bool ServoCtrl::is_down()
{
  return (servoctrl_state == SERVOCTRL_DOWN);
}

/// @brief check if the pen is in the up position
bool ServoCtrl::is_up()
{
  return (servoctrl_state == SERVOCTRL_UP);
}

#endif
