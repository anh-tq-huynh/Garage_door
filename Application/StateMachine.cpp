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
#define PAGE_SIZE 64
using namespace std;



StateMachine::StateMachine(MQTTService& mqtt):
	door(2,3,6,13,4,5,27,28),
	memory(),
	mqtt(mqtt),

	btns (9,7,8), //sw2,0,1
	leds(20,22)

{};


std::string StateMachine::get_door_state_string() const {
	if (!door.is_calibrated()) return "Unknown";

	int current = door.get_current_step();
	int total = door.get_total_step();

	if (current <= 0) return "Open";
	if (current >= total) return "Closed";

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

void StateMachine::handle_mqtt_command(const char *payload)
{
	string command(payload);
	cmd_str = command;
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
				cmd_str = "Invalid command";
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

		}
		else //last_state == "Unknown"
		{
			this -> state = MachineState::UNCALIBRATED; //need to recalibrate to know which direction to run
		}
	}
}
bool StateMachine::read_eeprom()
{
	uint8_t array[PAGE_SIZE];
	MachineState latest_state = memory.read_all_and_parse(array);
	if (latest_state == MachineState::IDLE)
	{
		eeprom_read_done = false;
		cout << "No valid step data. Require re-calibration to proceed." <<endl;
		state = MachineState::UNCALIBRATED;
		return false; //No data from EEPROM
	}

	int total_steps= 0;
	int current_steps = 0;
	uint8_t step_data_array[PAGE_SIZE];
	if (memory.get_steps(step_data_array,total_steps,current_steps))
	{
		door.set_total_steps(total_steps);
		door.set_current_steps(current_steps);
		door.set_calibration(true);
		eeprom_read_done = true;
		state = latest_state;
		cout << "EEPROM read successfully. Total steps: " << total_steps << ". Current steps: " << current_steps <<endl;
	}
	else
	{
		door.set_calibration(false);
		cout << "No valid step data. Require re-calibration to proceed." <<endl;
		return false;
	}
	return true;
}


void StateMachine::report_status()
{
	string current_state = get_door_state_string();
	string error = get_error_state_string();
	string calib = get_calibration_state_string();

	//Show on OLED
	oled.show_status(current_state, error, calib);

	send_status(); //to MQTT
	save_status(current_state,error,calib); //to EEPROM
	string steps_data = door.get_steps_data();
	cout << "Writing step data: " << steps_data << endl;
	memory.write_addr_zero(steps_data.c_str()); //update latest position

	//Print on serial port
	print_states(current_state, error, calib);

}


void StateMachine::sw1_toggle_state()
{
	if (!door.is_calibrated())
	{
		state = MachineState::UNCALIBRATED;
	}

	switch (state)
	{
		//Cases when it was moving
		case MachineState::OPENING:
		case MachineState::CLOSING:
		case MachineState::MOVE_TO_TARGET:
			state =  (door.get_last_dir() == -1) ? MachineState::STOPPED_OPENING : MachineState::STOPPED_CLOSING;
			break;

		case MachineState::OPEN:
		case MachineState::STOPPED_OPENING:
			state = MachineState::CLOSE;
			break;

		case MachineState::CLOSE:
		case MachineState::STOPPED_CLOSING:
			state = MachineState::OPEN;
			break;

		default:
			break;
	}
}



