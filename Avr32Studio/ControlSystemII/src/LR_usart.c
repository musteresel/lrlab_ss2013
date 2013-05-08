/*
 * LR_usart.c
 */
//=============================================================================
/** Include directives */
//#include <avr32/io.h>
//#include "compiler.h"
//#include "board.h"
#include "intc.h"
#include "gpio.h"
#include "usart.h"
#include "LR_ringbuffer.h"





//==============================================================================
/** Global variables */
LR_ringbuffer * _buffer;
//int __send[150];
//LR_ringbuffer _send;




//==============================================================================
/** Interrupt handler to serve USART interrupt
 *
 * This function saves the received character into the assigned ringbuffer.
 */
__attribute__((__interrupt__))
static void usart_int_handler( void )
{
  int c;
  usart_read_char( &AVR32_USART0, &c );
  LR_ringbuffer_put(_buffer,c);
}




//==============================================================================
void LR_usart_init( unsigned long fosc, LR_ringbuffer * b )
{
  /** GPIO mapping */
  static const gpio_map_t USART_GPIO_MAP =
  {
    { AVR32_USART0_RXD_0_0_PIN, AVR32_USART0_RXD_0_0_FUNCTION },
    { AVR32_USART0_TXD_0_0_PIN, AVR32_USART0_TXD_0_0_FUNCTION }
  };
  gpio_enable_module( USART_GPIO_MAP,
                      sizeof(USART_GPIO_MAP) / sizeof(USART_GPIO_MAP[0]));

  /** Configure USART */
  static const usart_options_t USART_OPTIONS =
  {
    .baudrate = 57600,
    .charlength = 8,
    .paritytype = USART_NO_PARITY,
    .stopbits = USART_1_STOPBIT,
    .channelmode = USART_NORMAL_CHMODE
  };
  usart_init_rs232( &AVR32_USART0, &USART_OPTIONS, fosc );

  /** Assign receive buffer */
  _buffer = b;
  //LR_ringbuffer_init(&_send, __send, 150);

  /** Assign interrupt handler */
  Disable_global_interrupt();
  INTC_register_interrupt( &usart_int_handler,
                           AVR32_USART0_IRQ,
                           AVR32_INTC_INT1);
  // Enable USART Rx interrupt.
  (&AVR32_USART0)->ier = AVR32_USART_IER_RXRDY_MASK;
  Enable_global_interrupt();
}





//==============================================================================
void LR_usart_write(char const * string)
{
  usart_write_line(&AVR32_USART0, string);
}
