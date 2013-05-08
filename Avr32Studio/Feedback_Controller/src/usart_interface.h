#ifndef USART_H_
#define USART_H_

#include "ringbuffer.h"

void
init_usart_interface( unsigned long fosc, struct ringbuffer * buffer );

void
usart_interface_write(char const * string);

#endif /* USART_H_ */
