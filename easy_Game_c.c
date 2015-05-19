/*
==========================
==		Easy Game		==
==        ver.1         == 
==		 by Simon		==
==========================
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* SYSTEM */ 
#define SYSCFG 0x3ff0000
#define IOPMOD  ((volatile unsigned *) (SYSCFG + 0x5000))
#define IOPDATA ((volatile unsigned *) (SYSCFG + 0x5008))

/* Interrupt Registers */ 
#define INTMOD  ((volatile unsigned *) (SYSCFG + 0x4000))
#define INTPND  ((volatile unsigned *) (SYSCFG + 0x4004))
#define INTMASK ((volatile unsigned *) (SYSCFG + 0x4008))

/* Timer Registers */
#define TMOD    ((volatile unsigned *) (SYSCFG + 0x6000))
#define TDATA   ((volatile unsigned *) (SYSCFG + 0x6004))
#define START   1
#define STOP    0
#define ONE_SEC 0x02FAF080

/* LEDs */
#define LED_MASK 0xf0        // 1111 0000, port[7:4]
#define LED_D1 0x80          // 1000 0000
#define LED_D2 0x40          // 0100 0000
#define LED_D3 0x20          // 0010 0000
#define LED_D4 0x10          // 0001 0000
#define LED_ALL 0xF0         // 1111 0000
#define LED_NO  0x00         // 0000 0000

/* 7-Segment dlsplay */
#define SEG_MASK 0x1fc00     // 0001 1111 1100 0000 0000, port[16:10]
#define SEG_A (1 << 10)      // port10
#define SEG_B (1 << 11)      // port11
#define SEG_C (1 << 12)      // port12
#define SEG_D (1 << 13)      // port13
#define SEG_E (1 << 14)      // port14
#define SEG_F (1 << 16)      // port16
#define SEG_G (1 << 15)      // port15
#define DISP_0 (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)    // display 0
#define DISP_1 (SEG_B | SEG_C)                                    // display 1
#define DISP_2 (SEG_A | SEG_B | SEG_G | SEG_E | SEG_D)            // display 2
#define DISP_3 (SEG_A | SEG_B | SEG_G | SEG_C | SEG_D)            // display 3
#define DISP_4 (SEG_F | SEG_G | SEG_B | SEG_C)                    // display 4
#define DISP_5 (SEG_A | SEG_F | SEG_G | SEG_C | SEG_D)            // display 5
#define DISP_6 (SEG_A | SEG_F | SEG_E | SEG_D | SEG_C | SEG_G)    // display 6
#define DISP_7 (SEG_A | SEG_B | SEG_C)                            // display 7
#define DISP_8 (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)    // display 8
#define DISP_9 (SEG_A | SEG_B | SEG_C | SEG_D | SEG_G | SEG_F)    // display 9
#define DISP_A (SEG_A | SEG_B | SEG_C | SEG_G | SEG_E | SEG_F)    // display A
#define DISP_b (SEG_F | SEG_E | SEG_D | SEG_C | SEG_G        )    // display b
#define DISP_C (SEG_A | SEG_F | SEG_E | SEG_D)                    // display C
#define DISP_d (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G)            // display d
#define DISP_E (SEG_A | SEG_F | SEG_G | SEG_E | SEG_D)            // display E
#define DISP_F (SEG_A | SEG_F | SEG_G | SEG_E)                    // display F
#define DISP_H (SEG_F | SEG_E | SEG_G | SEG_B | SEG_C)            // display H
#define DISP_O (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)    // display O
#define DISP_L (SEG_F | SEG_E | SEG_D)                            // display L
#define o_DOWN (SEG_G | SEG_C | SEG_D | SEG_E)                    // norm o
#define o_UP   (SEG_A | SEG_B | SEG_G | SEG_F)                    // jump o 

/* DIP Switch */
#define SWITCH_MASK 0x0f     // 0000 1111, port[3:0]

/* -------------------------------------------------- */

void initialization(void);
void timer_switch(unsigned option);
void bobo(void);               		 // Process function
void bobo_jump(void);          		 // bobo jump!
void sleep(unsigned value);          // sleep function
void level_up(void);                 // level up Function
void change_level(void);
void game_OVER(void);

/* -------------------------------------------------- */

unsigned counter;
unsigned number_display[] = {DISP_0, DISP_1, DISP_2, DISP_3, DISP_4, DISP_5, DISP_6, DISP_7, DISP_8, DISP_9};
unsigned led_display[] = {LED_D1, LED_D2, LED_D3, LED_D4, LED_D4};
unsigned score, current_switch, prev_switch, level, time_interval;

/* -------------------------------------------------- */

/* Main Function */

void game_main(void){
	
	/* Initialization */
	initialization();
	
	/* Infinite Loop */
	while(1){
		/* Game Over */
		if((*IOPDATA & LED_MASK) == LED_MASK){       // All LED turn on
			timer_switch(0);	
			break;
		}
		/* Game processing */
		else{
			bobo();
		}
	}
	
	game_OVER();
	
}

/* -------------------------------------------------- */
/* timer IRQ Function */

void timer_irq(void){
	
	/* This function gengerate the LED randomly */
	
	unsigned ioData_LED = led_display[rand() % 4];    
	
	
	*IOPDATA |= ioData_LED;
	
	/* Recover the IRQ Interrupt */
	*INTPND |= 0xFFFFFFFF;      // Clear all pending bits
	*INTPND &= 0x00000400;      // Clear INTPND
}

