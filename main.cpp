//
// Created by Anh Huynh on 16.2.2026.
//
#include <fstream>
#include <iostream>
#include "pico/stdio.h"
#include "pico/time.h"
#include "Application/StateMachine.h"
#include "Application/MQTTService.h"
#include "private.h"
using namespace std;

void welcome_text()
{
	cout << "Hello! Welcome to the program" << endl;
}

int main()
{
	stdio_init_all();
	sleep_ms(3000);

	//Connect Internet and MQTT
	MQTTService mqtt(SSID, PASSWORD);
	mqtt.connect_tcp();
	mqtt.connect_mqtt();

	StateMachine sm(mqtt);
	mqtt.set_state_machine(&sm);
	mqtt.subscribe("garage/door/command");

	//Start program & check last state
	welcome_text();
	sm.read_eeprom();

	while (true) {
		mqtt.client_yield();
		sm.run();
	}
}
