#ifndef LR_USART_H_
#define LR_USART_H_
//==============================================================================
/** Include directives */
#include "LR_ringbuffer.h"





//==============================================================================
/** Initialize the USART interface and assign a ringbuffer.
 *
 * This functions initializes the USART hardware and assigns a ringbuffer
 * to store the received data.
 */
void LR_usart_init( unsigned long fosc, LR_ringbuffer * b);
/** Write a string through the usart interface
 *
 * This functions should be used to write a string through USART
 */
void LR_usart_write(char const * string);





#endif /* LR_USART_H_ */
