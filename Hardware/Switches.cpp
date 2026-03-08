//
// Created by Anh Huynh on 23.2.2026.
//

#include "Switches.h"

#include "pico/time.h"
#include "pico/types.h"

bool Switches::sw0_pressed() const
{
	return sw0.read();
}

bool Switches::sw2_pressed() const
{
	return sw2.read();
}

bool Switches::sw1_pressed() const
{
	return sw1.read();
}
bool Switches::both_btn_pressed() const
{
	return sw0_pressed() && sw2_pressed();
}

bool Switches::debounce_sw(int sw_nr)
{
	//sw_nr = 1, debounce sw1
	//sw_nr = 2, debounce 2 switches sw0, sw2
	bool sw_pressed = false;
	if (sw_nr == 1) sw_pressed = sw1_pressed();
	if (sw_nr == 2) sw_pressed = both_btn_pressed();

	absolute_time_t current_time = get_absolute_time();

	if (sw_pressed)
	{
		if (current_time - last_press_time > 200)
		{
			last_press_time = current_time;
			return true;
		}
	}
	return false;
}



