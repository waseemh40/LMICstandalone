/*
 * rgb_led.h
 *
 *  Created on: Mar 27, 2017
 *      Author: waseemh
 */

#ifndef SRC_RGB_LED_H_
#define SRC_RGB_LED_H_

#include "pwm.h"
#include "delay.h"
#include "rs232.h"

#define 	LTC_HIGH_TIME		80
#define 	LTC_LOW_TIME		15
	/*
	 *private functions
	 */
void	 generate_pulses(uint8_t n_pulses);
	/*
	 * public functions
	 */
void 	rgb_init(void);
void 	rgb_reinit(void);									//NOT used
void 	rgb_on(bool red, bool blue, bool green);
void 	rgb_shutdown(void);


#endif /* SRC_RGB_LED_H_ */
