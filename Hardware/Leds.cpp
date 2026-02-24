//
// Created by Anh Huynh on 23.2.2026.
//

#include "Leds.h"

#include <ctime>

#include "pico/time.h"

void Leds::blink_led()
{
	for (int i = 0; i <= 5; ++i)
	{
		led1.write(1);
		led2.write(1);
		sleep_ms(200);
		led1.write(0);
		led2.write(0);
		sleep_ms(200);
	}
}
