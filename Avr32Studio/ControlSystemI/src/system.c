/*
 * system.c
 */
//==============================================================================
/** Include directives */
#include "board.h"
#include "compiler.h"
#include "intc.h"
#include "stdio.h"
#include "pm.h"
#include "LR_ringbuffer.h"
#include "LR_usart.h"
#include "LR_display.h"
#include "LR_timer.h"
#include "LR_adc.h"
#include "math.h"
#include "LR_pwm.h"
#include "gpio.h"
#include <avr32/io.h>




//==============================================================================
/** State descriptions */
#define SYS_STATE_STA 1
#define SYS_STATE_SEN 2
#define SYS_STATE_CAL 3
#define SYS_STATE_ANG 4
#define SYS_STATE_CON 5
#define SYS_STATE_SET 6

#define SYS_IN_STATE 0
#define SYS_ENTER_STATE 1
#define SYS_LEAVE_STATE 2




//==============================================================================
/** Command word definitions */
#define CMD_CON (('C' << 24) | ('O' << 16) | ('N' << 8) | 0)
#define CMD_ANG (('A' << 24) | ('N' << 16) | ('G' << 8) | 0)
#define CMD_CAL (('C' << 24) | ('A' << 16) | ('L' << 8) | 0)
#define CMD_SEN (('S' << 24) | ('E' << 16) | ('N' << 8) | 0)
#define CMD_STA (('S' << 24) | ('T' << 16) | ('A' << 8) | 0)
#define CMD_SET (('S' << 24) | ('E' << 16) | ('T' << 8) | 0)
#define CMD_ACK (('A' << 24) | ('C' << 16) | ('K' << 8) | 0)




//==============================================================================
/** State machine state */
typedef struct
{
  int cur;
  int prev;
  int next;
  signed char move;
}State;




//==============================================================================
//** Global variables */
LR_ringbuffer rb_usart;
#define RB_USART_SIZE 160
int __rb_usart[RB_USART_SIZE];

avr32_pwm_channel_t pwm_channel = { .ccnt = 0 };
LR_pwm pwm;

#define FILTER_SIZE 10
double _filter_data[FILTER_SIZE];

volatile double calib_x = 932.502000;
volatile double calib_y;
volatile double calib_z = 920.836000;
volatile double sens_max;
volatile double sens_min;
volatile double sens = -42.00300;

volatile double pwm_freq = 100;
volatile double sample_time = 10;
volatile double current_angle = 0;

volatile double con_P = 1;
volatile double con_I = .0005;
volatile double con_D = .05;

volatile double con_Desired = 0;
volatile double con_Integral = 0;
volatile double con_lastE = 0;



//==============================================================================
/** Returned current angle of the seesaw using 2 axis method
 *
 * This function reads x and z values from the sensor and computes the resulting
 * angle of the seesaw using atan2(z,x).
 *
 * Correct calib_x, calib_z and sens values must have been set BEFORE using
 * this function.
 *
 * The function uses a simple mean filter with 10 values.
 */
double get_filtered_angle()
{
  double ang = 0;
  double x = 0;
  double z = 0;
  static int filter_position = 0;
  int iterator = 0;
  LR_adc_start();
  x = (LR_adc_get(5) - calib_x) / sens;
  z = (LR_adc_get(7) - calib_z) / sens;
  //ang = asin(ang) * (180. / 3.1415);
  ang = atan2(z,x) * (180. / 3.1415);
  ang = ang - 90;
  _filter_data[filter_position] = ang;
  filter_position++;
  if (filter_position >= FILTER_SIZE)
  {
    filter_position = 0;
  }
  ang = 0;
  for(; iterator < FILTER_SIZE; iterator++)
  {
    ang += _filter_data[iterator];
  }
  ang /= FILTER_SIZE;
  return ang;
}




//==============================================================================
/** Display update handler
 *
 * This function is called periodically to update the display.
 */
