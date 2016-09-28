#include <dev/leds.h>
#include "Arduino.h"
#include <avr/io.h>

//RED is set on PB0
//GREEN is set on PD7

/*---------------------------------------------------------------------------*/
void leds_arch_init(void)
{
  DDRB |= (LEDS_CONF_RED);// | LEDS_CONF_YELLOW);
  DDRD |= (LEDS_CONF_GREEN);
  PORTB |= (LEDS_CONF_RED);// | LEDS_CONF_YELLOW);
  PORTD |= (LEDS_CONF_GREEN);
}
/*---------------------------------------------------------------------------*/
unsigned char leds_arch_get(void)
{
  return ((PORTB & LEDS_CONF_RED) ? LEDS_RED : 0)
    | ((PORTD & LEDS_CONF_GREEN) ? LEDS_GREEN : 0);
    //| ((LEDS_PxOUT & LEDS_CONF_YELLOW) ? 0 : LEDS_YELLOW);
}
/*---------------------------------------------------------------------------*/
void leds_arch_set(unsigned char leds)
{
  PORTB = (PORTB & ~(LEDS_CONF_RED))
    |  ((leds & LEDS_RED) ? LEDS_CONF_RED : 0);

  PORTD = (PORTD & ~(LEDS_CONF_GREEN))
    |  ((leds & LEDS_GREEN) ? LEDS_CONF_GREEN : 0);
}
/*---------------------------------------------------------------------------*/
