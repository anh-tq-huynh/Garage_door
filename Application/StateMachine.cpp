//
// Created by Anh Huynh on 23.2.2026.
//

#include "StateMachine.h"

#include <iostream>
using namespace std;

StateMachine::StateMachine():
	door(2,3,6,13,16,17,27,28,20,22),
	btns (9,7)
{};

void StateMachine::print_states() const
{
	cout << "DoorState: "<<door.get_door_state_string()<<endl;
	cout << "ErrorState: "<<door.get_error_state_string()<<endl;
	cout << "CalibrateState: "<<door.get_calibration_state_string()<<endl;
	cout << "===============================\n";
	cout << "\n";
}

void StateMachine::run()
{
	if (btns.both_btn_pressed())
	{
		door.start_calibration();
		print_states();
	}
	if (btns.sw0_pressed())
	{
		if (!door.is_calibrated())
		{
			cout << "Door is not calibrated, please calibrate first" << endl;
		}
		while (btns.sw0_pressed()); //debounce button
		door.operate();
		sleep_ms(50);
	}
}

void StateMachine::roll_door()
{
	bool roll_end = door.update();
	if (roll_end)
	{
		print_states();
	}
}