__attribute__((__interrupt__))
static void display_handler(void)
{
  static int n = 0;
  if (n < 100)
  {
    n++;
  }
  else
  {
    n = 0;
    char string[50];
    sprintf(string,"P:%5.3f D:%5.3f",con_P, con_D);
    dip204_set_cursor_position(1,1);
    dip204_write_string(string);
    sprintf(string,"I:%5.3f Int:%6.3f", con_I, con_Integral);
    dip204_set_cursor_position(1,2);
    dip204_write_string(string);
    sprintf(string,"R:%5.1f A:%5.1f ",con_Desired, current_angle);
    dip204_set_cursor_position(1,3);
    dip204_write_string(string);
    sprintf(string,"F:%6.2f T:%4.1f", pwm_freq, sample_time);
    dip204_set_cursor_position(1,4);
    dip204_write_string(string);


    static unsigned char i = 0;
    dip204_set_cursor_position(20,1);
    if (i < 10)
    {
      dip204_write_string("+");
    }
    else
    {
      dip204_write_string(" ");
    }
    dip204_set_cursor_position(20,2);
    if (i > 10 && i < 20)
    {
      dip204_write_string("+");
    }
    else
    {
      dip204_write_string(" ");
    }
    dip204_set_cursor_position(20,3);
    if (i > 20 && i < 30)
    {
      dip204_write_string("+");
    }
    else
    {
      dip204_write_string(" ");
    }
    dip204_set_cursor_position(20,4);
    if (i > 30 && i < 40)
    {
      dip204_write_string("+");
    }
    else
    {
      dip204_write_string(" ");
    }
    i++;
    if (i == 40)
    {
      i = 0;
    }
  }
  LR_timer_sti(0);
}




//==============================================================================
/** CON handler
 *
 * This function is the controller of the closed loop control system
 */
__attribute__((__interrupt__))
void con_handler(void)
{
  current_angle = get_filtered_angle();
  //----------------------------------------------------------------------------
  double U = 0;
  double e = con_Desired - current_angle;

  // Proportional part
  U = con_P * e;

  // Integral part
  con_Integral += con_I * sample_time * e;
  U += con_Integral;


  // Derivative part
  double derived = e - con_lastE;
  con_lastE = e;
  if (derived != 0)
  {
    U += con_D * (derived / sample_time);
  }

  //----------------------------------------------------------------------------
  // Saturation function
  if (U > 100)
  {
    U = 100;
  }
  else if (U < -100)
  {
    U = -100;
  }

  //----------------------------------------------------------------------------
  // Negative to AIN*
  if (U < 0)
  {
    gpio_set_gpio_pin(AVR32_PIN_PC04);
    gpio_clr_gpio_pin(AVR32_PIN_PC05);
    U *= -1;
  }
  else
  {
    gpio_set_gpio_pin(AVR32_PIN_PC05);
    gpio_clr_gpio_pin(AVR32_PIN_PC04);
  }

  // Update duty
  /*char string[80];
  sprintf(string,"ANG: %f   E: %f   U: %f\n",ang,e,U);
  LR_usart_write(string);*/
  LR_pwm_update_duty(&pwm, U/100.);
  //----------------------------------------------------------------------------
  LR_timer_sti(1);
}





//==============================================================================
/** ANG handler
 *
 * This function writes the current angle to the USART.
 */
__attribute__((__interrupt__))
void ang_handler(void)
{
  // Read angle
  double ang = get_filtered_angle();
  char string[30];
  sprintf(string,"ANG: %f\n",ang);
  LR_usart_write(string);
  LR_timer_sti(1);
}




//==============================================================================
/** Calibration handler.
 *
 * This function reads 1000 samples to calibrate the zero angle position of the
 * sensor.
 */
__attribute__((__interrupt__))
void calx_handler(void)
{
  static unsigned short n = 0;
  static unsigned long x = 0;
  if (n < 1000)
  {
    LR_adc_start();
    x += LR_adc_get(5);
    n++;
  }
  else
  {
    LR_timer_stop(1);
    n = 0;
    calib_x = x / 1000.;
    x = 0;
  }
  LR_timer_sti(1);
}



//==============================================================================
/** Calibration handler.
 *
 * This function reads 1000 samples to calibrate the zero angle position of the
 * sensor.
 */
