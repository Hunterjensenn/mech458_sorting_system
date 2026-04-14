// READ EVERY SINGLE WORD IN THIS PIECE OF CODE...IF YOU DON'T YOU WILL NOT UNDERSTAND THIS!!!!!!!
// READ EVERY SINGLE WORD IN THIS PIECE OF CODE...IF YOU DON'T YOU WILL NOT UNDERSTAND THIS!!!!!!!
// READ EVERY SINGLE WORD IN THIS PIECE OF CODE...IF YOU DON'T YOU WILL NOT UNDERSTAND THIS!!!!!!!
// READ EVERY SINGLE WORD IN THIS PIECE OF CODE...IF YOU DON'T YOU WILL NOT UNDERSTAND THIS!!!!!!!
// READ EVERY SINGLE WORD IN THIS PIECE OF CODE...IF YOU DON'T YOU WILL NOT UNDERSTAND THIS!!!!!!!
// READ EVERY SINGLE WORD IN THIS PIECE OF CODE...IF YOU DON'T YOU WILL NOT UNDERSTAND THIS!!!!!!!

// Open up the document in START -> WinAVR -> AVR LibC -> User Manual -> avr/interrupt.h
// Chapter 15, in Full Manual... THIS HAS A LOT OF IMPORTANT INFO...I have mentioned this at least 3 times!!!

// For those that are still having major problems, I've seen about 1/3 of the class with major problems in
// code structure. If you are still having major problems with your code, it's time to do a VERY quick overhaul.
// I've provided a skeleton structure with an example using two input capture interrupts on PORTDA0 and A3
// Please try this in the debugger.

// Create a watch variable on STATE. To do this right click on the variable STATE and then
// Add Watch 'STATE'. You can see how the variable changes as you click on PINDA0 or PINDA3. Note that the interrupt
// catches a rising edge. You modify this to suit your needs.

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h> // the header of the general-purpose standard library of C programming language
#include <util/delay_basic.h>
#include "lcd.h"
#include "ringbuffer.h"

/*=========================DEFINES===========================*/
#define Brake_to_Vcc 0x0F
#define CCW_DC 0x07
#define black_plastic 0
#define steel 1
#define white_plastic 2
#define aluminum 3
#define aluminum_threshold 200
#define steel_threshold 700
#define white_threshold 934
#define BUFFER_SIZE 48

/* ====================Global Variables======================*/

volatile char STATE;
volatile uint8_t kill_flag = 0;
volatile uint8_t pause_flag = 0;
volatile uint8_t bucket_flag = 0;
volatile uint8_t reflective_flag = 0;
volatile uint8_t timer_expired_flag = 0;

volatile int count = 0;
volatile unsigned int ADC_result;
volatile unsigned int ADC_result_flag;
volatile unsigned int refl_low = 1023; //intital compare value for ADC conversion
volatile int high_temp = 0;
volatile int low_temp = 0;
volatile int curr_step = 0;
volatile char curr_tray = 0; //0 - black| 1 - Steel | 2 - White | 3 - Aluminum
volatile char next_tray = 0; //0 - black| 1 - Steel | 2 - White | 3 - Aluminum
volatile char stepper_tray = 0; // where stepper physically is
volatile char desired_tray = 0;
volatile char last_direction = 0; // 0 is CW | 1 is CCW
volatile uint8_t lookahead_flag = 0;
volatile char double_flag = 0; // to indicate if the next piece in the queue is the same as the current piece in the queue
//int step_array[] = {48, 6, 40, 5};
//int step_array[] = {53, 54, 46, 45};
int step_array[] = {54, 46, 45, 53};

// ======================NEW ONES TO TRY ====================================
// int accel_array[] = {2000, 1980, 1940, 1870, 1770, 1660, 1530, 1400, 1270, 1140, 1030, 930, 860, 820, 800};
// int accel_array[]= {1700, 1680, 1640, 1570, 1470, 1360, 1230, 1100, 970, 840, 730, 630, 560, 520, 500};
// int accel_array[] = {2000, 1743, 1536, 1348, 1193, 1038, 966, 915, 878, 850, 830, 815, 806, 801, 800};

