/*
 * pwm_pwm.h
 */

#ifndef PWM_PWM_H_
#define PWM_PWM_H_

#include "pwm.h"

typedef struct ___pwm_pwm_chan
{
	avr32_pwm_channel_t * chan;
	unsigned int fosc;
	unsigned long cprd;
}pwm_pwm_chan;

void pwm_pwm_update_freq(pwm_pwm_chan * pwm, double freq);
void pwm_pwm_update_duty(pwm_pwm_chan * pwm, double duty);
void pwm_pwm_init();
void pwm_pwm_configure_channel(pwm_pwm_chan * pwm, double freq, double duty, unsigned int fosc);

#endif /* PWM_PWM_H_ */
