/* include libraries */
#include <stdlib.h>
#include <avr/io.h>
#include "ringbuffer.h"

/**************************************************************************************/
/***************************** FUNCTIONS********************************************/
/**************************************************************************************/


void initqueue(ringbuffer *rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->num_elements = 0;
}

char isEmpty(ringbuffer *rb) {
    return (rb->num_elements == 0);
}

uint8_t firstValue(ringbuffer *rb){
    if(rb->num_elements > 0) {
        return rb->buffer[rb->head];
    }
    return 0; // Return 0 if the buffer is empty
}


void enqueue(ringbuffer *rb, uint8_t material) {
    if (rb->num_elements < BUFFER_SIZE) {
        rb->buffer[rb->tail] = material;
        rb->tail = (rb->tail + 1) % BUFFER_SIZE;
        rb->num_elements++;
    }
}

void dequeue(ringbuffer *rb) {
    if (rb->num_elements > 0){
        rb->head = (rb->head + 1 ) % BUFFER_SIZE;
        rb->num_elements--;
    }
}

int buffer_size(ringbuffer *rb) {
    return rb->num_elements;
}