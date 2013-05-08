/*
 * tc_pwm.h
 */

#ifndef TC_PWM_H_
#define TC_PWM_H_

void tc2pwm_init(unsigned char tc_channel, __int_handler handler, unsigned char ra_int, unsigned char rb_int);
void tc2pwm_set_freq(unsigned char tc_channel, unsigned int fcpu_hz, unsigned int freq);
void tc2pwm_set_duty(unsigned char tc_channel, unsigned char pwm, unsigned short duty);
void tc2pwm_reset_interrupt(unsigned char tc_channel);

#endif /* TC_PWM_H_ */
