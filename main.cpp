//
// Created by Anh Huynh on 16.2.2026.
//
#include <fstream>
#include <iostream>

#include "Hardware/GPIOPin.h"
#include "hardware/gpio.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "Application/GarageDoor.h"
#include "Application/StateMachine.h"
#include "Application/MQTTService.h"
#include "private.h"
std::string getEnvVar (std::string const &key);
using namespace std;

// here is a test code using serial command instead of button controller,
// we can move this after build a formal logic.
#if 0
void helper_print() {
	cout<< "Commands:\n"
		"c:calibrate - Start calibration\n"
		"o:open - Open the garage door\n"
		"x:close - Close the garage door\n"
		"s:stop - Stop the garage door\n"
		"p:state - Print current state of the garage door\n"
		"===============================\n";
}

int main() {
	stdio_init_all();
	sleep_ms(3000);

	GarageDoor door(2,3,6,13,
		16,17,
		27,28);

	helper_print();
	while (true) {
		door.update();

		int c = getchar_timeout_us(0);
		if (c != PICO_ERROR_TIMEOUT) {
			switch (c) {
				case'c':
					door.start_calibration();
					cout << "DoorState: "<<door.get_door_state_string()<<endl;
					cout << "ErrorState: "<<door.get_error_state_string()<<endl;
					cout << "CalibrateState: "<<door.get_calibration_state_string()<<endl;
					break;
				case'o':
					door.open();
					cout << "DoorState: "<<door.get_door_state_string()<<endl;
					cout << "ErrorState: "<<door.get_error_state_string()<<endl;
					cout << "CalibrateState: "<<door.get_calibration_state_string()<<endl;
					break;
				case'x':
					door.close();
					break;
				case's':
					door.stop();
					break;
				case'p':
					cout << "DoorState: "<<door.get_door_state_string()<<endl;
					cout << "ErrorState: "<<door.get_error_state_string()<<endl;
					cout << "CalibrateState: "<<door.get_calibration_state_string()<<endl;
					break;
			}
		}
		//sleep_ms(10);
	}
	return 0;
}
#endif

void welcome_text()
{
	cout << "Hello, program starts! ";
	cout << "Please press SW0 and SW2 to calibrate." << endl;
}
/*
int main()
{

	StateMachine garage_door;
	stdio_init_all();
	welcome_text();
	while (true)
	{
		garage_door.roll_door();
		garage_door.run();
	}

}*/
/*
int main () //Test MQTT connection
{
	stdio_init_all();
	sleep_ms(3000);

//	cyw43_arch_poll();
	//Remember to connect your laptop to the WiFi MP-IOT
	MQTTService MQTT_inst (SSID,PASSWORD);

	//Connect TCP - loop until connection is successful
	MQTT_inst.connect_tcp();

	//Connect MQTT - loop until connection is successful
	MQTT_inst.connect_mqtt();

	//Subsribe to topic, ready to receive command
	//We need to install docker, and run the Eclipse Mosquitto to send command
	MQTT_inst.subscribe("garage/door/command");

	absolute_time_t timeout = get_absolute_time();

	while (true) {
		MQTT_inst.client_yield();

		if (absolute_time_diff_us(get_absolute_time(), timeout) < 0) {
			char* send_topic = "garage/door/status";
			MQTT_inst.send_message("STATUS: IDLE", send_topic);
			timeout = make_timeout_time_ms(5000); //send message every 5 seconds
		}
	}
}
*/

int main()
{
	stdio_init_all();
	sleep_ms(3000);

	MQTTService mqtt(SSID, PASSWORD);
	mqtt.connect_tcp();
	mqtt.connect_mqtt();


	StateMachine sm(mqtt);
	mqtt.set_state_machine(&sm);
	mqtt.subscribe("garage/door/command");

	welcome_text();

	while (true) {
		mqtt.client_yield();
		sm.run();
		sm.roll_door();
	}
}

