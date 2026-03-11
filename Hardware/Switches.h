//
// Created by Anh Huynh on 23.2.2026.
//

#ifndef GARAGE_DOOR_SWITCHPAIR_H
#define GARAGE_DOOR_SWITCHPAIR_H
#include "GPIOPin.h"
#include "pico/types.h"


class Switches
{
	public:
		Switches(int sw0, int sw2, int sw1) :
			sw0(sw0, true, true, true),
			sw2(sw2, true, true, true),
			sw1(sw1, true, true, true){};
		bool both_btn_pressed () const;
		bool sw0_pressed() const;
		bool sw2_pressed() const;
		bool sw1_pressed() const;
		bool debounce_sw(int sw_nr);
	private:
		GPIOPin sw0;
		GPIOPin sw2;
		GPIOPin sw1;
		absolute_time_t last_press_time = 0;
};


#endif //GARAGE_DOOR_SWITCHPAIR_H