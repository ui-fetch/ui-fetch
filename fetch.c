
#include <stdio.h>
#include <string.h> 
#include <plib.h>
#include "config_bits.h"
#include "fetch.h"

// These variables are used to read in the PWM Signal
float _risetime; 
float _falltime;
float _hitime;

// These variables are used to determine when the start
// of a PWM signal occurs.
float _store;
float _ftore;

// As our team does not have the specific DC motor we will be using,
// we created a fake tachometer counter to determine how far the motor 
// should be extending/unextending the arm.
float _faketach;

// These variables determine whether the motor should be running, and
// which direction it should be going.
int _motorState;
int _motorGo;

//////////////////////////////////////////////////////////////////////////////////
/// name: initialization
/// params: none
/// return: void
/// desc: contains the configuration of hardware and clock, as well as
///       the initialization of variables
void intialization(void)
{
    // button input pin
    PORTSetPinsDigitalIn(IOPORT_E, BUTTON); 
    // LED output pin
    PORTSetPinsDigitalOut(IOPORT_E, LED_1); 
    // motor direction pin
    PORTSetPinsDigitalOut(IOPORT_B, DC_MOTOR_1);  
	

    // configuring timer  
    OpenTimer2(T2_ON | T2_PS_1_1, TERMINAL_COUNT);
    OpenOC4(OC_ON | OC_TIMER_MODE16 | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0x80, 0x60); 
  
    // start these values at 0
    _risetime = 0;
    _falltime = 0;
    _hitime = 0;
  
    // motor starts stationary
    _motorState = STOP;	
    _motorGo = 0;
  
    // start arm fully retracted
    _faketach = 0;
  
    // set these values to 1, as they should start out not
    // equal to _risetime and _falltime for initial pwm read
    _store = 1;
    _ftore = 1;
  
}

//////////////////////////////////////////////////////////////////////////////////
/// name: main
/// params: none
/// return: int
/// desc: calls the initialization of variables/hardware, and contains
///       the main loop with all active functionality
int main(void)
{
    // initialize ports for buttons and LED, PWM receiving, and PWM writing
    intialization();
  
    while ( ! NULL ) 
	{
	    // read in button value (push on, push off)
	    readbutton();
      
	    // read incoming PWM signal for arm extension
	    ReadPWM(PWM_IN_1);

	    // read incoming PWM signal for arm extension
	    // ReadPWM(PWM_IN_2);
      
	    // set the motor state based on PWM frequency
	    InterpretPWM();
      
	    // write to the motor depending on current state
	    WritePWM();
	} 

}

//////////////////////////////////////////////////////////////////////////////////
/// name: readbutton
/// params: none
/// return: void
/// desc: detects the pushing of the LED button. since the button is
///       push-on/push-off, each button press will flip the state of
///       the LED. the loops are to ensure that the button is pressed
///       for at least two iterations of msDelay (making sure a it is
///       intentionally pushed)
void readbutton(void)
{
    int counter;  
    int btn1 = BTN_DOWN;
  
    // if button is pressed
    if ( BTN_DOWN == 0 )	
	{
	    btn1 = BTN_DOWN;
	    // ensure button is pushed for long enough
	    for ( counter = 1; counter < 2; msDelay(20))
		if(BTN_DOWN == 0) 
		    counter++;
      
	    // reads till btn1 is no longer set
	    while ( BTN_DOWN == 0 ) 
		{}
      
	    // ensure button is released for long enough
	    for ( counter--; counter != 0; msDelay(20) )
		if( BTN_DOWN != 0 ) 
		    counter--;
      
	    // flip the state of the LED
	    LATEINV = LED_1;	
	}
}


/*	Code provided by Eric Johnston
 */
//////////////////////////////////////////////////////////////////////////////////
/// name: msDelay
/// params: unsigned int
/// return: void
/// desc: provides a pause/sleep for duration specified by input param
///       code provided by Eric Johnston
void msDelay(unsigned int delay_pd)
{
    unsigned int tWait;
    unsigned int tStart;
    
    // read current core timer
    tStart = ReadCoreTimer(); 
    
    // set time to be current time plus specified offset
    tWait = tStart + (CORE_TICKS_per_MS * delay_pd);  
    
    // wait for the time to pass
    while ( ReadCoreTimer() < tWait )
	{}
    
}

