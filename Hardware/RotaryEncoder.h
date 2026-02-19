//
// Created by Anh Huynh on 17.2.2026.
//

#ifndef GARAGE_DOOR_ROTARYENCODER_H
#define GARAGE_DOOR_ROTARYENCODER_H
#include "GPIOPin.h"
#include "pico/util/queue.h"
#include "hardware/gpio.h"


class RotaryEncoder
{
	public:
		RotaryEncoder(int rotA, int rotB);
		static void rotary_callback(uint gpio, uint32_t event_mask);
		// added by Yue, because the queue is private, we need a public method to let GarageDoor access the events
		bool try_get_event(int& out_value) {
			return queue_try_remove(&events, &out_value);
		}
	private:
		GPIOPin rotA;
		GPIOPin rotB;
		queue_t events;

};
extern RotaryEncoder* rotary_instance;



#endif //GARAGE_DOOR_ROTARYENCODER_H