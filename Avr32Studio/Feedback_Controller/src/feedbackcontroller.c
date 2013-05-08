/*
 * feedbackcontroller.c
 *
 */
#include "feedbackcontroller.h"
#include "tc.h"
#include "intc.h"

//volatile struct FeedbackController state;
tc_waveform_opt_t wave_opt;
tc_interrupt_t int_opt;
volatile unsigned char _DELAYED_UPDATE;

#if __GNUC__
__attribute__((__interrupt__))
#elif __ICCAVR32__
__interrupt
#endif
static void do_FeedbackController(void)
{
	static unsigned char delay = 0;
        state.S[FC_STATE_E] = state.S[FC_STATE_R] - state.S[FC_STATE_Y];
	state.S[FC_STATE_U] = state.S[FC_STATE_U] + ( state.S[FC_STATE_T] * state.S[FC_STATE_I] * state.S[FC_STATE_E] );
	state.S[FC_STATE_Y] = state.S[FC_STATE_X] * state.S[FC_STATE_U];
	tc_write_rc(&AVR32_TC, state.tc_channel, state.clock_f * state.S[FC_STATE_T]);
	delay++;
	if (delay > _DELAYED_UPDATE)
	{
	  FeedbackController_UPDATE = 1;
	  delay = 0;
	}
	tc_read_sr(&AVR32_TC, state.tc_channel); // TODO: needed?
}

void init_FeedbackController(unsigned char tc_channel, unsigned long f)
{
	wave_opt.channel = tc_channel;
	wave_opt.wavsel = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER;
	wave_opt.tcclks = TC_CLOCK_SOURCE_TC3; // FPBA / 8
	state.clock_f = f >> ( 1 + wave_opt.tcclks);

	// No software triggers
	wave_opt.bswtrg = TC_EVT_EFFECT_NOOP;
	wave_opt.aswtrg = TC_EVT_EFFECT_NOOP;
	// No compare events on a and b
	wave_opt.acpa = TC_EVT_EFFECT_NOOP;
	wave_opt.acpc = TC_EVT_EFFECT_NOOP;
	wave_opt.bcpb = TC_EVT_EFFECT_NOOP;
	wave_opt.bcpc = TC_EVT_EFFECT_NOOP;
	// No external events
	wave_opt.aeevt = TC_EVT_EFFECT_NOOP;
	wave_opt.beevt = TC_EVT_EFFECT_NOOP;
	wave_opt.eevt = 1;
	wave_opt.enetrg = FALSE;
	wave_opt.eevtedg = TC_SEL_NO_EDGE;
	// more ...
	wave_opt.cpcdis = FALSE;
	wave_opt.cpcstop = FALSE;
	wave_opt.burst = FALSE;
	wave_opt.clki = FALSE;

	// We only need an interrupt on RC compare
	int_opt.etrgs = 0;
	int_opt.ldrbs = 0;
	int_opt.ldras = 0;
	int_opt.cpcs  = 1;
	int_opt.cpbs  = 0;
	int_opt.cpas  = 0;
	int_opt.lovrs = 0;
	int_opt.covfs = 0;

	Disable_global_interrupt();
	INTC_register_interrupt(do_FeedbackController, AVR32_TC_IRQ0 + tc_channel, AVR32_INTC_INT0);
	Enable_global_interrupt();
}


void start_FeedbackController()
{
	tc_init_waveform(&AVR32_TC, &wave_opt);
	tc_write_rc(&AVR32_TC, state.tc_channel, state.clock_f * state.S[FC_STATE_T]);
	tc_configure_interrupts(&AVR32_TC, state.tc_channel, &int_opt);
	tc_start(&AVR32_TC, state.tc_channel);
	_DELAYED_UPDATE = (1.0/state.S[FC_STATE_T])/10;
}



inline void set_FeedbackController(unsigned char T, double V)
{
	state.S[T] = V;
	if (T == FC_STATE_T)
	{
	  _DELAYED_UPDATE = (1.0/state.S[FC_STATE_T])/10;
	}
}
inline double get_FeedbackController(unsigned char T)
{
	return state.S[T];
}