__attribute__((__interrupt__))
void calz_handler(void)
{
  static unsigned short n = 0;
  static unsigned long z = 0;
  if (n < 1000)
  {
    LR_adc_start();
    z += LR_adc_get(7);
    n++;
  }
  else
  {
    LR_timer_stop(1);
    n = 0;
    calib_z = z / 1000.;
    z = 0;
  }
  LR_timer_sti(1);
}



//==============================================================================
/** Read the max sensor value */
__attribute__((__interrupt__))
void sens_max_handler(void)
{
  static unsigned short n = 0;
  static unsigned long val = 0;
  if (n < 1000)
  {
    LR_adc_start();
    val += LR_adc_get(5);
    n++;
  }
  else
  {
    LR_timer_stop(1);
    n = 0;
    sens_max = val / 1000.;
    val = 0;
  }
  LR_timer_sti(1);
}




//==============================================================================
/** Read the min sensor value */
__attribute__((__interrupt__))
void sens_min_handler(void)
{
  static unsigned short n = 0;
  static unsigned long val = 0;
  if (n < 1000)
  {
    LR_adc_start();
    val += LR_adc_get(5);
    n++;
  }
  else
  {
    LR_timer_stop(1);
    n = 0;
    sens_min = val / 1000.;
    val = 0;
  }
  LR_timer_sti(1);
}




//==============================================================================
/** Wait to receive a command word via USART
 *
 * This function gets called after the state dependend operations are finished
 * and waits for a new command word via USART.
 */
int wait_cmd()
{
  int s = -1;
  int c = 0;
  unsigned int cmd = 0;
  unsigned char received = 0;
  while (s == -1)
  {
    if (LR_ringbuffer_get(&rb_usart, &c))
    {
      cmd |= (0xFF & c) << ( 8 * (3 - received) );
      received++;
      if (received == 4)
      {
        if (cmd == CMD_CON)
        {
          s = SYS_STATE_CON;
        }
        else if (cmd == CMD_ANG)
        {
          s = SYS_STATE_ANG;
        }
        else if (cmd == CMD_CAL)
        {
          s = SYS_STATE_CAL;
        }
        else if (cmd == CMD_SEN)
        {
          s = SYS_STATE_SEN;
        }
        else if (cmd == CMD_STA)
        {
          s = SYS_STATE_STA;
        }
        else if (cmd == CMD_SET)
        {
          s = SYS_STATE_SET;
        }
        if (s == -1)
        {
          LR_usart_write("NACK\n");
          cmd = 0;
          received = 0;
        }
        else
        {
          LR_usart_write("ACK\n");
        }
      }
    }
  }
  return s;
}



//==============================================================================
/** Wait to receive an ACK via USART.
 *
 * This function waits until it received ACK via USART.
 */
void wait_ack()
{
  signed char s = -1;
  int c = 0;
  unsigned int cmd = 0;
  unsigned char received = 0;
  while (s == -1)
  {
    if (LR_ringbuffer_get(&rb_usart, &c))
    {
      cmd |= (0xFF & c) << ( 8 * (3 - received) );
      received++;
      if (received == 4)
      {
        if (cmd == CMD_ACK)
        {
          s = 1;
        }
        else
        {
          LR_usart_write("ACK?\n");
          cmd = 0;
          received = 0;
        }
      }
    }
  }
}



