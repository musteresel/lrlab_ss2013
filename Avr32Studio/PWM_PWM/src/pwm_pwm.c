/*
 * pwm_pwm.c
 *
 */

#include "pwm_pwm.h"



#include "stdio.h"
#include "display.h"
#include "delay.h"


void pwm_pwm_update_freq(pwm_pwm_chan * pwm, double freq)
{
	pwm->chan->CMR.cpd = PWM_UPDATE_PERIOD;
	pwm->cprd = (unsigned long)((double)pwm->fosc / freq);
	pwm->chan->cupd = pwm->cprd;
	pwm_async_update_channel (pwm->chan->ccnt, (pwm->chan));
}

void pwm_pwm_update_duty(pwm_pwm_chan * pwm, double duty)
{
	pwm->chan->CMR.cpd = PWM_UPDATE_DUTY;
	pwm->chan->cupd = duty * pwm->cprd;
	pwm_async_update_channel (pwm->chan->ccnt, (pwm->chan));
}

pwm_opt_t pwm_opt;

void pwm_pwm_init()
{

	pwm_opt.diva = AVR32_PWM_DIVA_CLK_OFF;
	pwm_opt.divb = AVR32_PWM_DIVB_CLK_OFF;
	pwm_opt.prea = AVR32_PWM_PREA_MCK;
	pwm_opt.preb = AVR32_PWM_PREB_MCK; // NOTE .. on stack?!?
	pwm_init(&pwm_opt);
}





void pwm_pwm_configure_channel(pwm_pwm_chan * pwm, double freq, double duty, unsigned int fosc)
{
	//pwm->chan.ccnt = num;
	pwm->chan->CMR.calg = PWM_MODE_LEFT_ALIGNED; // Channel mode.
	pwm->chan->CMR.cpol = PWM_POLARITY_HIGH; // Channel polarity.
	pwm->chan->CMR.cpre = AVR32_PWM_CPRE_MCK; // Channel prescaler.

	pwm->fosc = fosc;

	pwm->chan->cprd = (unsigned long)((double)pwm->fosc/freq); // Channel period.
	pwm->chan->cdty = duty * pwm->chan->cprd; // Channel duty cycle, 0 <= pwm_channel.cdty <= pwm_channel.cprd
	// freq = fosc/p
	// p = fosc/freq
	pwm_channel_init(pwm->chan->ccnt, (pwm->chan)); // Set channel configuration to channel
	pwm_start_channels(1 << pwm->chan->ccnt); // Start channel
}