//////////////////////////////////////////////////////////////////////////////////
/// name: ReadPWM
/// params: int pwmSignal
/// return: void
/// desc: handles the detection of incoming pwm signals. if a rising edge
///       is detected, the value on Timer2 is stored in _risetime. if a
///       falling edge is detected, the value on Timer2 is stored in _falltime,
///       and the overall time the signal was high is stored in _hitime.
///       
///       since the duration of the high and low signals are not equal, the
///       _risetime and _falltime should only be stored on the first instance
///       of rising/falling edge. this is done by storing the current risetime
///       on low signal, and storing the falltime on high signal, and only reading
///       the current signal if the stored values are not equal to the current
///       values. this ensures that the signal is only read when transitioning
///       between high and low signals, which means we are only detecting the
///       rising and falling edges of the pwm
void ReadPWM(int pwmSignal)
{
    // signal is high
    if ( (pwmSignal == 1) && (_falltime != _ftore) ) 
	{
	    // store current fall time
	    _ftore = _falltime;
      
	    // read the timer at the first rise
	    _risetime = ReadTimer2();	
	}
  
    // signal is low
    else if ( (pwmSignal == 0) && (_risetime != _store) )
	{
	    // store current rise time
	    _store = _risetime;
      
	    // read the timer at the first fall
	    _falltime = ReadTimer2();
      
	    // calculate how long the signal was high
	    if ( _risetime < _falltime )
		{
		    _hitime = _falltime - _risetime;
		}
	}

    // populated _hitime means we have a full pulse, so set flag that tells motor to go
    if ( _hitime )
	{
	    _motorGo = 1;
	}
}

//////////////////////////////////////////////////////////////////////////////////
/// name: InterpretPWM
/// params: none
/// return: void
/// desc: sets the state of the motor depending on the pwm signal being read.
///       if the _hitime (duration the signal was high) is greater than a 
///       predefined threshold, the motor state is set to UP.
///       if the _hitime is less than another predefined threshold, the motor
///       state is set to DOWN.
///       when either of these states are changed, the directional bit of the
///       motor is flipped.
///       if the _hitime is in between the high and low frequency thresholds,
///       the motor state is set to STOP.
///       the current state of the motor is first checked, as we don't want to
///       needlessly repeat these operations. only need to flip the state and
///       the bit when the pwm signal changes range
void InterpretPWM(void)
{
    // if the _motor is moving, detect the frequency and translate it into 
    // which way it should be going
    if ( _motorGo != 0 )	
	{
	    // high frequency detected
	    if ( (_motorState != UP) && (_hitime > HI_THRESH) && (_faketach > TACH_MIN) )
		{
		    // set the _motor state up
		    _motorState = UP;
	  
		    // switch directional bit
		    LATBSET = DC_MOTOR_1;
		}
      
	    // low frequency detected
	    else if ( (_motorState != DOWN) && (_hitime < LO_THRESH) && (_faketach < TACH_MAX) )
		{
		    // set _motor state down
		    _motorState = DOWN;
	  
		    // switch directional bit
		    LATBCLR = DC_MOTOR_1;
		}
      
	    // mid range frequency detected
	    else if ( (_hitime < HI_THRESH) && (_hitime > LO_THRESH) )
		{
		    // set _motor state to stop
		    _motorState = STOP;	
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////
/// name: WritePWM
/// params: none
/// return: void
/// desc: tells the dc motor what to do based on the state of the motor (_motorState).
///       if the motor state is UP and the tachometer is greater than the predefined
///       bound, the motor is written to at a predefined speed and the tachometer is
///       decremented.
///       if the motor state is DOWN and the tachometer is less than the predefined
///       threshold, the motor is written to and the tachometer is incremented.
///       if the motor state is STOP, the motor is halted.
void WritePWM(void)
{
    // if the state of the motor is not STOP, write to the motor, as the
    // directional bit was set in InterpretPWM
    if ( _motorState != STOP )
	{
	    SetDCOC4PWM(MOTORSPEED);
      
	    // if the motor state is DOWN and the tachometer is within
	    // its bounds, decrement the tach counter
	    if ( (_motorState == DOWN) && (_faketach < TACH_MAX) )
		_faketach++;
      
	    // if the motor state is UP and the tachometer is within
	    // its bounds, increment the tach counter
	    else if ( (_motorState == UP) && (_faketach > TACH_MIN) )
		_faketach--;
      
	    // if the tachometer has exceeded its bounds, halt the motor
	    // and set its state to STOP
	    else
		{
		    haltArm();
		    _motorState = _motorGo = STOP;
		}
	}
  
    // if the state of the motor is STOP, halt the motor
    else
	{
	    haltArm();
	}
}

//////////////////////////////////////////////////////////////////////////////////
/// name: haltArm
/// params: none
/// return: void
/// desc: write value of STOP to the motor, which is 0, making it stop.
void haltArm(void)
{
    SetDCOC4PWM(STOP);
}