//==============================================================================
/** Read a parameter from USART */
char read_parameter(double * target)
{
  signed char parm = 0;
  int c = 0;
  LR_usart_write("Waiting for Parameter!\n");
  while (parm == 0)
  {
    if (LR_ringbuffer_get(&rb_usart, &c))
    {
      parm = 0xFF & c;
    }
  }
  unsigned int i = 0;
  signed char state = 0;
  signed char negative = 1;
  while (state == 0)
  {
    if (LR_ringbuffer_get(&rb_usart, &c))
    {
      c = c & 0xFF;
      if (c == '-')
      {
        negative = -1;
        state = 1;
        LR_usart_write("Going negative!\n");
      }
      else if (c == 0)
      {
        state = -1;
        LR_usart_write("Finished .. \n");
      }
      else if (c == '.')
      {
        state = 2;
        LR_usart_write("Directly to comma value\n");
      }
      else if (c >= '0' && c <= '9')
      {
        i = c - '0';
        state = 1;
        LR_usart_write("First digit\n");
      }
    }
  }
  while (state == 1)
  {
    if (LR_ringbuffer_get(&rb_usart, &c))
    {
      c = 0xFF & c;
      if (c == 0)
      {
        state = -1;
        LR_usart_write("no more digits\n");
      }
      else if (c == '.')
      {
        state = 2;
        LR_usart_write("comma time\n");
      }
      else if (c >= '0' && c <= '9')
      {
        i *= 10;
        i += c - '0';
        LR_usart_write("another digit\n");
      }
    }
  }
  unsigned int n = 0;
  unsigned int div = 1;
  while (state == 2)
  {
    if (LR_ringbuffer_get(&rb_usart, &c))
    {
      c = 0xFF & c;
      if (c == 0)
      {
        state = -1;
        LR_usart_write("Done\n");
      }
      else if (c >= '0' && c <= '9')
      {
        n *= 10;
        div *= 10;
        n += c - '0';
        LR_usart_write("A digit .. \n");
      }
    }
  }
  *target = negative * (i + ((double)n / div));
  return parm;
}



//==============================================================================
/** Die function
 *
 * This function gets called when something goes terribly wrong inside the
 * state machine. It just prints a error message through USART and then
 * loops for ever (Disables interrupts).
 */
__attribute__((noreturn))
void state_machine_die()
{
  LR_usart_write("ERROR: State Machine died!\n");
  while (1) Disable_global_interrupt();
}




//==============================================================================
/** Generic state enter function */
void state_machine_enter(State * state)
{
  state->move = SYS_IN_STATE;
  char string[20];
  sprintf(string, "ENTER: %d\n",state->cur);
  LR_usart_write(string);
}
/** Generic state change function */
void state_machine_change(State * state)
{
  state->move = SYS_LEAVE_STATE;
}
/** Generic state leave function */
void state_machine_leave(State * state)
{
  state->move = SYS_ENTER_STATE;
  char string[20];
  sprintf(string, "LEAVE: %d\n",state->cur);
  LR_usart_write(string);
  state->prev = state->cur;
  state->cur = state->next;
}




//==============================================================================
/** Main loop function
 *
 * This function contains the main code of the system, which will be executed
 * after system setup until loss of power.
 * This includes usart parsing, state control and general code to maintain
 * the system at runtime.
 */
