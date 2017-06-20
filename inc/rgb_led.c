/*
 * rgb_led.c
 *
 *  Created on: Mar 24, 2017
 *      Author: waseemh
 */

#include "rgb_led.h"
	/*
	 * private functions
	 */
void generate_pulses(uint8_t n_pulses){
	int 	loop_var=0;
	for(loop_var=1;loop_var<n_pulses;loop_var++){
		GPIO_PinOutSet(LED_PORT, LEDEN);
		pwm_us_delay(LTC_HIGH_TIME);
		GPIO_PinOutClear(LED_PORT, LEDEN);
		pwm_us_delay(LTC_LOW_TIME);
	}
	GPIO_PinOutSet(LED_PORT, LEDEN);
	return;
}
	/*
	 * public functions
	 */
void rgb_init(void){
	CMU_ClockEnable(cmuClock_GPIO, true);
	GPIO_PinModeSet(LED_PORT, LEDEN, gpioModePushPull, 0);
	GPIO_DriveModeSet(LED_PORT, gpioDriveModeLowest);
	GPIO_PinOutClear(LED_PORT, LEDEN);
	pwm_init();
	return;
}
void rgb_reinit(void){										//NOT used
	pwm_init();
	return;
}
void rgb_on(bool red, bool green, bool blue){
	rgb_shutdown();
	if (red== true && blue==true && green==true){			//white mode
		generate_pulses(7);
	}
	else if(red== true && blue==false && green==false){
		generate_pulses(4);
	}
	else if(red== false && blue==true && green==false){
		generate_pulses(1);
	}
	else if(red== false && blue==false && green==true){
		generate_pulses(2);
	}
	else if(red== true && blue==true && green==false){
		generate_pulses(5);
	}
	else if(red== true && blue==false && green==true){
		generate_pulses(6);
	}
	else if(red== false && blue==true && green==true){
		generate_pulses(3);
	}
	else{													//shutdown condition...
		rgb_shutdown();
		//return;
	}
	return;
}

void rgb_shutdown(void){
	GPIO_PinOutClear(LED_PORT, LEDEN);
	pwm_us_delay(1000);	//LTC3212 off time...
	return;
}
