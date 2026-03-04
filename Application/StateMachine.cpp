//
// Created by Anh Huynh on 23.2.2026.
//

#include "StateMachine.h"
#include "MQTTService.h"
#define CLOSE_CMD "CLOSE"
#define OPEN_CMD "OPEN"
#define STOP_CMD "STOP"
#define CALIBRATE_CMD "CALIBRATE"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdlib.h>
using namespace std;

StateMachine::StateMachine(MQTTService& mqtt):
	door(2,3,6,13,4,5,27,28),
	btns (9,7,8), //sw2,0,1
	leds(20,22),
	mqtt(mqtt)
{};

void StateMachine::blink_wait()
{
	absolute_time_t timeout = make_timeout_time_ms(500);
	bool led_state = false;
	bool last_sw1_status = false;
	while (!btns.both_btn_pressed() && cmd != DoorCommand::CALIBRATE)
	{
		mqtt.client_yield();
		bool current_sw1_status = btns.sw1_pressed();
		if ((current_sw1_status && !last_sw1_status))
		{
			cout << "Door is not calibrated, please calibrate first" << endl;
		}
		if (cmd == DoorCommand::CLOSE || cmd == DoorCommand::OPEN || cmd == DoorCommand::STOP)
		{
			cout << "Door is not calibrated, please calibrate first" << endl;
			cmd_response(false, true, true);
			cmd = DoorCommand::IDLE;
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
	oled.show_status(door.get_door_state_string(), door.get_error_state_string(), door.get_calibration_state_string());
}

void StateMachine::run()
{
	if (btns.both_btn_pressed() || cmd == DoorCommand::CALIBRATE)
	{
		leds.leds_off();
		door.start_calibration();
		leds.set_blink_not_finished();
		while (btns.both_btn_pressed());

		print_states();
		cmd = DoorCommand::IDLE;
		send_status();
		cmd_response(true, false, false);

		return;
	}

	if (btns.sw1_pressed() || cmd != DoorCommand::IDLE)
	{
		if (btns.sw1_pressed())
		{
			while (btns.sw1_pressed()); //debounce button
		}
		if (!door.is_calibrated())
		{
			door.reset_state();
			send_status();
			cout << "Door is not calibrated, please calibrate first" << endl;
			cmd_response(false, false,true);
		}
		else
		{
			if (cmd == DoorCommand::STOP)
			{
				cmd_response(true, false, false);
				send_status();
				mqtt.publish("Door stopped!", "garage/door/response");
			}
			if (cmd == DoorCommand::CLOSE || cmd == DoorCommand::OPEN)
			{
				mqtt.publish("Executing command. Door is moving!", "garage/door/response");
			}
		}

		if (cmd != DoorCommand::IDLE) //Change door command to prevent endless loop
		{
			door.set_state(cmd);
			cmd = DoorCommand::IDLE;
		}

		door.operate();

		//print_states();
		//send_status();
		sleep_ms(50);
	}
}

void StateMachine::roll_door()
{
	bool roll_end = door.update();
	if (roll_end)
	{
		print_states();
		send_status();
		cmd_response(true, false, false);
		/*
		mqtt.publish(door.get_door_state_string() + " | " +
		         door.get_error_state_string() + " | " +
		         door.get_calibration_state_string(),
		         "garage/door/status");*/
	}

	if (door.is_error_state())
	{
		if (!leds.blink_finished()) {
			cmd_response(false, true, true);
			cmd = DoorCommand::IDLE;
			door.reset_state();
			blink_wait(); // This will loop until buttons are pressed or new command is sent
			leds.set_blink_finished();
		}
	}
};
/*
void StateMachine::handle_mqtt_command(const char* payload)
{
	string cmd(payload);
	cout << "[MQTT] Command received: '" << cmd << "'" << endl;

	if (!door.is_calibrated()) {
		cout << "[MQTT] Door not calibrated, ignoring command." << endl;
		mqtt.publish("Not calibrated", "garage/door/status");
		return;
	}

	if (cmd == "OPEN") {
		door.open();
	} else if (cmd == "CLOSE") {
		door.close();
	} else if (cmd == "STOP") {
		door.stop();
	} else if (cmd == "CALIBRATE") {
		door.start_calibration();
	} else {
		cout << "[MQTT] Unknown command: " << cmd << endl;
		return;
	}

	print_states();
	mqtt.publish(door.get_door_state_string() + " | " +
	             door.get_error_state_string() + " | " +
	             door.get_calibration_state_string(),
	             "garage/door/status");
}*/

void StateMachine::handle_mqtt_command(const char *payload)
{
	string command(payload);
	transform(command.begin(), command.end(), command.begin(), ::toupper);

	if (command == CLOSE_CMD)
	{
		cmd = DoorCommand::CLOSE;
	}
	else if (command == OPEN_CMD)
	{
		cmd = DoorCommand::OPEN;
	}
	else if (command == STOP_CMD)
	{
		cmd = DoorCommand::STOP;
	}
	else if (command == CALIBRATE_CMD)
	{
		cmd = DoorCommand::CALIBRATE;
	}
}

void StateMachine::send_status() const
{
	mqtt.publish(door.get_door_state_string() + " | " +
				 door.get_error_state_string() + " | " +
				 door.get_calibration_state_string(),
				 "garage/door/status");
}

void StateMachine::cmd_response(const bool success, bool door_stuck, bool need_calibrated) const
{
	if (success)
	{
		mqtt.publish("Command executed successfully!","garage/door/response");
	}
	else
	{
		mqtt.publish("Error - Failed to execute command.", "garage/door/response");
	}
	if (door_stuck)
	{
		mqtt.publish ("Door is stuck.", "garage/door/response");
	}
	if (need_calibrated)
	{
		mqtt.publish("Need calibrating before running!","garage/door/response");
	}
}


