//
// Created by Anh Huynh on 23.2.2026.
//

#include "Leds.h"

#include <ctime>

#include "pico/time.h"

void Leds::leds_on()
{
	led1.write(1);
	led2.write(1);
	leds_state = true;
}

void Leds::leds_off()
{
	led1.write(0);
	led2.write(0);
	leds_state = false;
}

bool Leds::blink_finished() const
{
	return blink_done;
}



void Leds::set_blink_finished()
{
	blink_done = true;
}

void Leds::set_blink_not_finished()
{
	blink_done = false;
}

bool Leds::leds_are_on() const
{
	return leds_state;
}







