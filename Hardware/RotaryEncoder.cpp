//
// Created by Anh Huynh on 17.2.2026.
//

#include "RotaryEncoder.h"
#include "hardware/gpio.h"

RotaryEncoder* rotary_instance = nullptr;

RotaryEncoder::RotaryEncoder(const int rotA, const int rotB) : rotA(rotA, true, true, false), rotB(rotB, true, true, false), events{}
{
	rotary_instance = this;
	queue_init(&events, sizeof(int), 10);
	gpio_set_irq_enabled_with_callback(rotA, GPIO_IRQ_EDGE_RISE, true, &RotaryEncoder::rotary_callback);
}

void RotaryEncoder::rotary_callback(uint gpio, uint32_t event_mask)
{
	if (rotary_instance != nullptr)
	{
		if (rotary_instance -> rotA.read())
		{
			if (rotary_instance -> rotB.read()) //counter-clockwise
			{
				const int value = -1;
				queue_try_add(&rotary_instance -> events, &value);
			}
			else
			{
				const int value = 1;
				queue_try_add(&rotary_instance -> events, &value);
			}
		}
	}
}


