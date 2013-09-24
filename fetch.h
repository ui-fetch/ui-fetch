
#ifndef __FETCH_H__
#define __FETCH_H__

#define MASK (BIT_1)

#define SYS_FREQ 			(80000000L)
#define CORE_TICKS_per_MS 	(SYS_FREQ/2/1000)
#define PB_CLK_FREQ 10000000L
#define TERMINAL_COUNT (PB_CLK_FREQ*1E-3 - 1)

// states of the motor
#define STOP 0
#define UP 1
#define DOWN 2

#define LED_1 BIT_0
#define BUTTON BIT_2
#define BTN_DOWN (PORTE & BIT_2)

#define DC_MOTOR_1 BIT_9
#define DC_MOTOR_2  //for cutting motor

#define PWM_IN_1 PORTDbits.RD8
#define PWM_IN_2  //for cutting mechanism
// frequency threshold
#define FREQ_THRESH 5000
#define LO_THRESH 3000
#define HI_THRESH 7000

#define ONESEC 1000

#define TACH_MIN 0
#define TACH_MAX 100000

// 50% motor speed
#define MOTORSPEED 5000

// this is to halt the arm gracefully (10% motor speed)
#define DECVALUE 1000

// initialize all variables, ports, clock
void initialization();
void msDelay( unsigned int delay_pd);


// reads the button value and switches LED accordingly
void readbutton(); 

// functions to handle pwm interpretation and motor control
//void ReadPWM(void);
void ReadPWM(int);
void InterpretPWM(void);
void WritePWM(void);
void haltArm(void);

#endif
