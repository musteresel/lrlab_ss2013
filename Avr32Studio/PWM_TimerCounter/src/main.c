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
//#include "pwm.h"
#include "stdio.h"


#include "display.h"
#include "tc_pwm.h"



volatile struct shared_info_t
{
  unsigned char duty1; // Duty cycles in %
  unsigned char duty2;
  unsigned int freq; // Frequency in Hz
  unsigned short freq_step; // Current step size in Hz
  unsigned char display_update_request : 1;
  unsigned char pwm_freq_update_request : 1;
  unsigned char pwm1_update_request : 1;
  unsigned char pwm2_update_request : 1;
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
		shared_info.pwm_freq_update_request = 1;
		shared_info.pwm1_update_request = 1;
		shared_info.pwm2_update_request = 1;
		shared_info.freq += shared_info.freq_step;
		gpio_clear_pin_interrupt_flag(GPIO_PUSH_BUTTON_0);
	}
	else if (gpio_get_pin_interrupt_flag(GPIO_PUSH_BUTTON_1))
	{
		shared_info.pwm_freq_update_request = 1;
		shared_info.pwm1_update_request = 1;
		shared_info.pwm2_update_request = 1;
		shared_info.freq -= shared_info.freq_step;
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
		if (shared_info.duty1 < 100)
		{
			shared_info.duty1++;
			shared_info.pwm1_update_request = 1;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_UP);
	}
	else if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_DOWN))
	{
		if (shared_info.duty1 > 0)
		{
			shared_info.duty1--;
			shared_info.pwm1_update_request = 1;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_DOWN);
	}
	else if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_LEFT))
	{
		if (shared_info.duty2 < 100)
		{
			shared_info.duty2++;
			shared_info.pwm2_update_request = 1;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_LEFT);
	}
	else if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_RIGHT))
	{
		if (shared_info.duty2 > 0)
		{
			shared_info.duty2--;
			shared_info.pwm2_update_request = 1;

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







//=====================================================================
#if defined (__GNUC__)
__attribute__((__interrupt__))
#elif defined (__ICCAVR32__)
#pragma handler = EXAMPLE_TC_IRQ_GROUP, 1
__interrupt
#endif
static void pwm_handler(void)
{
  static unsigned char state = 1;
  tc2pwm_reset_interrupt(0);
  if (state == 0)
  {
    if (shared_info.pwm_freq_update_request)
    {
	  tc2pwm_set_freq(0,FOSC0,shared_info.freq);
	  shared_info.pwm_freq_update_request = 0;
    }
    if (shared_info.pwm1_update_request)
    {
      tc2pwm_set_duty(0,0,shared_info.duty1);
      shared_info.pwm1_update_request = 0;
    }
    if (shared_info.pwm2_update_request)
    {
      tc2pwm_set_duty(0,1,shared_info.duty2);
      shared_info.pwm2_update_request = 0;
    }
    gpio_clr_gpio_pin(AVR32_PIN_PC05);
    state = 1;
  }
  else
  {
    gpio_set_gpio_pin(AVR32_PIN_PC05);
    state = 0;
  }
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
      sprintf(string,"Duty PWM1:    %3d  x",shared_info.duty1);
      dip204_set_cursor_position(1,1);
      dip204_write_string(string);
      sprintf(string,"Duty PWM2:    %3d  x",shared_info.duty2);
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


  static const gpio_map_t tc2pwm_gpio_toa =
  {
    {AVR32_TC_A0_0_0_PIN,AVR32_TC_A0_0_0_FUNCTION}
  };
  gpio_enable_module(tc2pwm_gpio_toa,sizeof(tc2pwm_gpio_toa) / sizeof(tc2pwm_gpio_toa[0]));
  tc2pwm_init(0,&pwm_handler,0,0);

  shared_info.freq_step = 10;
  shared_info.freq = 100;
  shared_info.duty1 = 20;
  shared_info.duty2 = 80;
  shared_info.display_update_request = 1;

  tc2pwm_set_freq(0,FOSC0, shared_info.freq);
  tc2pwm_set_duty(0,0,shared_info.duty1);
  tc2pwm_set_duty(0,1,shared_info.duty2);

  configure_push_buttons();
  configure_joystick();

  main_loop();
  return 0;
}
