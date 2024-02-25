#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include <string.h>

#include "i2c.h"

#define CMD_GET_TRIGGERED 0x00

static uint8_t triggered;

int8_t i2c_tx(uint8_t idx, uint8_t *val)
{
	switch (idx) {
		case CMD_GET_TRIGGERED:
			*val = triggered;
			triggered = 0;
			break;
		default:
			*val = idx;
			return -1;
	}
	return 0;
}

int8_t i2c_rx(uint8_t idx, uint8_t val)
{
	return 0;
}

ISR(AC0_AC_vect)
{
	PORTA.OUTSET = (1 << 6);
	triggered = 1;
	AC0_STATUS = 0x1;
}

static void init_analog_comperator(void)
{
	VREF_CTRLA = VREF_DAC0REFSEL_0V55_gc; 
	AC0_MUXCTRLA = AC_MUXNEG1_bm;
	AC0_INTCTRL = AC_CMP_bm;
	AC0_CTRLA = AC_INTMODE_POSEDGE_gc | AC_ENABLE_bm;
}

static int ac_state(void)
{
	return (AC0_STATUS & AC_STATE_bm) != 0;
}

int main(void)
{
	PORTA.DIR = (0x1 << 3) | (0x1 << 6);

	i2c_device_init(0x52);
	init_analog_comperator();

	CPU_SREG |= CPU_I_bm;

	while (1) {
		PORTA.OUT = (0x1 << 6) * triggered | (1 << 3) * ac_state();
		_delay_ms(10);
	}

	return 0;
}
