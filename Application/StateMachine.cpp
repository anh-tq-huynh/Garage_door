//
// Created by Anh Huynh on 23.2.2026.
//

#include "StateMachine.h"

#include <iostream>
using namespace std;

StateMachine::StateMachine():
	door(2,3,6,13,16,17,27,28),
	btns (9,7,8), //sw2,0,1
	leds(20,22)
{};

void StateMachine::blink_wait() const
{
	absolute_time_t timeout = make_timeout_time_ms(500);
	bool led_state = false;
	bool last_sw1_status = false;
	while (!btns.both_btn_pressed())
	{
		bool current_sw1_status = btns.sw1_pressed();
		if (current_sw1_status && !last_sw1_status)
		{
			cout << "Door is not calibrated, please calibrate first" << endl;
		}
		last_sw1_status = current_sw1_status;
		if (time_reached(timeout))
		{
			led_state = !led_state;
			if (led_state)
			{
				leds.leds_on();
			}else
			{
				leds.leds_off();
			}
			timeout = make_timeout_time_ms(500);
		}
		sleep_ms(10);
	}
	leds.leds_off();
}

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
		leds.leds_off();
		door.start_calibration();
		leds.set_blink_not_finished();
		while (btns.both_btn_pressed());
		print_states();
		return;
	}

	if (btns.sw1_pressed())
	{
		if (!door.is_calibrated())
		{
			cout << "Door is not calibrated, please calibrate first" << endl;
		}
		while (btns.sw1_pressed()); //debounce button
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
	if (door.is_error_state())
	{
		if (!leds.blink_finished()) {
			blink_wait(); // This will loop until buttons are pressed
			leds.set_blink_finished();
		}
	}

};