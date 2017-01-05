/**
 * \file
 *         LED architecture of the Zigbit.
 * \author
 *         Vinícius Galvão <vinicius_galvao@msn.com>
 */

#include "contiki-conf.h"
#include "dev/leds.h"
#include <avr/io.h>


/*---------------------------------------------------------------------------*/
void leds_arch_init(void)
{
  LEDS_PxDIR |= (LEDS_CONF_RED | LEDS_CONF_GREEN | LEDS_CONF_YELLOW);
  LEDS_PxOUT |= (LEDS_CONF_RED | LEDS_CONF_GREEN | LEDS_CONF_YELLOW);
}
/*---------------------------------------------------------------------------*/
unsigned char leds_arch_get(void)
{
  return ((LEDS_PxOUT & LEDS_CONF_RED) ? 0 : LEDS_RED)
    | ((LEDS_PxOUT & LEDS_CONF_GREEN) ? 0 : LEDS_GREEN)
    | ((LEDS_PxOUT & LEDS_CONF_YELLOW) ? 0 : LEDS_YELLOW);
}
/*---------------------------------------------------------------------------*/
void leds_arch_set(unsigned char leds)
{
  LEDS_PxOUT = (LEDS_PxOUT & ~(LEDS_CONF_RED|LEDS_CONF_GREEN|LEDS_CONF_YELLOW))
    | ((leds & LEDS_RED) ? 0 : LEDS_CONF_RED)
    | ((leds & LEDS_GREEN) ? 0 : LEDS_CONF_GREEN)
    | ((leds & LEDS_YELLOW) ? 0 : LEDS_CONF_YELLOW);
}
/*---------------------------------------------------------------------------*/
