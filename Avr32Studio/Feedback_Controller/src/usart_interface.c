//=============================================================================
#include <avr32/io.h>
#include "compiler.h"
#include "board.h"
#include "intc.h"
#include "gpio.h"
#include "usart.h"

#include "ringbuffer.h"
//=============================================================================
#  define EXAMPLE_USART                 (&AVR32_USART0)
#  define EXAMPLE_USART_RX_PIN          AVR32_USART0_RXD_0_0_PIN
#  define EXAMPLE_USART_RX_FUNCTION     AVR32_USART0_RXD_0_0_FUNCTION
#  define EXAMPLE_USART_TX_PIN          AVR32_USART0_TXD_0_0_PIN
#  define EXAMPLE_USART_TX_FUNCTION     AVR32_USART0_TXD_0_0_FUNCTION
#  define EXAMPLE_USART_IRQ             AVR32_USART0_IRQ
#  define EXAMPLE_USART_BAUDRATE        57600
#  define EXAMPLE_TARGET_PBACLK_FREQ_HZ FOSC0  // PBA clock target frequency, in Hz
//=============================================================================
struct ringbuffer * _buffer;

//=============================================================================
#if defined (__GNUC__)
__attribute__((__interrupt__))
#elif defined(__ICCAVR32__)
__interrupt
#endif
static void
usart_int_handler( void )
{
  int c;
  usart_read_char( EXAMPLE_USART, &c );
  ringbuffer_put(_buffer,c);
}
//=============================================================================
void
init_usart_interface( unsigned long fosc, struct ringbuffer * b )
{
  static const gpio_map_t USART_GPIO_MAP =
    {
      { AVR32_USART0_RXD_0_0_PIN, AVR32_USART0_RXD_0_0_FUNCTION },
      { AVR32_USART0_TXD_0_0_PIN, AVR32_USART0_TXD_0_0_FUNCTION } };
  // USART options.
  static const usart_options_t USART_OPTIONS =
    { .baudrate = 57600, .charlength = 8, .paritytype = USART_NO_PARITY,
        .stopbits = USART_1_STOPBIT, .channelmode = USART_NORMAL_CHMODE };
  gpio_enable_module( USART_GPIO_MAP, sizeof(USART_GPIO_MAP)
      / sizeof(USART_GPIO_MAP[0]) );

  usart_init_rs232( &AVR32_USART0, &USART_OPTIONS, fosc );

  _buffer = b;

  Disable_global_interrupt();
  INTC_register_interrupt( &usart_int_handler, AVR32_USART0_IRQ,
      AVR32_INTC_INT0);
  // Enable USART Rx interrupt.
  (&AVR32_USART0)->ier = AVR32_USART_IER_RXRDY_MASK;
  Enable_global_interrupt();
}





void
usart_interface_write(char const * string)
{
  usart_write_line(&AVR32_USART0, string);
}
