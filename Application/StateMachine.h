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


class StateMachine
{
	public:
		StateMachine(MQTTService& mqtt);
		void print_states() const;
		//void garage_door_operation();
		void roll_door();
		void run();
		void blink_wait();

		void handle_mqtt_command(const char* payload);
		void send_status() const;
		void cmd_response(bool success, bool door_stuck, bool need_calibrate) const;
	private:
		GarageDoor door;
		SwitchPair btns;
		Leds leds;
		mutable OLEDDisplay oled;
		MQTTService& mqtt;
		DoorCommand cmd;
};


#endif //GARAGE_DOOR_STATEMACHINE_H