void StateMachine::set_state_on_cmd()
{
	switch (cmd)
	{
		case (DoorCommand::CALIBRATE):
			state = MachineState::CALIBRATE;
			break;
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
	if (btns.both_btn_pressed()) //2 switches are pressed
	{
		while (btns.both_btn_pressed());
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
		initiate_cmd_var();
		return true;
	}
	return false;
}

void StateMachine::initiate_cmd_var()
{
	mqtt.publish("Executing command: " + cmd_str, "garage/door/response");
	new_cmd_available = false;
	executing_cmd = true;
}

void StateMachine::dispatch_action_on_state()
{

	//When initiating a movement, we need to write to EEPROM "MOVING", so that when powered off during movement, calibration is required
	if (state == MachineState::OPEN || state == MachineState::CLOSE || state == MachineState::MOVE_TO_TARGET)
	{
		movement_done = false;
		memory.write_new_entry("MOVING");
	}

	switch (state)
	{
		case MachineState::UNCALIBRATED:
			cout << "Door is not calibrated. Press sw0 and sw2 to calibrate" << endl;
			oled.show_sw2_sw0();
			break;

		case MachineState::CALIBRATE:
			door.set_error(false);
			door.start_calibration();
			report_calibration_result();
			break;

			//Initiating movement
		case MachineState::CLOSE:
			door.close();
			state = MachineState::CLOSING;
			break;
		case MachineState::OPEN:
			door.open();
			state = MachineState::OPENING;
			break;
		case MachineState::MOVE_TO_TARGET:
			door.move_to_target();
			state = (door.get_last_dir() == -1) ? MachineState::OPENING : MachineState::CLOSING;
			break;

			//Stop while moving
		case MachineState::STOPPED_CLOSING:
		case MachineState::STOPPED_OPENING:
			door.stop();
			//report_status();
			break;
		case MachineState::ERROR:
			cout << "Error - Door is stuck";
			door.set_error(true);
			break;

		default:
			break;
	}
}
void StateMachine::error_handling()
{
	if (door.is_error_state())
	{
		if (time_reached(next_blink)) {
			leds.leds_are_on() ? leds.leds_off() : leds.leds_on();
			next_blink = make_timeout_time_ms(500);
		}
	}else
	{
		leds.leds_off();
	}
}

void StateMachine::display_calibration()
{
	if (!door.is_calibrated())
	{
		oled.show_sw2_sw0();
		state = MachineState::UNCALIBRATED;
	}
}

void StateMachine::execute_last_state_from_eeprom()
{
	eeprom_read_done = false; //set to false so it won't trigger again after the first time
	movement_done = true;
	report_status();
}

bool StateMachine::run_movement()
{
	if (!door.is_calibrated() || movement_done)
	{
		return false;
	}

	return door.execute();
}

void StateMachine::report_if_finished(bool is_finished)
{
	if (is_finished)
	{
		movement_done = true;
		if (state == MachineState::OPENING) state = MachineState::OPEN;
		else if (state == MachineState::CLOSING) state = MachineState::CLOSE;

		if (executing_cmd)
		{
			if (door.is_error_state() && state != MachineState::ERROR) //If the door is stuck while moving, report error once
			{
				state = MachineState::ERROR;
				mqtt.cmd_response(false, true, true);
			} else if (!door.is_error_state())
			{
				mqtt.cmd_response(true, false, false);
			}
			executing_cmd = false;
		}
		report_status(); //Once the movement is finished, EEPROM will save the status in the indicated format
	}

}

void StateMachine::uncalibrated_debug_print()
{
	if (executing_cmd)
	{
		mqtt.cmd_response(false,false,true);
		executing_cmd = false;
	}
}

void StateMachine::report_calibration_result()
{
	if (door.is_error_state())
	{
		state = MachineState::ERROR;
		if (executing_cmd)
		{
			mqtt.cmd_response(false, true, true);
			executing_cmd = false;
		}
	}
	else
	{
		state = MachineState::CLOSE;
		if (executing_cmd)
		{
			mqtt.cmd_response(true, false, false);
			executing_cmd = false;
		}
	}
	report_status();
}

void StateMachine::run()
{
	// 1. One-time EEPROM setup
	if (eeprom_read_done)
	{
		execute_last_state_from_eeprom();
	}

	// 2. Poll Inputs
	bool input_detected = update_state();

	// 3. Forced State Protection
	if (!door.is_calibrated() && state != MachineState::CALIBRATE && state != MachineState::UNCALIBRATED && state != MachineState::ERROR)
	{
		state = MachineState::UNCALIBRATED;
		uncalibrated_debug_print();
	}

	// 4. Dispatch Actions
	if (input_detected)
	{
		dispatch_action_on_state();
	}

	// 5. Execution
	bool movement_finished = run_movement();
	report_if_finished(movement_finished);

	// 6. Background
	error_handling();
	if (state == MachineState::UNCALIBRATED) oled.show_sw2_sw0();
}


