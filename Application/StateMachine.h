//
// Created by Anh Huynh on 23.2.2026.
//

#ifndef GARAGE_DOOR_STATEMACHINE_H
#define GARAGE_DOOR_STATEMACHINE_H
#include "GarageDoor.h"
#include "../Hardware/Leds.h"
#include "../Hardware/SwitchPair.h"


class StateMachine
{
	public:
		StateMachine();
		void print_states() const;
		//void garage_door_operation();
		void roll_door();
		void run();
	private:
		GarageDoor door;
		SwitchPair btns;
		Leds leds;
};


#endif //GARAGE_DOOR_STATEMACHINE_H