//
// Created by Anh Huynh on 23.2.2026.
//

#ifndef GARAGE_DOOR_STATEMACHINE_H
#define GARAGE_DOOR_STATEMACHINE_H
#include "GarageDoor.h"
#include "../Hardware/Leds.h"
#include "../Hardware/SwitchPair.h"
#include "../Hardware/OLEDDisplay.h"
#include "MQTTService.h"
#include "LocalMemory.h"

enum class MachineState
{
	IDLE,
	UNCALIBRATED,
	OPEN,
	CLOSED,
	STOPPED_OPENING, //Got stopped during opening
	STOPPED_CLOSING,  //Got stopped during closing
	MOVE_TO_TARGET
};

class StateMachine
{
	public:
		StateMachine(MQTTService& mqtt);

		//Machine state
		void print_states();

		//Machine operation
		bool roll_door();
		void run();
		void blink_wait();

		//MQTT
		void handle_mqtt_command(const char* payload);
		void send_status() const;
		void cmd_response(bool success, bool door_stuck, bool need_calibrate) const;

		//Local Memory
		void save_status(const string &state, const string &error, const string &calib);
		void read_and_parse(uint8_t *array);
		void get_latest_state(const string &state, const string &error, const string &calib);
	private:
		GarageDoor door;
		SwitchPair btns;
		Leds leds;
		mutable OLEDDisplay oled;
		MQTTService& mqtt;
		DoorCommand cmd = DoorCommand::IDLE;
		LocalMemory memory;
		MachineState state = MachineState::IDLE;
};


#endif //GARAGE_DOOR_STATEMACHINE_H