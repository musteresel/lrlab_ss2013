#include "intc.h"
#include "compiler.h"
#include "board.h"
#include "tc.h"

tc_waveform_opt_t wave_opt;
tc_interrupt_t int_opt;
unsigned short __rc;

void tc2pwm_init(unsigned char tc_channel, __int_handler rc_handler, unsigned char ra_int, unsigned char rb_int)
{
  wave_opt.channel    = tc_channel;
  wave_opt.bswtrg   = TC_EVT_EFFECT_NOOP;
  wave_opt.beevt    = TC_EVT_EFFECT_NOOP;
  wave_opt.bcpc     = TC_EVT_EFFECT_NOOP;                // RC compare effect on TIOB.
  wave_opt.bcpb     = TC_EVT_EFFECT_NOOP;              // RB compare effect on TIOB.
  wave_opt.aswtrg   = TC_EVT_EFFECT_NOOP;                // Software trigger effect on TIOA.
  wave_opt.aeevt    = TC_EVT_EFFECT_NOOP;                // External event effect on TIOA.
  wave_opt.acpc     = TC_EVT_EFFECT_CLEAR;
  wave_opt.acpa     = TC_EVT_EFFECT_SET;              // RA compare effect on TIOA: toggle (other possibilities are none, set and clear).
  wave_opt.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER;// Waveform selection: Up mode with automatic trigger(reset) on RC compare.
  wave_opt.enetrg   = FALSE;                             // External event trigger enable.
  wave_opt.eevt     = 1;                                 // External event selection.
  wave_opt.eevtedg  = TC_SEL_NO_EDGE;                    // External event edge selection.
  wave_opt.cpcdis   = FALSE;                             // Counter disable when RC compare.
  wave_opt.cpcstop  = FALSE;                             // Counter clock stopped with RC compare.
  wave_opt.burst    = FALSE;                             // Burst signal selection.
  wave_opt.clki     = FALSE;                             // Clock inversion.
  wave_opt.tcclks   = TC_CLOCK_SOURCE_TC3;                // Internal source clock 3, connected to fPBA / 8.

  int_opt.etrgs = 0;
  int_opt.ldrbs = 0;
  int_opt.ldras = 0;
  int_opt.cpcs  = 1;
  int_opt.cpbs  = 1;//(rb_int) ? 1 : 0,
  int_opt.cpas  = 0; //(ra_int) ? 1 : 0,
  int_opt.lovrs = 0;
  int_opt.covfs = 0;

  Disable_global_interrupt();
  INTC_register_interrupt(rc_handler, AVR32_TC_IRQ0 + tc_channel, AVR32_INTC_INT0);
  Enable_global_interrupt();

  tc_init_waveform(&AVR32_TC, &wave_opt);
  tc_write_rc(&AVR32_TC, tc_channel, 12348);
  tc_configure_interrupts(&AVR32_TC, tc_channel, &int_opt);
  tc_start(&AVR32_TC, tc_channel);
}

void tc2pwm_set_freq(unsigned char tc_channel, unsigned int fcpu_hz, unsigned int freq)
{
  __rc = (fcpu_hz/8) / freq;
  tc_write_rc(&AVR32_TC, tc_channel, __rc );
}

void tc2pwm_set_duty(unsigned char tc_channel, unsigned char pwm, unsigned short duty)
{
  // RA = (100% - duty) * RC
  volatile unsigned short rc = __rc;//tc_read_rc(&AVR32_TC, tc_channel);
  rc = ((100.0 - duty)/100.0) * rc;
  if (pwm == 0)
  {
    tc_write_ra(&AVR32_TC, tc_channel, rc);
  }
  else
  {
    tc_write_rb(&AVR32_TC, tc_channel, rc);
  }
}


void tc2pwm_reset_interrupt(unsigned char tc_channel)
{
  tc_read_sr(&AVR32_TC, tc_channel);
}
