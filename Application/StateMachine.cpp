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
			//cmd_response(false, true, true);
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
std::string StateMachine::get_door_state_string() const {
	if (state == MachineState::OPEN) return "Open";
	if (state == MachineState::CLOSE) return "Closed";
	if (!door.is_calibrated()) return "Unknown";
	return "In between";
}

std::string StateMachine::get_error_state_string() const {
	if (state == MachineState::ERROR) return "Door stuck";
	return "Normal";
}

std::string StateMachine::get_calibration_state_string() const {
	return door.is_calibrated() ? "Calibrated" : "Not calibrated";
}

void StateMachine::print_states(const string &current_state,const string &error,const string &calib)
{
	cout << "DoorState: "<<current_state<<endl;
	cout << "ErrorState: "<<error<<endl;
	cout << "CalibrateState: "<<calib<<endl;
	cout << "===============================\n";
	cout << "\n";
}
/*
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
};*/


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
				cmd = DoorCommand::MOVE_TO_TARGET;
			}
			else
			{
				cout << "Invalid input." << endl;
				new_cmd_available = false;
			}
		}
	}
	new_cmd_available = true;
}

void StateMachine::send_status() const
{
	if (!mqtt.mqtt_is_connected())
	{
		return;
	}
	mqtt.send_status(get_door_state_string(), get_error_state_string(), get_calibration_state_string());
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
	if (calib == "Not calibrated")
	{
		this ->state = MachineState::UNCALIBRATED;
	}
	else if (error == "Door stuck")
	{
		this -> state = MachineState::ERROR;
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
			this -> state = MachineState::CLOSE;

		}else if (last_state == "In between - closed")
		{
			this -> state = MachineState::STOPPED_CLOSING;
		}
		else //last_state == "Unknown"
		{
			this -> state = MachineState::ERROR; //need to recalibrate to know which direction to run
		}
	}
}

void StateMachine::report_status()
{
	string current_state = get_door_state_string();
	string error = get_error_state_string();
	string calib = get_calibration_state_string();

	print_states(current_state, error, calib);
	send_status(); //to MQTT
	oled.show_status(current_state, error, calib);
	save_status(current_state,error,calib); //to EEPROM
}


void StateMachine::sw1_toggle_state()
{
	switch (state)
	{
		//When the next direction is towards Opening
		case MachineState::CLOSING:
			state = MachineState::STOPPED_CLOSING;
			break;
		case MachineState::STOPPED_CLOSING:
			state = MachineState::OPEN;
			break;

		//When the next direction is towards Closing
		case MachineState::OPENING:
			state = MachineState::STOPPED_OPENING;
			break;
		case MachineState::STOPPED_OPENING:
			state = MachineState::CLOSE;
			break;

			//When the door is at its limit
		case MachineState::OPEN:
			state = MachineState::CLOSE;
			break;
		case MachineState::CLOSE:
			state = MachineState::OPEN;
			break;
		case MachineState::UNCALIBRATED:
			state = MachineState::UNCALIBRATED;
			break;

		case MachineState::CALIBRATE:
			state = MachineState::OPEN;
			break;

		case MachineState::MOVE_TO_TARGET:
			if (door.get_last_dir() == -1) // Was opening
			{
				state = MachineState::STOPPED_OPENING;
			}else
			{
				state = MachineState::STOPPED_CLOSING;
			}
			break;
		case MachineState::ERROR:
		case MachineState::IDLE:
			break;
	}
	if (!door.is_calibrated())
	{
		state = MachineState::UNCALIBRATED;
	}
}
void StateMachine::set_state_on_cmd()
{
	switch (cmd)
	{
		case (DoorCommand::CLOSE):
			state = MachineState::CLOSE;
			break;
		case(DoorCommand::OPEN):
			state = MachineState::OPEN; //Door is now closed -> need to open
			break;
		case (DoorCommand::STOP):
			if (door.get_last_dir() == -1)
			{
				state = MachineState::STOPPED_OPENING;
			}
			else
			{
				state = MachineState::STOPPED_CLOSING;
			}
			break;
		case(DoorCommand::MOVE_TO_TARGET):
			state = MachineState::MOVE_TO_TARGET;
		default:
			break;
	}
}


bool StateMachine::update_state()
{
	if (btns.debounce_sw(2)) //2 switches are pressed
	{
		state = MachineState::CALIBRATE;
		return true;
	}
	else if (btns.sw1_pressed()) //sw1 is pressed
	{
		while (btns.sw1_pressed()){};
		sw1_toggle_state();
		return true;
	} else if (new_cmd_available)
	{
		set_state_on_cmd();
		cmd = DoorCommand::IDLE;
		new_cmd_available = false;
		return true;
	}
	return false;
}



void StateMachine::run(const bool &eeprom_read)
{
	bool new_state = update_state();
	if (! door.is_calibrated())
	{
		oled.show_sw2_sw0();
	}
	if (new_state || eeprom_read)
	{
		switch (state)
	{
		case MachineState::UNCALIBRATED:
			cout << "Door is not calibrated. Press sw0 and sw2 to calibrate" << endl;
			oled.show_sw2_sw0();
			break;

		case MachineState::CALIBRATE:
				door.start_calibration();
				state = MachineState::CLOSE;
				report_status();
			break;
		case MachineState::CLOSE:
			if (!door.is_calibrated())
			{
				state = MachineState::UNCALIBRATED;
			}
			else if (door.is_error_state())
			{
				state = MachineState::ERROR;
			}
			else
			{
				door.close();
				movement_done = false;
				state = MachineState::CLOSING;
			}
			break;
		case MachineState::OPEN:
			if (!door.is_calibrated())
			{
				state = MachineState::UNCALIBRATED;
			}
			else if (door.is_error_state())
			{
				state = MachineState::ERROR;
			}
			else
			{
				door.open();
				movement_done = false;
				state = MachineState::OPENING;
			}
			break;
		case MachineState::STOPPED_CLOSING:
		case MachineState::STOPPED_OPENING:
			if (!door.is_calibrated())
			{
				state = MachineState::UNCALIBRATED;
			}
			else if (door.is_error_state())
			{
				state = MachineState::ERROR;
			}
			else
			{
				door.stop();
				movement_done = false;
			}
			break;
		case MachineState::ERROR:
			cout << "Error - Door is stuck";
			report_status();
			break;
		case MachineState::MOVE_TO_TARGET:
			if (!door.is_calibrated())
			{
				state = MachineState::UNCALIBRATED;
			}
			else if (door.is_error_state())
			{
				state = MachineState::ERROR;
			}
			else
			{
				door.move_to_target();
				movement_done = false;
			}
			break;
		case MachineState::IDLE:
			break;
	}
	}
	if (door.is_error_state())
	{
		if (time_reached(next_blink)) {
			leds.leds_are_on() ? leds.leds_off() : leds.leds_on();
			next_blink = make_timeout_time_ms(500);
		}
	}
	if (door.is_calibrated())
	{
		if (!movement_done)
		{
			bool roll_door = door.execute();
			if (roll_door)
			{
				movement_done = true;
				if (state == MachineState::OPENING) state = MachineState::OPEN;
				if (state == MachineState::CLOSING) state = MachineState::CLOSE;
				report_status();
			}
		}
	}
}



