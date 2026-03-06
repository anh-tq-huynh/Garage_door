//
// Created by Anh Huynh on 23.2.2026.
//

#include "StateMachine.h"
#include "MQTTService.h"
#define CLOSE_CMD "CLOSE"
#define OPEN_CMD "OPEN"
#define STOP_CMD "STOP"
#define CALIBRATE_CMD "CALIBRATE"
#define POS_25 "MODE1"
#define POS_50 "MODE2"
#define POS_75 "MODE3"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
using namespace std;



StateMachine::StateMachine(MQTTService& mqtt):
	door(2,3,6,13,4,5,27,28),
	btns (9,7,8), //sw2,0,1
	leds(20,22),
	mqtt(mqtt),
	memory()
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

void StateMachine::print_states()
{
	string current_state = door.get_door_state_string();
	string error = door.get_error_state_string();
	string calib = door.get_calibration_state_string();

	cout << "DoorState: "<<current_state<<endl;
	cout << "ErrorState: "<<error<<endl;
	cout << "CalibrateState: "<<calib<<endl;
	cout << "===============================\n";
	cout << "\n";

	oled.show_status(current_state, error, calib);
	save_status(current_state,error,calib);
}

void StateMachine::run()
{
	if (btns.both_btn_pressed() || cmd == DoorCommand::CALIBRATE)
	{
		leds.leds_off();
		door.start_calibration();
		leds.set_blink_not_finished();
		while (btns.both_btn_pressed()){};

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
				cout << "Executing command. Door is moving" << endl;
			}
			if (cmd == DoorCommand::MOVE_TO_TARGET)
			{
				cout << "Moving to target"<< endl;
			}

		}

		if (cmd != DoorCommand::IDLE) //Change door command to prevent endless loop
		{
			door.set_state(cmd);
			cmd = DoorCommand::IDLE;
		}

		door.operate();

		print_states();
		//send_status();
		sleep_ms(50);
	}
}

bool StateMachine::roll_door()
{
	bool roll_end = door.update();
	if (roll_end)
	{
		print_states();
		send_status();
		cmd_response(true, false, false);
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
	return roll_end;
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
	command.erase(command.find_last_not_of(" \n\r\t") + 1);
	transform(command.begin(), command.end(), command.begin(), ::toupper);

	if (command == CLOSE_CMD)
	{
		cmd = DoorCommand::CLOSE;
		door.set_target_steps(0);
	}
	else if (command == OPEN_CMD)
	{
		cmd = DoorCommand::OPEN;
		door.set_target_steps(100);
	}
	else if (command == STOP_CMD)
	{
		cmd = DoorCommand::STOP;
	}
	else if (command == CALIBRATE_CMD)
	{
		cmd = DoorCommand::CALIBRATE;
	}else if (command == POS_25)
	{
		cmd = DoorCommand::MOVE_TO_TARGET;
		door.set_target_steps(25);
	}else if (command == POS_50)
	{
		cmd = DoorCommand::MOVE_TO_TARGET;
		door.set_target_steps(50);
	}else if (command == POS_75)
	{
		cmd = DoorCommand::MOVE_TO_TARGET;
		door.set_target_steps(75);
	}
	else
	{
		if (!command.empty() && isdigit(command[0]))
		{
			char* endptr;
			long target = strtol(command.c_str(), &endptr, 10);

			if (endptr != command.c_str()) {
				door.set_target_steps(static_cast<int>(target));
			}
		}
		cmd = DoorCommand::MOVE_TO_TARGET;
	}
}

void StateMachine::send_status() const
{
	if (!mqtt.mqtt_is_connected()) return;
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

void StateMachine::save_status(const string &current_state, const string &error, const string &calib)
{
	string wr_array;
	if (current_state == "In between")
	{
		string dir = "opening";
		if (door.get_last_dir() == -1)
		{
			dir = "closing";
		}
		wr_array = current_state + " - " + dir + "|" + error + "|" + calib;
	}
	else
	{
		wr_array = current_state + "|" + error + "|" + calib;
	}
	memory.write_new_entry(wr_array.c_str());
}

void StateMachine::read_and_parse(uint8_t *array)
{
	memory.read_all_entries(array);
	ostringstream convert;
	for (int i = 0; array[i] != '\0'; i++)
	{
		convert << array[i];
	}
	convert << '\0';
	string s_array = convert.str();

	size_t state_start{0};
	string::size_type state_end = s_array.find('|', state_start);
	size_t state_size = state_end - state_start;
	string last_state = s_array.substr(state_start, state_size);

	size_t error_start{state_end + 1};
	size_t error_end = s_array.find('|',error_start);
	size_t error_size = error_end - error_start;
	string error = s_array.substr(error_start,error_size);

	size_t calib_start{error_end + 1};
	size_t calib_end = s_array.find('\0',calib_start);
	size_t calib_size = calib_end - calib_start;
	string calib = s_array.substr(calib_start,calib_size);

	get_latest_state(last_state,error, calib);

}

void StateMachine::get_latest_state(const string &last_state, const string &error, const string &calib)
{
	if (error == "Door stuck" || calib == "Not calibrated")
	{
		this ->state = MachineState::UNCALIBRATED;
	}
	else
	{
		if (last_state == "In between - closing")
		{
			this -> state = MachineState::STOPPED_CLOSING;

		} else if (last_state == "In between - open")
		{
			this -> state = MachineState::STOPPED_OPENING;

		}else if (last_state == "Open")
		{
			this -> state = MachineState::OPEN;

		}else if (last_state == "Closed")
		{
			this -> state = MachineState::CLOSED;

		}else if (last_state == "In between - closed")
		{
			this -> state = MachineState::STOPPED_CLOSING;
		}
		else //last_state == "Unknown"
		{
			this -> state = MachineState::UNCALIBRATED; //need to recalibrate to know which direction to run
		}
	}
}



