
#include "ringbuffer.h"

void ringbuffer_put(struct ringbuffer * b, int d)
{
  *(b->write) = d;
  (b->write)++;
  if ((b->write) == (b->end))
  {
    b->write = b->start;
  }
}

char ringbuffer_get(struct ringbuffer * b, int * d)
{
  if ((b->read) == (b->write))
  {
    return 0;
  }
  *d = *(b->read);
  (b->read)++;
  if ((b->read) == (b->end))
  {
    b->read = b->start;
  }
  return 1;
}


void ringbuffer_init(struct ringbuffer * b, int * buffer, unsigned int size)
{
  b->start = buffer;
  b->end = buffer + size;
  b->write = b->start;
  b->read = b->start;
}
