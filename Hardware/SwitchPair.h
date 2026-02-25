//
// Created by Anh Huynh on 23.2.2026.
//

#ifndef GARAGE_DOOR_SWITCHPAIR_H
#define GARAGE_DOOR_SWITCHPAIR_H
#include "GPIOPin.h"


class SwitchPair
{
	public:
		SwitchPair(int sw0, int sw2, int sw1) :
			sw0(sw0, true, true, true),
			sw2(sw2, true, true, true),
			sw1(sw1, true, true, true){};
		bool both_btn_pressed () const;
		bool sw0_pressed() const;
		bool sw2_pressed() const;
		bool sw1_pressed() const;
	private:
		GPIOPin sw0;
		GPIOPin sw2;
		GPIOPin sw1;
};


#endif //GARAGE_DOOR_SWITCHPAIR_H