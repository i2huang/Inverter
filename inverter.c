#define F_CPU    16000000UL		// 16MHz clock

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Current connection:
// PD 4 - High side switch
// PD 7 - High side switch
// PWM0A/B - Low side switches

//typedef unsigned char uint8_t;
//typedef char int8_t;
 
// Setup Timer 0 for PWM mode, two channels
void setup_tmr0_pwm(void)
{
	// Enable the port, set for PWM/dual slope mode	
	TCCR0A = (1 << COM0A1) | (1 << COM0A0) | 
		     (1 << COM0B1) | (0 << COM0B0) |
		     (1 << WGM00)  | (0 << WGM01) ;

	TCCR0B = (0 << WGM02) |
		     (1 << CS00);		// clkIO = No prescaling
}

void inline set_tmr0_pwm(uint8_t ch0, uint8_t ch1)
{
	OCR0A = ch0;
	OCR0B = ch1;
}

// Setup Timer 0 for PWM mode, two channels
void setup_tmr2_pwm(void)
{
	// Enable the port, set for PWM/dual slope mode	
	TCCR2A = (1 << COM2A1) | (1 << COM2A0) | 
		     (1 << COM2B1) | (0 << COM2B0) |
		     (1 << WGM20)  | (0 << WGM21) ;

	TCCR2B = (0 << WGM22) |
		     (1 << CS20);		// clkIO = No prescaling
}

void inline set_tmr2_pwm(uint8_t ch0, uint8_t ch1)
{
	OCR2A = ch0;
	OCR2B = ch1;
}

// Basic sine table, angle from 0 to 127 for 360 degrees
// Output is unsigned, midpoint set at 127
const uint8_t sine_table[] PROGMEM = {0, 
                               6, 12, 19, 25, 31, 37, 43, 49,
                              54, 60, 65, 71, 76, 81, 85, 90,
                              94, 98, 102, 106, 109, 112, 115, 117,
                              120, 122, 123, 125, 126, 126, 127, 127};
#define SINE_LOOKUP(x) (pgm_read_byte(&(sine_table[x])))

uint8_t sine(uint8_t angle)
{
	if (angle <= 32) return (SINE_LOOKUP(angle) + 127);	    // 0 to 90 degrees
	if (angle <= 64) return (SINE_LOOKUP(64 - angle) + 127);	// 90 to 180 degrees
	if (angle <= 96) return (127 - SINE_LOOKUP(angle - 64));	    // 180 to 270 degrees
	if (angle <= 127) return (127 - SINE_LOOKUP(64 - (angle - 64)));	    // 180 to 270 degrees
	// Out of range error
	return 0;
}

// Use a ISR that fires at each count, rate should be 31.25kHz
static uint8_t pwm_angle;
static uint8_t pwm_scaler;
ISR (TIMER0_OVF_vect)
{
	uint8_t base_sine;
	uint8_t pos_ph;
	uint8_t neg_ph;
	uint8_t pos_ph_lsd;		// low side drive
	uint8_t neg_ph_lsd;
	PORTB |= 0x02;
	pwm_scaler++;
	if (pwm_scaler >= 4) {
		pwm_scaler = 0;
		pwm_angle++;
		if (pwm_angle > 127) pwm_angle = 0;
		base_sine = sine(pwm_angle);
		// Split the sine in bi-phase drive
		pos_ph = base_sine > 127 ? base_sine - 127 : 0;
		neg_ph = base_sine < 127 ? 127 - base_sine : 0;
		pos_ph *= 2;
		neg_ph *= 2;
		//pos_ph_lsd = pos_ph >= 10 ? pos_ph - 10 : 0;
		pos_ph_lsd = pos_ph;
		//neg_ph_lsd = neg_ph >= 10 ? neg_ph - 10 : 0;
		neg_ph_lsd = neg_ph;
		set_tmr0_pwm(pos_ph_lsd, pos_ph);
		set_tmr2_pwm(neg_ph_lsd, neg_ph);
	}
	PORTB &= ~0x02;
}
 
void mod_sine_wave(uint8_t pwm_angle)
{
	// Generate simple modified sine wave on PORTB
	if (pwm_angle < 16) {
		PORTB = (PORTB & ~0x3c) | ~0x0c;
	}
	else 
	if (pwm_angle < 48) {
		PORTB = (PORTB & ~0x3c) | ~0x18;
	}
	else
	if (pwm_angle < 80) {
		PORTB = (PORTB & ~0x3c) | ~0x0c;
	}
	else 
	if (pwm_angle < 112) {
		PORTB = (PORTB & ~0x3c) | ~0x24;
	}
	else {
		PORTB = (PORTB & ~0x3c) | ~0x0c;
	}
}

int main (void)
{
	uint8_t counter;

	pwm_angle = 0;
	pwm_scaler = 0;

  	// Setup the TMR0 for PWM 
	setup_tmr0_pwm();
	setup_tmr2_pwm();
	// Enable overflow interrupt
	TIMSK0 = _BV(TOIE0);
	// PWM port need to be set as output
	DDRD = 0xFC;			
  
	// set PORTB for output
	DDRB = 0xFF;

	// Enable global interrupt
	sei();
	
	while (1)
	{
		/* set PORTB.2 high */
		PORTB |= 0x01;
		
		/* wait (10 * 120000) cycles = wait 1200000 cycles */
		counter = 0;
		while (counter != 50)
		{
			/* wait (30000 x 4) cycles = wait 120000 cycles */
			_delay_loop_2(10000);
			counter++;
		}
	
		/* set PORTB.2 low */
		PORTB &= ~0x01;
		
		/* wait (10 * 120000) cycles = wait 1200000 cycles */
		counter = 0;
		while (counter != 50)
		{
			/* wait (30000 x 4) cycles = wait 120000 cycles */
			_delay_loop_2(10000);
			counter++;
		}
	}
	
	return 1;
}

