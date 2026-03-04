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
		void run(int &mqtt_msg_count);
		void blink_wait() const;

		void handle_mqtt_command(const char* payload);
		void send_status() const;
	private:
		GarageDoor door;
		SwitchPair btns;
		Leds leds;
		mutable OLEDDisplay oled;
		MQTTService& mqtt;
		DoorCommand cmd;
};


#endif //GARAGE_DOOR_STATEMACHINE_H