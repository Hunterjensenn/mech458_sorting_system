/* ringbuffer.h */


/*Type definitions*/
#define BUFFER_SIZE 48
typedef struct {
    uint8_t buffer[BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t num_elements;
} ringbuffer;
/* Constant declations */
/* 	E.g. #define STAGE0 0 ... sets STAGE0 = 0 

	Constant declarations can make you code much more readable, and will make life easier when required
	you need to enter in a constant value through many places in your code such as PI, or the GOLDEN NUMBER
	etc.
*/


/* Subroutine headers */
/* 	List the top line of your subroutine here. 
	WARNING: Make sure you put a semi-colon after each line, if you fail to do this it will make your life
	miserable to try and figure out where your bug is
*/
//MAKE sure there are semi colons at the end of these if you change them!!!

void	    initqueue	(ringbuffer *rb);
void 	    enqueue		(ringbuffer *rb, uint8_t material);
void 	    dequeue		(ringbuffer *rb);
uint8_t 	firstValue	(ringbuffer *rb);
char 	    isEmpty		(ringbuffer *rb);
int 	    buffer_size	(ringbuffer *rb);

