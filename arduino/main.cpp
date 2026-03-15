#include "uart.h"
#include "pos2len.h"
#include "servoctrl.h"
#include <stdint.h>

int main(void)
{
  uart_init();
  stdout = &uart_stream;
  stdin = &uart_stream;

  const float L{1200.0F};
  const float x{100.0F};
  const float y{100.0F};
  const float R{10.0F};
  const float q{1000000.0F};

  float u;
  float v;
  pos2leni(x, y, L, u, v);
  printf("ideal: u=%3.2f, v=%3.2f\n", u, v);

  pos2lenr(x, y, L, R, u, v);
  printf("radius: u=%3.2f, v=%3.2f\n", u, v);

  while (1)
  {
    pos2len(x, y, L, R, q, u, v);
    printf("catenary: u=%3.2f, v=%3.2f\n", u, v);
    getc(stdin);
  }

  return 0;
};