__attribute__((noreturn))
void main_loop( void )
{
  /** state etc */
  State state;
  state.cur = SYS_STATE_STA;
  state.prev = SYS_STATE_STA;
  state.next = SYS_STATE_STA;
  state.move = SYS_ENTER_STATE;

  while(1)
  {
    //oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooCON
    if (state.cur == SYS_STATE_CON)
    {
      /** Actions which belong to the control state */
      //-----------------------------------------------------------------------I
      if (state.move == SYS_IN_STATE)
      {
        // TODO
        // activate PID controller
        int s = wait_cmd();
        if (s == SYS_STATE_SET)
        {
          // read new parameter
          double new_parm = 0;
          char parm = read_parameter(&new_parm);
          if (parm == 'F') // PWM FREQ
          {
            pwm_freq = new_parm;
            LR_timer_stop(1);
            LR_pwm_stop(&pwm);
            LR_pwm_start(&pwm, pwm_freq, 0, FOSC0);
            LR_timer_start(1);
          }
          else if (parm == 'P')
          {
            con_P = new_parm;
          }
          else if (parm == 'I')
          {
            con_I = new_parm;
            con_Integral = 0;
          }
          else if (parm == 'D')
          {
            con_D = new_parm;
          }
          else if (parm == 'R')
          {
            con_Desired = new_parm;
          }
          else if (parm == 'S')
          {
            sample_time = new_parm;
            LR_timer_stop(1);
            LR_timer_assign(1,con_handler,sample_time);
            LR_timer_start(1);
          }
          else
          {
            LR_usart_write("NACK: Unknown parameter\n");
          }
        }
        else if (s != state.cur)
        {
          state.next = s;
          state_machine_change(&state);
        }
      }
      //-----------------------------------------------------------------------E
      else if (state.move == SYS_ENTER_STATE)
      {
        state_machine_enter(&state);
        LR_pwm_start(&pwm, pwm_freq, 0, FOSC0);
        LR_timer_assign(1, con_handler, sample_time);
        LR_timer_start(1);
      }
      //-----------------------------------------------------------------------L
      else if (state.move == SYS_LEAVE_STATE)
      {
        LR_pwm_stop(&pwm);
        LR_timer_stop(1);
        gpio_clr_gpio_pin(AVR32_PIN_PC05);
        gpio_clr_gpio_pin(AVR32_PIN_PC04);
        state_machine_leave(&state);
      }
      //-----------------------------------------------------------------------D
      else
      {
        /** Actions when state move failed */
        state_machine_die();
      }
    }
    //oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooANG
    else if (state.cur == SYS_STATE_ANG)
    {
      /** Actions which belong to the angle measure state */
      //-----------------------------------------------------------------------I
      if (state.move == SYS_IN_STATE)
      {
        LR_timer_assign(1,ang_handler,500);
        LR_timer_start(1);
        int s = wait_cmd();
        if (s != state.cur)
        {
          state.next = s;
          state_machine_change(&state);
        }
      }
      //-----------------------------------------------------------------------E
      else if (state.move == SYS_ENTER_STATE)
      {
        state_machine_enter(&state);
      }
      //-----------------------------------------------------------------------L
      else if (state.move == SYS_LEAVE_STATE)
      {
        LR_timer_stop(1);
        state_machine_leave(&state);
      }
      //-----------------------------------------------------------------------D
      else
      {
        /** Actions when state move failed */
        state_machine_die();
      }
    }
    //oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooCAL
    else if (state.cur == SYS_STATE_CAL)
    {
      /** Actions which belong to the calibration state */
      //-----------------------------------------------------------------------I
      if (state.move == SYS_IN_STATE)
      {
        LR_usart_write("Sensor at 0g X Position?\n");
        wait_ack();
        LR_timer_assign(1,calx_handler,10);
        LR_timer_start(1);
        LR_timer_wait(1);
        char string[40];
        sprintf(string,"VxOffset: %f\n",calib_x);
        LR_usart_write(string);
        wait_ack();
        LR_usart_write("Sensor at 0g Z Position?\n");
        wait_ack();
        LR_timer_assign(1,calz_handler,10);
        LR_timer_start(1);
        LR_timer_wait(1);
        sprintf(string,"VzOffset: %f\n", calib_z);
        LR_usart_write(string);
        wait_ack();
        state.next = state.prev;
        state_machine_change(&state);
      }
      //-----------------------------------------------------------------------E
      else if (state.move == SYS_ENTER_STATE)
      {
        state_machine_enter(&state);
        // TODO on enter
      }
      //-----------------------------------------------------------------------L
      else if (state.move == SYS_LEAVE_STATE)
      {
        // TODO on leave
        state_machine_leave(&state);
      }
      //-----------------------------------------------------------------------D
      else
      {
        /** Actions when state move failed */
        state_machine_die();
      }
    }
    //oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooSEN
    else if (state.cur == SYS_STATE_SEN)
    {
      /** Actions which belong to the sensivity measure state */
      //-----------------------------------------------------------------------I
      if (state.move == SYS_IN_STATE)
      {
        // TODO
        LR_usart_write("X-Axis at +1g Position?\n");
        wait_ack();
        // read v
        LR_timer_assign(1, sens_max_handler, 10);
        LR_timer_start(1);
        LR_timer_wait(1);
        char string[40];
        sprintf(string,"VxMax: %f\n", sens_max);
        LR_usart_write(string);
        wait_ack();
        LR_usart_write("X-Axis at -1g Position?\n");
        wait_ack();
        // read v
        LR_timer_assign(1,sens_min_handler, 10);
        LR_timer_start(1);
        LR_timer_wait(1);
        sprintf(string,"VxMin: %f\n",sens_min);
        LR_usart_write(string);
        wait_ack();
        sens = (sens_max - sens_min) / 2.;
        sprintf(string,"Sensivity: %f\n", sens);
        LR_usart_write(string);
        wait_ack();
        state.next = state.prev;
        state_machine_change(&state);
      }
      //-----------------------------------------------------------------------E
      else if (state.move == SYS_ENTER_STATE)
      {
        state_machine_enter(&state);
        // TODO on enter
      }
      //-----------------------------------------------------------------------L
      else if (state.move == SYS_LEAVE_STATE)
      {
        // TODO on leave
        state_machine_leave(&state);
      }
      //-----------------------------------------------------------------------D
      else
      {
        /** Actions when state move failed */
        state_machine_die();
      }
    }
    //oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooSTA
    else if (state.cur == SYS_STATE_STA)
    {
      /** Actions which belong to the standby state */
      //-----------------------------------------------------------------------I
      if (state.move == SYS_IN_STATE)
      {
        // TODO
        int s = wait_cmd();
        if (s != state.cur)
        {
          state.next = s;
          state_machine_change(&state);
        }
      }
      //-----------------------------------------------------------------------E
      else if (state.move == SYS_ENTER_STATE)
      {
        state_machine_enter(&state);
        // TODO on enter
      }
      //-----------------------------------------------------------------------L
      else if (state.move == SYS_LEAVE_STATE)
      {
        // TODO on leave
        state_machine_leave(&state);
      }
      //-----------------------------------------------------------------------D
      else
      {
        /** Actions when state move failed */
        state_machine_die();
      }
    }
    //oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooDIE
    else
    {
      /** Actions to take when the state machine fails */
      state_machine_die();
    }
  }
}