//PRETTY GOOD AT HIGH TORQUE
// int accel_array1[] = {1700, 1699, 1696, 1685, 1659, 1603, 1495, 1323, 1100, 877, 705, 597, 541, 515, 504, 501, 500};

//NOT BAD AT LOW TORQUE 
// int accel_array1[] = {1600, 1599, 1596, 1586, 1563, 1511, 1412, 1254, 1050, 846, 688, 589, 537, 514, 504, 501, 500};


//NOT BAD
// int accel_array[] = {1700, 1680, 1640, 1560, 1460, 1350, 1220, 1080, 950, 840, 740, 660, 620, 600};

//GOOD ONE
// int accel_array1[] = {1700, 1680, 1620, 1520, 1400, 1260, 1100, 940, 800, 680, 580, 520, 500};

//FALL BACK
int accel_array1[] = {1800, 1780, 1720, 1620, 1500, 1360, 1200, 1040, 900, 780, 680, 620, 600};

// int accel_array1[] = {2000, 1980, 1920, 1820, 1700, 1560, 1400, 1240, 1100, 980, 880, 820, 800};

// int accel_array1[] = {1900, 1887, 1847, 1782, 1695, 1589, 1468, 1337, 1200, 1063, 932, 811, 705, 618, 553, 513, 500};

//COSINE TO TRY 15 STEPS
// int accel_array1[] = {1800, 1784, 1736, 1658, 1555, 1432, 1295, 1150, 1005, 868, 745, 642, 564, 516, 500};

//CAN USE WITH PRERELEASE AT 15
// int accel_array1[] = {1700, 1685, 1641, 1569, 1474, 1360, 1234, 1100, 966, 840, 726, 631, 559, 515, 500};

//SIGMOID TO TRY 
// int accel_array1[] = {1800, 1799, 1793, 1768, 1687, 1488, 1150, 812, 613, 532, 507, 501, 500};

//Seems sort of okay
// int accel_array1[] = {1500, 1490, 1460, 1420, 1350, 1280, 1190, 1100, 1000, 900, 810, 720, 650, 580, 540, 510, 500};

//Bit slow
// int accel_array[] = {1700, 1690, 1650, 1600, 1520, 1430, 1330, 1220, 1100, 980, 870, 770, 680, 600, 550, 510, 500};

//PRETTY GOOD - NEVERMIND
// int accel_array1[] = {1500, 1490, 1460, 1420, 1350, 1280, 1190, 1100, 1000, 900, 810, 720, 650, 580, 540, 510, 500};

//No
// int accel_array[] = {1600, 1590, 1550, 1480, 1390, 1290, 1170, 1050, 930, 810, 710, 620, 550, 510, 500};

// int accel_array[] = {1700, 1680, 1620, 1524, 1400, 1255, 1100, 945, 800, 676, 580, 520, 500};

//Pretty good at high torque
// int accel_array[] = {1700, 1688, 1654, 1599, 1524, 1433, 1330, 1217, 1100, 983, 870, 767, 676, 601, 546, 512, 500};

// typedef struct {
//     int *delays;
//     int size;
// } AccelProfile;

// int accel_array1[] = {1700, 1650, 1560, 1430, 1280, 1120, 960, 810, 690, 600, 550, 520, 500};
// int accel_array2[] = {1700, 1699, 1696, 1685, 1659, 1603, 1495, 1323, 1100, 877, 705, 597, 541, 515, 504, 501, 500};

// Create an array of your profiles
// AccelProfile profiles[2] = {
//     {accel_array1, sizeof(accel_array1) / sizeof(accel_array1[0])},
//     {accel_array2, sizeof(accel_array2) / sizeof(accel_array2[0])}
// };

// Global index to track which profile is active
// uint8_t current_profile_index = 0;

