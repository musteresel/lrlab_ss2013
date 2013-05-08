/*
 * main.c
 */

#include "board.h"
#include "compiler.h"
#include "intc.h"
#include "gpio.h"
#include "pm.h"
#include "delay.h"
#include <avr32/io.h>
#include "pwm_pwm.h"
#include "stdio.h"
#include "delay.h"


#include "display.h"


#define VMAX 3.3


struct shared_info_t
{
  unsigned char duty; // Duty cycles in %
  float avg;
  unsigned int freq; // Frequency in Hz
  unsigned short freq_step; // Current step size in Hz
  unsigned char display_update_request : 1;
  unsigned char pwm_freq_update_request : 1;
  unsigned char pwm_update_request : 1;
  pwm_pwm_chan pwm;
} shared_info;

//=============================================================================



#if __GNUC__
__attribute__((__interrupt__))
#elif __ICCAVR32__
__interrupt
#endif
static void pb_int_handler(void)
{
	shared_info.display_update_request = 1;
	if (gpio_get_pin_interrupt_flag(GPIO_PUSH_BUTTON_0))
	{
		shared_info.freq += shared_info.freq_step;
		shared_info.pwm_freq_update_request = 1;
		gpio_clear_pin_interrupt_flag(GPIO_PUSH_BUTTON_0);
	}
	else if (gpio_get_pin_interrupt_flag(GPIO_PUSH_BUTTON_1))
	{
		shared_info.freq -= shared_info.freq_step;
		shared_info.pwm_freq_update_request = 1;
		gpio_clear_pin_interrupt_flag(GPIO_PUSH_BUTTON_1);
	}
	else if (gpio_get_pin_interrupt_flag(GPIO_PUSH_BUTTON_2))
	{
		shared_info.freq_step *= 10;
		if (shared_info.freq_step > 1000)
		{
			shared_info.freq_step = 10;
		}
		gpio_clear_pin_interrupt_flag(GPIO_PUSH_BUTTON_2);
	}
}



#if __GNUC__
__attribute__((__interrupt__))
#elif __ICCAVR32__
__interrupt
#endif
static void joystick_int_handler(void)
{
	shared_info.display_update_request = 1;
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_UP))
	{
		if (shared_info.avg < 3.3)
		{
			shared_info.avg += 0.1;
			if (shared_info.avg > 3.3)
			{
				shared_info.avg = 3.3;
			}
			double t = shared_info.avg / VMAX;
			shared_info.duty = (unsigned char)(t * 100);
			shared_info.pwm_update_request = 1;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_UP);
	}
	else if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_DOWN))
	{
		if (shared_info.avg > 0)
		{
			shared_info.avg -= 0.1;
			if (shared_info.avg < 0)
			{
				shared_info.avg = 0;
			}
			double t = shared_info.avg / VMAX;
			shared_info.duty = (unsigned char)(t * 100);
			shared_info.pwm_update_request = 1;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_DOWN);
	}
	else if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_LEFT))
	{
		if (shared_info.duty < 100)
		{
			shared_info.duty++;
			shared_info.avg = (shared_info.duty/100.0) * VMAX;
			shared_info.pwm_update_request = 1;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_LEFT);
	}
	else if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_RIGHT))
	{
		if (shared_info.duty > 0)
		{
			shared_info.duty--;
			shared_info.avg = (shared_info.duty/100.0) * VMAX;
			shared_info.pwm_update_request = 1;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_RIGHT);
	}
}




