#ifndef UART_H
#define UART_H

#include "system.h"
#include <avr/io.h>
#include <stdio.h>

// UART configuration
// Asynchronous normal mode: UBRR = F_CPU/(16*BAUD) - 1 (rounded integer division, add half LSB)
constexpr uint16_t UBRR_VALUE = ((F_CPU + 8UL * BAUD) / (16UL * BAUD) - 1UL);
constexpr uint8_t UBRRL_VALUE = (UBRR_VALUE & 0xff);
constexpr uint8_t UBRRH_VALUE = (UBRR_VALUE >> 8);
// 8-bit data, no parity, 1 stop bit, enable RX and TX
constexpr uint8_t UCSR0C_VALUE = (1 << UCSZ01) | (1 << UCSZ00);
constexpr uint8_t UCSR0B_VALUE = (1 << TXEN0) | (1 << RXEN0);

void uart_init();
int uart_putchar(char, FILE *);
int uart_getchar(FILE *);

FILE uart_stream = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

void uart_init(void)
{
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
  UCSR0C = UCSR0C_VALUE;
  UCSR0B = UCSR0B_VALUE;
}

// Send a single character over UART
int uart_putchar(char c, FILE *stream)
{
  if (c == '\n')
  {
    uart_putchar('\r', stream);
  }
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
  return 0;
}

// Receive a single character from UART
int uart_getchar(FILE *stream)
{
  // Wait until RX buffer has data
  loop_until_bit_is_set(UCSR0A, RXC0);
  return UDR0;
}

#ifdef DEBUG
#define dbgprint(...)                      \
  {                                        \
    char buf[128];                         \
    printf(buf, sizeof(buf), __VA_ARGS__); \
  }
#else
#define dbgprint(...) \
  {                   \
  }
#endif

#endif