//ORIGINAL ONE USE THIS FOR LOW TORQUE
// int accel_array1[] = {1700, 1650, 1560, 1430, 1280, 1120, 960, 810, 690, 600, 550, 520, 500};
//USE THIS AT HIGH TORQUE
// int accel_array2[] = {1700, 1699, 1696, 1685, 1659, 1603, 1495, 1323, 1100, 877, 705, 597, 541, 515, 504, 501, 500};
//int accel_array[] = {170, 167, 162, 154, 144, 132, 118, 104, 90, 78, 68, 60, 55, 52, 50};
//int accel_array[] = {180, 177, 172, 164, 152, 138, 123, 107, 92, 78, 66, 58, 52, 50};
//int accel_array[] = {200, 195, 185, 173, 160, 145, 128, 112, 95, 80, 68, 58, 52, 50};
/* 50_steps_90deg: 50 steps */
// int accel_array[] = {1550, 1273, 1040, 870, 658, 610, 576, 552, 534, 520, 511, 504, 501, 500};
// int accel_array[] = {
//     2000, 1970, 1920, 1850, 1760, 1650, 1520, 1380, 1230, 1080,
//     940, 810, 700, 610, 550, 520,
//     500
// };

//LAST ONE TESTED 
// int accel_array[] = {1700, 1274, 1043, 871, 686, 629, 590, 562, 541, 526, 514, 507, 502, 500};
//  500,
//      500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 
   //  500, 500, 501, 504, 511, 520, 534, 552, 576, 610, 658, 870, 1040, 1273, 1550};
// float accel_array[] = {14976.4, 14939.6, 14850.8, 14650.6, 14240.5, 13503.4, 12375, 10931.7, 9388.31, 
//     7981.51, 6854.06, 6032.6, 5472.33, 5106.54, 4874.31, 4729.46, 4640.08, 4500, 4500, 4500, 4500, 
//     4500, 4500, 4500, 4500, 4500, 4500, 4500, 4500, 4500, 4500, 4500, 4500, 4640.08, 4729.46, 4874.31, 
//     5106.54, 5472.33, 6032.6, 6854.06, 7981.51, 9388.31, 10931.7, 12375, 13503.4, 14240.5, 14650.6, 14850.8, 
//     14939.6, 14976.4};
//int accel_array[] = {200, 197,190,178,164,146,130,114,96,82,70,63,60};	
//int accel_array[] = {170, 167, 162, 154, 143, 130, 115, 100, 85, 72, 60, 53, 51, 50};
//int accel_array[] = {250, 247, 237, 221, 200, 176, 150, 124, 100, 79, 63, 53, 50};
//int accel_array[] = {250, 242, 230, 212, 188, 160, 132, 108, 90, 78, 72, 68, 65, 62, 60};
int sorted_array[] = {0, 0, 0, 0};
int num_sorted = 0;
	
int accel_size = sizeof(accel_array1)/sizeof(accel_array1[0]);

// int step_90_size = sizeof(step_delay_90)/sizeof(step_delay_90[0]);
// int step_180_size = sizeof(step_delay_180)/sizeof(step_delay_180[0]);
// int step_270_size = sizeof(step_delay_270)/sizeof(step_delay_270[0]);
// int step_360_size = sizeof(step_delay_360)/sizeof(step_delay_360[0]);
//Count for each material to be sorted
/*
volatile uint8_t aluminum = 0; 
volatile uint8_t steel= 0; 
volatile uint8_t white_plastic = 0; 
volatile uint8_t black_plastic = 0;
*/
volatile unsigned char is_killed = 0; //To trigger ramp down functionality
volatile unsigned char is_paused = 0; //To trigger pause and display functionality

ringbuffer rb; //ring buffer declaration





/*===================Function Declarations===================*/
void mTimer(int count);
void PWM_setup(void);
void ADC_init(void);
void CW_stepper(int steps);
void CCW_stepper(int steps);
void ACC_CW_STEPPER(int steps);
void ACC_CCW_STEPPER(int steps);
void killTimer_init(void);

void CW_STEPPER_90();
void CW_STEPPER_180();
void CW_STEPPER_270();
void CW_STEPPER_360();
void CCW_STEPPER_90();
void CCW_STEPPER_180();
void CCW_STEPPER_270();
void CCW_STEPPER_360();

