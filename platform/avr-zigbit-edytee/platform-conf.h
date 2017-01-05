/**
 * \file
 *         Platform configuration of the Zigbit.
 * \author
 *         Vinícius Galvão <vinicius_galvao@msn.com>
 */

#ifndef PLATFORM_CONF_H_
#define PLATFORM_CONF_H_

/* LED ports */
#define LEDS_PxDIR DDRB // port direction register
#define LEDS_PxOUT PORTB // port register
#define LEDS_CONF_RED    (PB5) //red led
#define LEDS_CONF_GREEN  (PB6) // green led
#define LEDS_CONF_YELLOW (PB7) // yellow led

#endif /* PLATFORM_CONF_H_ */
