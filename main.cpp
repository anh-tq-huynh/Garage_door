//
// Created by Anh Huynh on 16.2.2026.
//
#include <fstream>
#include <iostream>
#include <limits>

#include "Hardware/GPIOPin.h"
#include "hardware/gpio.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "Application/GarageDoor.h"
#include "Application/StateMachine.h"
#include "Application/MQTTService.h"
#include "private.h"
#include "Application/LocalMemory.h"
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
	sm.read_eeprom();

	while (true) {
		mqtt.client_yield();
		sm.run();
	}
}
/*
int main ()
{
	stdio_init_all();
	welcome_text();

	GarageDoor door(2,3,6,13,4,5,27,28);
	enum class State {CALIBRATE,WAIT_CALIBRATE, OPEN, CLOSE, OPEN50, CLOSE50, OPEN25, CLOSE25, OPEN75, CLOSE75, IDLE};
	State state = State::CALIBRATE;
	bool started = false;
	bool roll_done = false;


	while (true)
	{
		switch (state)
		{
			case State::CALIBRATE:
				cout << "Calibrate"<< endl;
				door.start_calibration();
				state = State::OPEN25;
				break;
			case State::OPEN25:
				if (!started)
				{
					door.set_target_steps(25);
					door.open();
					started = true;
				}
				roll_done = door.update();

				if (roll_done && started)
				{
					cout << "Finished Open 25. Open 50" << endl;
					state = State::OPEN50;
					started = false;
				}
				break;
			case State::OPEN50:
				if (!started)
				{
					door.set_target_steps(50);
					door.open();
					started = true;
				}
				roll_done = door.update();

				if (roll_done && started)
				{
					cout << "Finished Open 50. Close 25" << endl;
					state = State::CLOSE25;
					started = false;
				}
				break;
			case State::CLOSE25:
				if (!started)
				{
					door.set_target_steps(25);
					door.close();
					started = true;
				}
				roll_done = door.update();

				if (roll_done && started)
				{
					cout << "Finished Close 25." << endl;
					state = State::IDLE;
					started = false;
				}
				break;
			case State::IDLE:
				//cout << "Idle"<< endl;
				break;
		}
		//roll_done = door.update();
		//sleep_ms(10);
	}
}*/

/*
int main ()
{
	stdio_init_all();
	welcome_text();
	MQTTService mqtt(SSID, PASSWORD);
	//mqtt.connect_tcp();
	//mqtt.connect_mqtt();


	StateMachine sm(mqtt);
	mqtt.set_state_machine(&sm);
	bool command_entered = false;

	string input_buffer{};
	while (true)
	{
		int c = getchar_timeout_us(0);

		while (c != PICO_ERROR_TIMEOUT) {
			if (c == '\n' || c == '\r') {
				// User pressed Enter - Process the command
				if (!input_buffer.empty()) {
					cout << "\n> Processing: " << input_buffer << endl;
					sm.handle_mqtt_command(input_buffer.c_str());
					input_buffer = ""; // Clear for next command
				}
				cout << "Enter command: " << flush;
			} else if (c == 127 || c == 8) { // Handle Backspace
				if (!input_buffer.empty()) {
					input_buffer.pop_back();
					putchar('\b'); putchar(' '); putchar('\b'); // Visual delete
				}
			} else {
				input_buffer += (char)c;
				//putchar(c); // Echo the character so you can see what you type
			}
			c = getchar_timeout_us(0); // Check for more characters
		}

		sm.run();
		sm.roll_door();

	}
}*/