int main(int argc, char *argv[]){
	
	CLKPR = 0x80;
	CLKPR = 0x01;		//  sets system clock to 8MHz

	TCCR1B |= _BV(CS11); //Prescale Timer 1 by 8 making it run at 1MHz
	
	STATE = 0;


	cli();		// Disables all interrupts
	
	PWM_setup();
	ADC_init();

	//Initialize the ring buffer
	initqueue(&rb);


	DDRA = 0x4F;		//Register A to output for stepper motor
	DDRD = 0b11110000;	// Going to set up INT2 & INT3 on PORTD
	DDRC = 0xFF;		// just use as a display
	DDRL = 0xFF;
	DDRE = 0x00; //for EX sensor
	DDRJ = 0x00; //Register J input for Hall Effect
	


	// Set up the Interrupt 0,3 options
	//External Interrupt Control Register A - EICRA (pg 110 and under the EXT_INT tab to the right
	// Set Interrupt sense control to catch a rising edge
	EICRA |= _BV(ISC21); //falling edge wind down //| _BV(ISC20);
	EICRA |= _BV(ISC11); //falling edge pause //| _BV(ISC30);
	EICRA |= _BV(ISC01)  | _BV(ISC00); //rising edge OR
	EICRB |= _BV(ISC41); //falling edge EX
	

	// See page 112 - EIFR External Interrupt Flags...notice how they reset on their own in 'C'...not in assembly
	EIMSK |= 0x17;
	
	//Initialize LCD module
	InitLCD(LS_BLINK|LS_ULINE);
	
	LCDClear();

	LCDWriteStringXY(0,0,"Counter ");
	LCDWriteIntXY(9,0,count, 5);
	
	// Enable all interrupts
	sei();	// Note this sets the Global Enable for all interrupts
	
	//Home stepper on black
	while((PINA & 0x80) == 0x80){
		CW_stepper(1);
	}
	
	CCW_stepper(3);
	//Initialize motor counter clockwise

	PORTB = CCW_DC;
	OCR0A = 175;

	goto POLLING_STAGE;

	// POLLING STATE
	POLLING_STAGE:
	/*
	LCDClear();
	LCDWriteStringXY(0,0,"Size ");
	LCDWriteIntXY(9,0,size(&head,&tail), 4);
	*/
	if(kill_flag){
        kill_flag = 0;
        LCDClear();
        LCDWriteStringXY(0,0,"**RAMPING DOWN**");
        goto POLLING_STAGE;
        
	}
	if(is_killed && timer_expired_flag){
        PORTB = Brake_to_Vcc;
        LCDClear();
        LCDWriteStringXY(0,0," *** KILLED ***");
        LCDWriteStringXY(0,1,"A");
        LCDWriteIntXY(1,1,sorted_array[aluminum], 2);
        LCDWriteStringXY(4,1,"S");
        LCDWriteIntXY(5,1,sorted_array[steel], 2);
        LCDWriteStringXY(8,1,"W");
        LCDWriteIntXY(9,1,sorted_array[white_plastic], 2);
        LCDWriteStringXY(12,1,"B:");
        LCDWriteIntXY(13,1,sorted_array[black_plastic], 2);
        while(1);
    }

	if(pause_flag){
		pause_flag = 0;
		goto PAUSE_DISPLAY_STAGE;
	}


	if(lookahead_flag){
		lookahead_flag = 0;
		char next = rb.buffer[rb.head];
		if(next != stepper_tray){
			if((next - stepper_tray == 3)||(next - stepper_tray == -1)){
				ACC_CW_STEPPER(50);
				last_direction = 0;
			}else if((next - stepper_tray == -3)||(next - stepper_tray == 1)){
				ACC_CCW_STEPPER(50);
				last_direction = 1;
			}else if(last_direction == 0){
				ACC_CW_STEPPER(100);
				last_direction = 0;
			}else{
				ACC_CCW_STEPPER(100);
				last_direction = 1;
			}
			curr_tray = next;
			stepper_tray = next;
		}
	}


	if(num_sorted > 3){
		OCR0A = 130; // Switches to accel_array2
	}


	if(bucket_flag > 0){
		bucket_flag--;
		goto BUCKET_STAGE;
	}

	
	goto POLLING_STAGE;

	

	PAUSE_DISPLAY_STAGE:
	// Do whatever is necessary HERE
	//PORTC = 0x01; // Just output pretty lights know you made it here
	//Reset the state variable
	while(is_paused){
		PORTB = Brake_to_Vcc;
		// unsigned int queue_size = size(&head, &tail);
		unsigned int queue_size = buffer_size(&rb);
		LCDClear();
		
		
		LCDWriteStringXY(0,0,"Al St Wh Bl PS ");
		LCDWriteIntXY(0,1,sorted_array[aluminum], 2);
		LCDWriteIntXY(3,1,sorted_array[steel], 2);
		LCDWriteIntXY(6,1,sorted_array[white_plastic], 2);
		LCDWriteIntXY(9,1,sorted_array[black_plastic], 2);
		LCDWriteIntXY(12,1,queue_size, 2);
		
		mTimer(20000);

	}
	PORTB = CCW_DC;
	//STATE = 0;
	goto POLLING_STAGE;

	REFLECTIVE_STAGE:
	
	goto POLLING_STAGE;
	
	BUCKET_STAGE:
	/*LCDClear();
	LCDWriteStringXY(0,0,"ADC Value ");
	LCDWriteIntXY(4,1,tail->e.material, 5);
	mTimer(500);
	*/
	
	//int size = buffer_size(&rb);

	if(double_flag){
		double_flag = 0;
		mTimer(4000);
	}
	if(rb.buffer[rb.head] == curr_tray){
		//mTimer(100);
		PORTB = CCW_DC;
		//mTimer(500);

	}else{
		//PORTB = Brake_to_Vcc;
		//========================================//
		// PUT THE BRAKE HERE INSTEAD OF ISR
		//PORTB = Brake_to_Vcc;
		//mTimer(100);
			if((rb.buffer[rb.head]-curr_tray == 3)||(rb.buffer[rb.head]-curr_tray == -1)){
				ACC_CW_STEPPER(50);
                // CW_stepper(50);
				last_direction = 0;
			}else if((rb.buffer[rb.head]-curr_tray == -3)||(rb.buffer[rb.head]-curr_tray == 1)){
				ACC_CCW_STEPPER(50);
                // CCW_stepper(50);
				last_direction = 1;
			}else if(last_direction == 0){
				ACC_CW_STEPPER(100);
                // CW_stepper(100);
				last_direction = 0;	
			}else{
				ACC_CCW_STEPPER(100);
                // CCW_stepper(100);
				last_direction = 1;
			}
		}
	

	

	sorted_array[rb.buffer[rb.head]]++;
	curr_tray = rb.buffer[rb.head];
	// stepper_tray = curr_tray;
	dequeue(&rb);
	

	if(buffer_size(&rb) > 0){
		char next = rb.buffer[rb.head];
		if(next == curr_tray){
			double_flag = 1;
		}
	}
	//mTimer(350);
	//PORTB = CCW_DC;


	if(isEmpty(&rb)){
		STATE = 0;
		goto POLLING_STAGE;
	}
	
	
	
	//mTimer(500);
	// Do whatever is necessary HERE
	//PORTC = 0x08;
	//Reset the state variable
	//STATE = 0;
	goto POLLING_STAGE;
	
	END:
	// The closing STATE ... how would you get here?
	PORTC = 0xF0;	// Indicates this state is active
	// Stop everything here...'MAKE SAFE'
	return(0);

}

