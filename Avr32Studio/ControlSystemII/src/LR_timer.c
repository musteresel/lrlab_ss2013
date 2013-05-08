/*
 * LR_timer.c
 */
//==============================================================================
/** Include directives */
#include "LR_timer.h"
#include "tc.h"
#include "intc.h"


//==============================================================================
/** Global data */
#define TC_CHANNELS 3
LR_timer_handler _handler[TC_CHANNELS];
unsigned short _period[TC_CHANNELS];
unsigned long _freq[TC_CHANNELS];
tc_waveform_opt_t wave_opt[TC_CHANNELS];
tc_interrupt_t int_opt[TC_CHANNELS];
volatile unsigned char STARTED = 0;



//==============================================================================
/** Init a channel of the timer counter to operate as timer.
 *
 * This function initializes a channel of the builtin timer counter to fire
 * an interrupt on RC compare event. f must denote the chip frequency, div_exp
 * defines the clock divider (2^div_exp)
 */
void LR_timer_init(unsigned char chan, unsigned long f)
{
  wave_opt[chan].channel = chan;
  wave_opt[chan].wavsel = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER;
  //wave_opt[chan].tcclks = (div_exp < 5) ? (div_exp) : (4);
  wave_opt[chan].tcclks = TC_CLOCK_SOURCE_TC2;// FPBA / 128
  _freq[chan] = f >> (wave_opt[chan].tcclks);
  // No software triggers
  wave_opt[chan].bswtrg = TC_EVT_EFFECT_NOOP;
  wave_opt[chan].aswtrg = TC_EVT_EFFECT_NOOP;
  // No compare events on a and b
  wave_opt[chan].acpa = TC_EVT_EFFECT_NOOP;
  wave_opt[chan].acpc = TC_EVT_EFFECT_NOOP;
  wave_opt[chan].bcpb = TC_EVT_EFFECT_NOOP;
  wave_opt[chan].bcpc = TC_EVT_EFFECT_NOOP;
  // No external events
  wave_opt[chan].aeevt = TC_EVT_EFFECT_NOOP;
  wave_opt[chan].beevt = TC_EVT_EFFECT_NOOP;
  wave_opt[chan].eevt = 1;
  wave_opt[chan].enetrg = FALSE;
  wave_opt[chan].eevtedg = TC_SEL_NO_EDGE;
  // more ...
  wave_opt[chan].cpcdis = FALSE;
  wave_opt[chan].cpcstop = FALSE;
  wave_opt[chan].burst = FALSE;
  wave_opt[chan].clki = FALSE;


  // We only need an interrupt on RC compare
  int_opt[chan].etrgs = 0;
  int_opt[chan].ldrbs = 0;
  int_opt[chan].ldras = 0;
  int_opt[chan].cpcs  = 1;
  int_opt[chan].cpbs  = 0;
  int_opt[chan].cpas  = 0;
  int_opt[chan].lovrs = 0;
  int_opt[chan].covfs = 0;

}




//==============================================================================
/** Set the interrupt flag for the channel to allow a new interrupt */
void LR_timer_sti(unsigned char chan)
{
  tc_read_sr(&AVR32_TC, chan);
}




//==============================================================================
/** Assign a handler with a period (in milliseconds) to a channel */
void LR_timer_assign(unsigned char chan,
                     LR_timer_handler handler,
                     unsigned short period)
{
  _handler[chan] = handler;
  _period[chan] = period;
}



//==============================================================================
/** Start the channel, only if a valid handler and period are given */
void LR_timer_start(unsigned char chan)
{
  if (chan < TC_CHANNELS && _handler[chan] != 0 && _period[chan] > 0)
  {
    Disable_global_interrupt();
    INTC_register_interrupt(_handler[chan],
                            AVR32_TC_IRQ0 + chan,
                            AVR32_INTC_INT0);
    Enable_global_interrupt();
    tc_init_waveform(&AVR32_TC, &(wave_opt[chan]));
    /*char string[20];
    sprintf(string,"%d\n",(unsigned short)((_freq[chan] * _period[chan])/1000));
    dip204_write_string(string);*/
    tc_write_rc(&AVR32_TC, chan, (unsigned short)((_freq[chan] * _period[chan])/1000));
    tc_configure_interrupts(&AVR32_TC, chan, &(int_opt[chan]));
    tc_start(&AVR32_TC, chan);
    STARTED |= (1 << chan);
  }
}


//==============================================================================
/** Stop channel */
void LR_timer_stop(unsigned char chan)
{
  tc_stop(&AVR32_TC, chan);
  STARTED &= ~(1 << chan);
}


//==============================================================================
/** Wait channel */
void LR_timer_wait(unsigned char chan)
{
  while ((STARTED & (1 << chan))) {};
}


