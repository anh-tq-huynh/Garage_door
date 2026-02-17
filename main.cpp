//
// Created by Anh Huynh on 16.2.2026.
//
#include <iostream>

#include "Hardware/GPIOPin.h"
#include "hardware/gpio.h"
#include "pico/stdio.h"
#include "pico/time.h"

int main()
{
		const uint led_pin = 22;
		uint count = 0;

		// Initialize LED pin
		gpio_init(led_pin);
		gpio_set_dir(led_pin, GPIO_OUT);

		// Initialize chosen serial port
		stdio_init_all();

		// Loop forever
		while (true) {

			// Blink LED
			std::cout << "Blinking! %u\r\n" <<  ++count << std::endl;
			gpio_put(led_pin, true);
			sleep_ms(1000);
			gpio_put(led_pin, false);
			sleep_ms(1000);
		}
}