//Optical sensor OR interrupt routine
ISR(INT0_vect){
	//sei();
	// initialize the ADC, start one conversion at the beginning ==========
	ADCSRA |= _BV(ADSC);
	//initLink(&newLink);
	refl_low = 1023;
	//reflective_flag = 1;
}

/* Set up the External Interrupt 2 Vector 
Use this ISR as the ramp down trigger\*/
ISR(INT2_vect){
    if(is_killed){
        return; // If already killed, ignore further presses
    }
	if((PIND & 0x04) == 0x00){
		mTimer(2000);//Debounc button

		is_killed = 1;
        killTimer_init(); // Start the kill timer to ramp down the system
		//Go to polling stage
		kill_flag = 1;

		while((PIND & 0x04) == 0x00);
		mTimer(2000);
	}
}

/* Use this ISR for the pause and display trigger.*/
ISR(INT1_vect){//Debounc button
	//PORTL = 0x80;
	//mTimer(1000);
	//switch direction
	if((PIND & 0x02) == 0x00){
		mTimer(2000);
		//PORTB = Brake_to_Vcc;
		is_paused ^= 1;
		//Go to pause display stage
		pause_flag ^= 1;
		while((PIND & 0x02) == 0x00);
		mTimer(2000);
	}

			

	/* Toggle PORTC bit 3 */
}