/* -------------------------------------------------- */

/* Initialize Environment */

void initialization(void){
	
	/* Initialize IO */
	*IOPMOD |= LED_MASK;
	*IOPMOD |= SEG_MASK;
	*IOPMOD &= ~SWITCH_MASK;    // set switch to input IO
	*IOPDATA &= ~SEG_MASK; 
    *IOPDATA &= ~LED_MASK;
	
	/* Initialize Interrupt Registers */
	*INTMOD  &= 0x00000000;      // Set IRQ interrupt Mode 
	*INTMASK |= 0xFFFFFFFF;      // Set [10] Timer0 can be serviced
	*INTPND  |= 0xFFFFFFFF;      // Clear all pending bits
	
	/* Initialize Timer */
	*TMOD  &= 0x00000000;         // Initialize Timer
	*TDATA &= 0x00000000  ;       
	
	/* Setting Envirnment */	
	*INTMASK &= 0x00DFFBFF;
	*INTPND  &= 0x00000400;
	*TMOD    |= 0x00000001;
	time_interval = (4 * ONE_SEC);     // Time Interval is 3 sec.
	*TDATA   |= time_interval;         // Set timer trigger interval
	*IOPDATA |= o_DOWN;                // Set 7-seg Display Bobo
	current_switch = (*IOPDATA & SWITCH_MASK);
	score = 0;                  	   // reset score
	level = 0;
	srand(time(NULL));
	
	return ;
}

/* -------------------------------------------------- */

/* Timer Switch */

void timer_switch(unsigned option){
	
	switch(option){
		case 0 :                       // stop timer
			*TDATA &= 0x00000000;      // Clear INTPND
			break;
		case 1 :                       // start timer
			*TDATA &= 0x00000000;
			*TDATA |= time_interval;
			break;
		default : 
			break;
	}
	
	return ;
}

/* -------------------------------------------------- */

/* Process Function */

void bobo(void){
	
	unsigned diff_switch;
	
	/* Get Switch Action */
	
	prev_switch = current_switch;
	current_switch = (*IOPDATA & SWITCH_MASK);      // get swtich value
	diff_switch = prev_switch ^ current_switch;      // get switch difference
	
	switch(diff_switch){
		
		case 1 :
		case 2 :
		case 4 :
		case 8 :		
			/* match LED successfully */
			if((*IOPDATA & (led_display[diff_switch / 2])) != 0){     
		
				/* Clear the corresponded LED */
				*IOPDATA &= ~(led_display[diff_switch / 2]);
		       
		        /* Jump */
				bobo_jump();
		        
				/* Add score */	
				score ++;
				
				/* Level Up */
				level_up();
			}
				
			break;
		default :
			/* Do Nothing */    
			break;
	}
	
	return ;
}

/* -------------------------------------------------- */

/* BOBO Jump!! */

void bobo_jump(void){
	
	*IOPDATA &= ~SEG_MASK;
	*IOPDATA |= o_UP;
	
	sleep(700000);
	
	*IOPDATA &= ~SEG_MASK;
	*IOPDATA |= o_DOWN;
	
	return ;
}

/* -------------------------------------------------- */

/* Sleep Function */

void sleep(unsigned value){
	
	int i;
	
	for(i = 0; i < value; i++);
		/* for loop */
		
	return ;
}

/* -------------------------------------------------- */

/* Level Up Function */

void level_up(void){
	
	switch(score){
		
		case 3 :
		case 6 :
		case 12 :
		case 20 :
		case 30 :	
			level++;
			timer_switch(0);           // stop to service interrupt
			change_level();
			timer_switch(1); 
			break;
		default : 
			break;
	}
	
	return ;
} 

/* -------------------------------------------------- */

/* GAME OVER */

void game_OVER(void){
	
	int i;
	
	/* Stop to service interrupt */
	*INTMASK |= 0xFFFFFFFF;
	
	/* Bobo Bye bye */
	for(i = 0; i < 3; i++){
		
		*IOPDATA |= LED_ALL;
		*IOPDATA |= o_DOWN;
		sleep(1000000);
		
		*IOPDATA &= ~SEG_MASK;
		*IOPDATA &= ~LED_MASK;
		sleep(1000000);
	
	}
	
	while(1){
		
		*IOPDATA |= ((score / 10) << 4) ;         // LED Display 十位數之後 
		*IOPDATA |= number_display[score % 10];   // 7-SEG Display 個位數 
		sleep(1000000);
		
		*IOPDATA &= ~SEG_MASK;
		*IOPDATA &= ~LED_MASK;
		sleep(1000000);
		
	}	
}

void change_level(void){
	
	int i;
	
	/* Decrease trigger time */
	if(level < 4){
		time_interval -= ONE_SEC;
	}		
	else if(level == 4){
		time_interval = 0.5 * ONE_SEC;
	}
	else{
		time_interval = 0.1 * ONE_SEC;
	}
		
			
	/* display changing level */
	for(i = 0; i < 3; i++){
		*IOPDATA &= ~SEG_MASK;
		*IOPDATA |= number_display[level];     
		sleep(1500000);
		*IOPDATA &= ~SEG_MASK;
		sleep(1500000);
	}
	
					
	*IOPDATA |= o_DOWN;
	sleep(1000000);
	
	return ;
}
