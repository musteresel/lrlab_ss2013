#ifndef LR_RINGBUFFER_H_
#define LR_RINGBUFFER_H_
//==============================================================================
/** The ringbuffer control structure
 *
 * This struct contains information to maintain the ringbuffer. This includes
 * several pointers to the start, the end, the write and the read position
 * inside the buffer. The struct itself does NOT contain a buffer!
 */
typedef struct __LR_ringbuffer
{
  int * start;
  int * end;
  int * write;
  int * read;
} LR_ringbuffer;




//==============================================================================
/** Put a datum into a ringbuffer
 *
 * This functions writes the int to the given buffer. There are NO checks
 * to test whether there is enough space in the buffer: If there is old data,
 * it will be overwritten!
 */
void LR_ringbuffer_put(LR_ringbuffer * b, int d);
/** Get a datum from a ringbuffer
 *
 * This functions returns a int from the given buffer only if there is data to
 * read.
 * On success 1 is returned, otherwise 0.
 */
char LR_ringbuffer_get(LR_ringbuffer * b, int * d);
/** Init a ringbuffer structure
 *
 * This function must be used to set up any ringbuffer before using it.
 * The given buffer must be alive during the whole live time of the buffer.
 * Pay attention to buffers which reside on the stack!
 */
void LR_ringbuffer_init(LR_ringbuffer * b, int * buffer, unsigned int size);



#endif /* LR_RINGBUFFER_H_ */