//End of line interrupt
ISR(INT4_vect)
{
	//sei();

	if(double_flag){
		// double_flag = 0;
		// start_flag = 0;
		bucket_flag++;
		return;
	}
	PORTB = Brake_to_Vcc;
	bucket_flag++;

	
	//Go to bucket stage
	//bucket_flag = 1;
}


// the interrupt will be trigured if the ADC is done ========================
ISR(ADC_vect)
{
	//low_temp = ADCL;
	//high_temp = ADCH;
	//Acquire 10-bit result
	//ADC_result = (low_temp | (high_temp << 8));

	
	if((PIND & 0x01) == 0x01){
		if(ADC < refl_low){
			refl_low = ADC;
		}
		ADCSRA |= _BV(ADSC);
	}else{
		// /* TEST BOTH TO SEE WHICH ONE OPERATES FASTER*/
		/*
		switch(refl_low){
				case 0 ... aluminum_threshold:
					enqueue(&rb, aluminum);
					break;
				case aluminum_threshold+1 ... steel_threshold:
					enqueue(&rb, steel);
					break;
				case steel_threshold+1 ... white_threshold:
					enqueue(&rb, white_plastic);
					break;
				default:
					enqueue(&rb, black_plastic);
		}
		*/
		if(isEmpty(&rb)){
            lookahead_flag = 1;
        }
		if(refl_low < aluminum_threshold){
			//Aluminum piece identified
			//newLink->e.material = aluminum;
			enqueue(&rb, aluminum);
			}else if(refl_low <steel_threshold ){
			//steel piece identified
			//newLink->e.material = steel;
			enqueue(&rb, steel);
			}else if(refl_low  < white_threshold){
			//white plastic piece identified
			// newLink->e.material = white_plastic;
			enqueue(&rb, white_plastic);
			}else{
			//black plastic piece identified
			//newLink->e.material = black_plastic;
			enqueue(&rb, black_plastic);
		}
		num_sorted++;

		//Adds the new link to the back of the queue
		// enqueue(&head, &tail, &newLink);
}
	/*
	ADC_result = ADC;
	*/
	//ADC_result_flag = 1;
}

ISR(TIMER3_COMPA_vect){
    static uint8_t hit_count = 0;
    hit_count++;
    if(hit_count >= 2){
        timer_expired_flag = 1;
        TCCR3B &= ~(_BV(CS32) | _BV(CS30)); // stop timer
        hit_count = 0;
    }
}


// If an unexpected interrupt occurs (interrupt is enabled and no handler is installed,
// which usually indicates a bug), then the default action is to reset the device by jumping
// to the reset vector. You can override this by supplying a function named BADISR_vect which
// should be defined with ISR() as such. (The name BADISR_vect is actually an alias for __vector_default.
// The latter must be used inside assembly code in case <avr/interrupt.h> is not included.
ISR(BADISR_vect)
{
	while(1){
		PORTL = 0xF0;
		mTimer(20000);
		PORTL = 0x00;// user code here
	}
}



/*
Description: Implements a timer delay in tenths of milliseconds corresponding to count variable
Parameters-
			count: integer count of number of tenths ofmilliseconds to delay.
Return - 
			Nothing
*/
void mTimer(int count){

	int i = 0; //Integer comparator for loop count

	TCCR1B |= _BV(WGM12); //Sets waveform generation mode of Timer 1 to CTC

	OCR1A = 0x0A; //Output compare register to 10 (For 0.01ms)

	TCNT1 = 0x0000; //Initialize count to 0

	//TIMSK1 |= 0x02; //Enables output compare interrupt enable

	TIFR1 |= _BV(OCF1A); //Clear the timer interrupt flag and begin new timing

	//Poll timer to determine when the timer has reached 0x03E8
	while(i < count){
		
		if((TIFR1 & 0x02) == 0x02){
			
			TIFR1 |= _BV(OCF1A); // Clear interrupt flag by writing a ONE to the bit
			
			i++; //increment loop count
			
		}
	}
	return;

}

void PWM_setup(void){
	/*=================== PWM SETUP =========================*/
	TCCR0A |= _BV(WGM01); // Set Timer0 to Fast PWM
	TCCR0A |= _BV(WGM00); // Set Timer0 to Fast PWM
	TCCR0A |= _BV(COM0A1); // Clear OC0A on compare match
	TCCR0B |= _BV(CS01); // Prescaler to 8 to get 3.9kHz Fast PWM
	DDRB = 0xFF; //Set all pins on PORTB to output

}

