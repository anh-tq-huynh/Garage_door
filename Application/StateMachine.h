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
		StateMachine(MQTTService& mqtt);

		void waiting_screen() const;

		//Machine state
		static void print_states(const string &current_state,const string &error,const string &calib);
		std::string get_door_state_string() const;
		std::string get_error_state_string() const;
		std::string get_calibration_state_string() const;

		//Machine operation
		bool roll_door();
		void run();
		bool update_state();

		void initiate_cmd_var();

		void dispatch_action_on_state();

		void error_handling();

		void display_calibration();

		void execute_last_state_from_eeprom();

		bool run_movement();

		void report_if_finished(bool is_finished);

		void blink_wait();
		void sw1_toggle_state();

		void transition_to(MachineState next_state);

		void execute_new_state();

		void execute_finised();

		void report_status(); //Report status locally and via MQTT

		//MQTT
		void handle_mqtt_command(const char* payload);
		void send_status() const;
		void set_state_on_cmd();

		//Local Memory
		void save_status(const string &state, const string &error, const string &calib);
		void read_and_parse(uint8_t *array);
		void get_latest_state(const string &state, const string &error, const string &calib);
		bool read_eeprom();
		bool get_total_steps();
	private:
		GarageDoor door;
		Switches btns;
		Leds leds;
		mutable OLEDDisplay oled;
		MQTTService& mqtt;
		DoorCommand cmd = DoorCommand::IDLE;
		LocalMemory memory;
		MachineState state = MachineState::UNCALIBRATED;
		bool new_cmd_available = false;
		bool executing_cmd = false;
		string cmd_str;
		absolute_time_t next_blink = get_absolute_time();
		bool movement_done = false;
		bool eeprom_read_done = false;
};


#endif //GARAGE_DOOR_STATEMACHINE_H