//
// Created by Anh Huynh on 23.2.2026.
//

#include "Leds.h"

#include <ctime>

#include "pico/time.h"

void Leds::leds_on() const
{
	led1.write(1);
	led2.write(1);
}

void Leds::leds_off() const
{
	led1.write(0);
	led2.write(0);
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