void ADC_init(void){
	/*==================== ADC SETUP ===============================*/
	// config ADC =========================================================
	// by default, the ADC input (analog input is set to be ADC0 / PORTF0
	ADCSRA |= _BV(ADEN); // enable ADC
	ADCSRA |= _BV(ADIE); // enable interrupt of ADC
	//8 8/8 = 1MHz
	ADCSRA |= _BV(ADPS1) | _BV(ADPS0);
	//16 8/16 = 500kHz
	// ADCSRA |= _BV(ADPS2);
	//32 8/32 = 250kHz
	// ADCSRA |= _BV(ADPS2) | _BV(ADPS0);
	//64 8/64 = 125kHz
	// ADCSRA |= _BV(ADPS2) | _BV(ADPS1);
	//128 8/128 = 62.5kHz
	// ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
	ADMUX |= _BV(REFS0); // Set voltage reference as AVCC with external capacitor at AREF pin 
									  // and left adjust the register
}

void killTimer_init(void){
    TCCR3A = 0x00;
    TCCR3B = 0x00;
    TCCR3B |= _BV(WGM32);              // CTC mode
    OCR3A = 19531; //      39062               // ~7.5 seconds per hit, 2 hits = 15s
    TCNT3 = 0x0000;
    TIFR3 |= _BV(OCF3A);               // clear any pending flag
    TIMSK3 |= _BV(OCIE3A);             // enable compare interrupt
    TCCR3B |= _BV(CS32) | _BV(CS30);  // 1024 prescaler, starts timer
}
/*Parameters-
			steps: integer number of steps to rotate (200 steps = 360 degrees).
Return - 
			Nothing
*/
void CW_stepper(int steps){
	
	
	for(int i = 0; i < steps; i++){
		
		curr_step++;
		
		if(curr_step > 4){
			curr_step = 1;
		}
		
		PORTA = step_array[curr_step - 1];
		mTimer(2000);
	}
    PORTB = CCW_DC;
	
}

/*======================================
WRAPPED THESE WITH INTERRUPT ENABLE/DISABLE*/
void ACC_CW_STEPPER(int steps){
	//ADCSRA &= ~_BV(ADIE);
	//cli();
	//Accelerate Motor

	// int size = profiles[current_profile_index].size;
	// int *arr = profiles[current_profile_index].delays;

	for(int i = 0; i< accel_size; i++){
		curr_step++;
			
		if(curr_step > 4){
			curr_step = 1;
		}
		PORTA = step_array[curr_step - 1];
		mTimer(accel_array1[i]);
	}
	//Fastest Speed
	for(int i = 0; i < steps - accel_size*2; i++){
			
		curr_step++;
			
		if(curr_step > 4){
			curr_step = 1;
		}
		
		if(i == (steps - accel_size*2-15)){
			//ADCSRA |= _BV(ADIE);
			//sei();
			PORTB = CCW_DC;
		}
			
		PORTA = step_array[curr_step - 1];
		mTimer(accel_array1[accel_size - 1]);
	}
	//sei();
	//PORTB = CCW_DC;
	//Decelerate Motor
	for(int i = 0; i< accel_size; i++){
		curr_step++;
			
		if(curr_step > 4){
			curr_step = 1;
		}
		PORTA = step_array[curr_step - 1];
		mTimer(accel_array1[(accel_size - 1) -i]);
	}

}

/*
Description: Rotates a stepper motor connected to PORTA counterclockwise a number of steps 
Parameters-
			steps: integer number of steps to rotate (200 steps = 360 degrees).
Return - 
			Nothing
*/
void CCW_stepper(int steps){
	
	for(int i = 0; i < steps; i++){
		curr_step--;
		
		if(curr_step < 1){
			curr_step = 4;
		}
		
		PORTA = step_array[curr_step - 1];
		mTimer(2000);
	}
    PORTB = CCW_DC;
}

