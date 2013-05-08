#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_


struct ringbuffer
{
  int * start;
  int * end;
  int * write;
  int * read;
};



void ringbuffer_put(struct ringbuffer * b, int d);
char ringbuffer_get(struct ringbuffer * b, int * d);
void ringbuffer_init(struct ringbuffer * b, int * buffer, unsigned int size);


#endif /* RINGBUFFER_H_ */