//=======================================================================
void configure_push_buttons(void)
{
  gpio_enable_pin_interrupt(GPIO_PUSH_BUTTON_0 , GPIO_RISING_EDGE);
  gpio_enable_pin_interrupt(GPIO_PUSH_BUTTON_2 , GPIO_RISING_EDGE);
  gpio_enable_pin_interrupt(GPIO_PUSH_BUTTON_1 , GPIO_RISING_EDGE);
  Disable_global_interrupt();
  INTC_register_interrupt( &pb_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_PUSH_BUTTON_2/8), AVR32_INTC_INT1);
  INTC_register_interrupt( &pb_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_PUSH_BUTTON_1/8), AVR32_INTC_INT1);
  INTC_register_interrupt( &pb_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_PUSH_BUTTON_0/8), AVR32_INTC_INT1);
  Enable_global_interrupt();
}
void configure_joystick(void)
{
  gpio_enable_pin_interrupt(GPIO_JOYSTICK_UP , GPIO_FALLING_EDGE);
  gpio_enable_pin_interrupt(GPIO_JOYSTICK_DOWN , GPIO_FALLING_EDGE);
  gpio_enable_pin_interrupt(GPIO_JOYSTICK_RIGHT , GPIO_FALLING_EDGE);
  //gpio_enable_pin_interrupt(GPIO_JOYSTICK_PUSH , GPIO_FALLING_EDGE);
  gpio_enable_pin_interrupt(GPIO_JOYSTICK_LEFT , GPIO_FALLING_EDGE);
  Disable_global_interrupt();
  INTC_register_interrupt( &joystick_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_UP/8), AVR32_INTC_INT1);
  INTC_register_interrupt( &joystick_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_DOWN/8), AVR32_INTC_INT1);
  INTC_register_interrupt( &joystick_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_RIGHT/8), AVR32_INTC_INT1);
  INTC_register_interrupt( &joystick_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_LEFT/8), AVR32_INTC_INT1);
  //INTC_register_interrupt( &dip204_example_Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_PUSH/8), AVR32_INTC_INT1);
  Enable_global_interrupt();
}






//======================================================================
void main_loop ()
{
  char string[20];
  for(;;)
  {
    // Check for display update request
    if (shared_info.display_update_request)
    {
      sprintf(string,"Avg.  PWM:   %3.2f  x",shared_info.avg);
      dip204_set_cursor_position(1,1);
      dip204_write_string(string);
      sprintf(string,"Duty  PWM:    %3d  x",shared_info.duty);
      dip204_set_cursor_position(1,2);
      dip204_write_string(string);
      sprintf(string,"Frequency:%7u Hz",shared_info.freq);
      dip204_set_cursor_position(1,3);
      dip204_write_string(string);
      sprintf(string,"         +/- %4d Hz ",shared_info.freq_step);
      dip204_set_cursor_position(1,4);
      dip204_write_string(string);
      dip204_set_cursor_position(1,4);
      shared_info.display_update_request = 0;
    }
    if (shared_info.pwm_freq_update_request || shared_info.pwm_update_request)
    {
    	unsigned long old_freq = shared_info.freq;
    	shared_info.pwm_freq_update_request = 0;
    	shared_info.pwm_update_request = 0;
    	pwm_pwm_update_freq(&(shared_info.pwm), shared_info.freq);
    	delay_ms(100);
    	pwm_pwm_update_duty(&(shared_info.pwm), shared_info.duty / 100.0);
    	delay_ms(100);
    }
  }
}




//======================================================================
/*
 * Main function. Setup of all required components, then handles control
 * over to the main loop.
 * Returns never.
*/
int main(void)
{
  // Switch main oscilator, frequency can be found in FOSC0
  pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);
  // Init delay driver. This is used for spi and the display
  //delay_init(FOSC0);


  // Setup interrupts - interrupt handling functions will be added later
  // by various component setup routines
  Disable_global_interrupt();
  INTC_init_interrupts();
  Enable_global_interrupt();

  // Activate display dip204
  init_display(FOSC0);
  //dip204_hide_cursor();




  shared_info.freq_step = 10;
  shared_info.freq = 8;
  shared_info.duty = 30;
  shared_info.avg = (shared_info.duty/100.0) * VMAX;
  shared_info.display_update_request = 1;
  avr32_pwm_channel_t pwm_channel = { .ccnt = 0 };
  shared_info.pwm.chan = &pwm_channel;

  gpio_enable_module_pin(AVR32_PIN_PB19, 0);

  pwm_pwm_init();


  pwm_pwm_configure_channel(&(shared_info.pwm), shared_info.freq, shared_info.duty / 100.0, FOSC0);

  configure_push_buttons();
  configure_joystick();

  main_loop();
  return 0;
}
