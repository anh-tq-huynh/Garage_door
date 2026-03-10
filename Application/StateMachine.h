//
// Created by Anh Huynh on 23.2.2026.
//

#ifndef GARAGE_DOOR_STATEMACHINE_H
#define GARAGE_DOOR_STATEMACHINE_H
#include "GarageDoor.h"
#include "../Hardware/Leds.h"
#include "../Hardware/Switches.h"
#include "../Hardware/OLEDDisplay.h"
#include "MQTTService.h"
#include "LocalMemory.h"

enum class MachineState
{
	IDLE,
	ERROR,
	UNCALIBRATED,
	CALIBRATE,
	OPEN,
	CLOSE,
	OPENING,
	CLOSING,
	STOPPED_OPENING, //Got stopped during opening
	STOPPED_CLOSING,  //Got stopped during closing
	MOVE_TO_TARGET
};

class StateMachine
{
	public:
		explicit StateMachine(MQTTService& mqtt);

		//Machine operation
		void run();
		void get_latest_state(const string &state, const string &error, const string &calib);
		void handle_mqtt_command(const char* payload);
		bool read_eeprom();

	private:
		//Drivers
		GarageDoor door;
		LocalMemory memory;
		MQTTService& mqtt;

		//Hardware
		Switches btns;
		Leds leds;
		mutable OLEDDisplay oled;

		//Operational variables
		MachineState state = MachineState::UNCALIBRATED;

		DoorCommand cmd = DoorCommand::IDLE;
		string cmd_str;
		bool new_cmd_available = false;
		bool executing_cmd = false;

		absolute_time_t next_blink = get_absolute_time();
		bool movement_done = false;
		bool eeprom_read_done = false;

		//Show states
		static void print_states(const string &current_state,const string &error,const string &calib);
		std::string get_door_state_string() const;
		std::string get_error_state_string() const;
		std::string get_calibration_state_string() const;
		void display_calibration();

		//State & action handling
		bool update_state();
		void dispatch_action_on_state();
		void error_handling();
		void execute_last_state_from_eeprom();
		void initiate_cmd_var();
		bool run_movement();
		void report_if_finished(bool is_finished);
		void uncalibrated_debug_print();
		void report_calibration_result();
		void sw1_toggle_state();


		void report_status(); //Report status locally and via MQTT

		//MQTT
		void send_status() const;
		void set_state_on_cmd();

		//Local Memory
		void save_status(const string &state, const string &error, const string &calib);
		void read_and_parse(uint8_t *array);


};


#endif //GARAGE_DOOR_STATEMACHINE_H