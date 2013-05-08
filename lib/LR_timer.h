#ifndef LR_TIMER_H_
#define LR_TIMER_H_
//==============================================================================
/** Typedefs */
typedef void (*LR_timer_handler) (void);




//==============================================================================
/** Exported functions */
void LR_timer_init(unsigned char chan, unsigned long f);
void LR_timer_assign(unsigned char chan,
                     LR_timer_handler handler,
                     unsigned short period);
void LR_timer_start(unsigned char chan);
void LR_timer_stop(unsigned char chan);
void LR_timer_sti(unsigned char chan);
void LR_timer_wait(unsigned char chan);


#endif /* LR_TIMER_H_ */
