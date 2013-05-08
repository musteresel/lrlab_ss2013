#ifndef LR_PWM_H_
#define LR_PWM_H_


#include "pwm.h"

typedef struct
{
        avr32_pwm_channel_t * chan;
        unsigned int fosc;
        unsigned long cprd;
}LR_pwm;

void LR_pwm_init();
void LR_pwm_start(LR_pwm * pwm, double freq, double duty, unsigned int fcpu);
void LR_pwm_stop(LR_pwm * pwm);
void LR_pwm_update_duty(LR_pwm * pwm, double duty);


#endif /* LR_PWM_H_ */
