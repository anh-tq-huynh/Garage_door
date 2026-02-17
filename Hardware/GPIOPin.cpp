//
// Created by Anh Huynh on 16.2.2026.
//

#include "GPIOPin.h"
#include "hardware/gpio.h"

uint32_t GPIOPin::pins_in_use = 0;

GPIOPin::GPIOPin(int pin, bool input, bool pullup, bool invert)
{
	gpio_init(pin);
	uint32_t mask = (1u << pin);

	if ((pins_in_use & mask) != 0)
	{
		is_dormant = true;
		return;
	}

	pins_in_use |= mask;
	is_dormant = false;

	this -> pin = pin;

	const auto direction = input == true ? GPIO_IN : GPIO_OUT;
	gpio_set_dir(pin, direction);

	if (input)
	{
		if (pullup) gpio_pull_up(pin);
		is_input = true;
	}
	else
	{
		is_input = false;
	}

	if (invert)
	{
		if (input) gpio_set_inover(pin, GPIO_OVERRIDE_INVERT);
		else gpio_set_outover(pin, GPIO_OVERRIDE_INVERT);
	}
}

GPIOPin::~GPIOPin()
{
	uint32_t mask = 1u << pin;
	pins_in_use &= ~mask;

	gpio_deinit(pin);
}

GPIOPin::operator bool() const
{
	if (is_dormant)
	{
		return false;
	}
	return true;
}

bool GPIOPin::read() const
{
	if (is_dormant)
	{
		return false;
	}
	return gpio_get(pin);
}

void GPIOPin::write(bool value) const
{
	if (is_input)
	{
		return;
	}
	gpio_put(pin,value);
}