void ACC_CCW_STEPPER(int steps){
	//ADCSRA &= ~_BV(ADIE);
	//cli();
	//Accelerate Motor
	// int size = profiles[current_profile_index].size;
	// int *arr = profiles[current_profile_index].delays;

	for(int i = 0; i< accel_size; i++){
		
		curr_step--;
		
		if(curr_step < 1){
			curr_step = 4;
		}
		PORTA = step_array[curr_step - 1];
		mTimer(accel_array1[i]);
	}
	//Fastest Speed
	for(int i = 0; i < steps-accel_size*2; i++){
			
		
		curr_step--;
		
		if(curr_step < 1){
			curr_step = 4;
		}
		
		if(i == (steps - accel_size*2-15)){
			//ADCSRA |= _BV(ADIE);'
			//sei();
			PORTB = CCW_DC;
		}
		PORTA = step_array[curr_step - 1];
		mTimer(accel_array1[accel_size-1]);
	}
	//sei();
	//PORTB = CCW_DC;
	//Decelerate Motor
	for(int i = 0; i< accel_size; i++){
		
		curr_step--;
		
		if(curr_step < 1){
			curr_step = 4;
		}
		PORTA = step_array[curr_step - 1];
		mTimer(accel_array1[(accel_size - 1) -i]);
	}

    
}

/*======================================
THE BELOW FUNCTIONS ARE FOR THE SPECIFIC ANGLE ROTATIONS WITH ACCELERATION AND DECEL
void CW_STEPPER_90(){
	
	for(int i=0; i<50; i++){
		curr_step++;
		
		if(curr_step > 4){
			curr_step = 1;
		}
		PORTA = step_array[curr_step - 1];
		mTimer(step_delay_90[i]);
		
		if(i == step_90_size - 30){
			PORTB = CCW_DC;
		}
	}
}
void CW_STEPPER_180(){
	
	for(int i=0; i<100; i++){
		curr_step++;
		
		if(curr_step > 4){
			curr_step = 1;
		}
		PORTA = step_array[curr_step - 1];
		mTimer(step_delay_180[i]);
		
		if(i == step_180_size - 30){
			PORTB = CCW_DC;
		}
	}
}

void CW_STEPPER_270(){
	
	
	for(int i=0; i<150; i++){
		curr_step++;
		
		if(curr_step > 4){
			curr_step = 1;
		}
		PORTA = step_array[curr_step - 1];
		mTimer(step_delay_270[i]);
		
		if(i == step_270_size - 30){
			PORTB = CCW_DC;
		}
	}
	
}
void CW_STEPPER_360(){
	
	for(int i=0; i<200; i++){
		curr_step++;
		
		if(curr_step > 4){
			curr_step = 1;
		}
		PORTA = step_array[curr_step - 1];
		mTimer(step_delay_360[i]);
		
		if(i == step_360_size - 30){
			PORTB = CCW_DC;
		}
	}
	
}
void CCW_STEPPER_90(){
	
		for(int i = 0; i< 50; i++){
			
			curr_step--;
			
			if(curr_step < 1){
				curr_step = 4;
			}
			PORTA = step_array[curr_step - 1];
			mTimer(step_delay_90[i]);
			
			if(i == step_90_size - 30){
				PORTB = CCW_DC;
				
			}
		}
		
}

void CCW_STEPPER_180(){
	for(int i = 0; i< 100; i++){
		
		curr_step--;
		
		if(curr_step < 1){
			curr_step = 4;
		}
		PORTA = step_array[curr_step - 1];
		mTimer(step_delay_180[i]);
		
		if(i == step_180_size - 30){
			PORTB = CCW_DC;
			
		}
	}
	
}
void CCW_STEPPER_270(){
	
	for(int i = 0; i< 150; i++){
		
		curr_step--;
		
		if(curr_step < 1){
			curr_step = 4;
		}
		PORTA = step_array[curr_step - 1];
		mTimer(step_delay_270[i]);
		
		if(i == step_270_size - 30){
			PORTB = CCW_DC;
			
		}
	}
}
void CCW_STEPPER_360(){
	for(int i = 0; i< 200; i++){
		
		curr_step--;
		
		if(curr_step < 1){
			curr_step = 4;
		}
		PORTA = step_array[curr_step - 1];
		mTimer(step_delay_360[i]);
		
		if(i == step_360_size - 30){
			PORTB = CCW_DC;
			
		}
	}
	
}
	*/