//==============================================================================
/** Main function to setup system state
 *
 * This function is called upon start and used to setup all required hardware
 * and components.
 */
int main ( void )
{
  /** Switch the oscilator */
  pm_switch_to_osc0( &AVR32_PM, FOSC0, OSC0_STARTUP);

  /** Setupt interrupts, handlers will be added later by the various system
   * components.
   */
  Disable_global_interrupt();
  INTC_init_interrupts();
  Enable_global_interrupt();

  /** Activate the display */
  LR_display_init( FOSC0 );

  /** Init the ringbuffers */
  LR_ringbuffer_init( &rb_usart, __rb_usart, RB_USART_SIZE);
  // TODO

  /** Init the usart inteface */
  LR_usart_init( FOSC0, &rb_usart);

  /** Init timer counters */
  LR_timer_init(0,FOSC0);
  LR_timer_init(1,FOSC0);

  /** Set a timer to write to the display */
  LR_timer_assign(0,display_handler,5);
  LR_timer_start(0);

  /** Configure ADC */
  LR_adc_init_channel(5);
  LR_adc_init_channel(6);
  LR_adc_init_channel(7);
  LR_adc_init();
  LR_adc_enable_channel(5);
  LR_adc_enable_channel(6);
  LR_adc_enable_channel(7);

  /** Init pwm */
  pwm.chan = &(pwm_channel);
  gpio_enable_module_pin(AVR32_PIN_PB19, 0); // PWM out
  gpio_enable_module_pin(AVR32_PIN_PC04, 0); // AIN1
  gpio_enable_module_pin(AVR32_PIN_PC05, 0); // AIN2
  gpio_set_gpio_pin(AVR32_PIN_PC04);
  gpio_clr_gpio_pin(AVR32_PIN_PC05);
  LR_pwm_init();


  /** Enter the main loop */
  main_loop( );
  return 0;
}


// VCC3 -> Sensor VCC
// GND  -> Sensor GND

// PA26 -> X
// PA27 -> Y
// PA28 -> Z

// PB19 -> PWMA
// PC04 -> AIN1
// PC05 -> AIN2
