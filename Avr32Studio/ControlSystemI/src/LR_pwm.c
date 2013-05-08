/*
 * LR_pwm.c
 *
 */
//==============================================================================
/** Include directives */
#include "LR_pwm.h"



//==============================================================================
/** Global data */
pwm_opt_t pwm_opt;




//==============================================================================
/** Init the pwm module.
 *
 * This function is sed to init the pwm module of the board. It sets
 * prescalers etc.
 */
void LR_pwm_init()
{
  pwm_opt.diva = AVR32_PWM_DIVA_CLK_OFF;
  pwm_opt.divb = AVR32_PWM_DIVB_CLK_OFF;
  pwm_opt.prea = AVR32_PWM_PREA_MCK;
  pwm_opt.preb = AVR32_PWM_PREB_MCK; // NOTE .. on stack?!?
  pwm_init(&pwm_opt);
}




//==============================================================================
/** Configure and start a channel.
 *
 * This function is used to set up a pwm channel and starts it thereafter.
 */
#include "LR_usart.h"
void LR_pwm_start(LR_pwm * pwm, double freq, double duty, unsigned int fcpu)
{
  pwm->chan->CMR.calg = PWM_MODE_LEFT_ALIGNED;
  pwm->chan->CMR.cpol = PWM_POLARITY_HIGH;
  pwm->chan->CMR.cpre = AVR32_PWM_CPRE_MCK;
  pwm->fosc = fcpu;
  pwm->chan->cprd = (unsigned long)((double)pwm->fosc / freq);
  pwm->cprd = pwm->chan->cprd;
  pwm->chan->cdty = duty * pwm->chan->cprd;

  char string[80];
  sprintf(string,"FreqReg=%lu\nDutyReg=%lu\n",pwm->chan->cprd, pwm->chan->cdty);
  LR_usart_write(string);

  pwm_channel_init(pwm->chan->ccnt, pwm->chan);
  pwm_start_channels(1 << pwm->chan->ccnt);
}




//==============================================================================
/** Stop a channel */
void LR_pwm_stop(LR_pwm * pwm)
{
  pwm_stop_channels(1 << pwm->chan->ccnt);
}




//==============================================================================
/** Update duty cycle of a channel */
void LR_pwm_update_duty(LR_pwm * pwm, double duty)
{
  pwm->chan->CMR.cpd = PWM_UPDATE_DUTY;
  pwm->chan->cupd = duty * pwm->cprd;
  pwm_async_update_channel(pwm->chan->ccnt, pwm->chan);
}



/*
void LR_pwm_update_freq(LR_pwm * pwm, double freq)
{
  pwm->chan->CMR.cpd = PWM_UPDATE_PERIOD;
  pwm->cprd = (unsigned long)((double)pwm->fosc / freq);
  pwm->chan->cupd = pwm->cprd;
  pwm_async_update_channel (pwm->chan->ccnt, (pwm->chan));
}
*/





