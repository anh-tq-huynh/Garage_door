//
// Created by Anh Huynh on 23.2.2026.
//

#ifndef GARAGE_DOOR_LED_H
#define GARAGE_DOOR_LED_H
#include "GPIOPin.h"


class Leds
{
	public:
		Leds(int led1, int led2) : led1(led1, false, false, false), led2(led2, false, false, false){};
		bool blink_finished() const;
		void set_blink_finished();
		void set_blink_not_finished();
		void leds_on();
		void leds_off();
		bool leds_are_on() const;

	private:
		GPIOPin led1;
		GPIOPin led2;
		bool blink_done = true;
		bool leds_state;
};


#endif //GARAGE_DOOR_LED_H