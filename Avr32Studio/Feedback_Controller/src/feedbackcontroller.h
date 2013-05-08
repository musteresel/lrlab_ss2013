/*
 * feedbackcontroller.h
 *
 */


#ifndef FEEDBACKCONTROLLER_H_
#define FEEDBACKCONTROLLER_H_


#define FC_STATE_R 0
#define FC_STATE_E 1
#define FC_STATE_I 2
#define FC_STATE_Y 3
#define FC_STATE_X 4
#define FC_STATE_T 5
#define FC_STATE_U 6



volatile struct FeedbackController
{
	double S[7];
	unsigned char tc_channel;
	unsigned long clock_f;
} state;

void start_FeedbackController();
void init_FeedbackController(unsigned char tc_channel, unsigned long f);
inline void set_FeedbackController(unsigned char T, double V);
inline double get_FeedbackController(unsigned char T);

volatile unsigned char FeedbackController_UPDATE;

#endif /* FEEDBACKCONTROLLER_H_ */
