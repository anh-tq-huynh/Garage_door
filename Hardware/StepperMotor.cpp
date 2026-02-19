//
// Created by Anh Huynh on 17.2.2026.
//

#include "StepperMotor.h"
#include <memory>
#include <vector>

#include "pico/time.h"

const int StepperMotor::step_sequence[8][4] = {
	{1, 0, 0, 0},
	{1, 1, 0, 0},
	{0, 1, 0, 0},
	{0, 1, 1, 0},
	{0, 0, 1, 0},
	{0, 0, 1, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 1}
};


StepperMotor::StepperMotor(int pinA, int pinB, int pinC, int pinD)
{
	coils.push_back(std::make_unique<GPIOPin>(pinA, false, false, false));
	coils.push_back(std::make_unique<GPIOPin>(pinB, false, false, false));
	coils.push_back(std::make_unique<GPIOPin>(pinC, false, false, false));
	coils.push_back(std::make_unique<GPIOPin>(pinD, false, false, false));
	//Note - to write on the coil we can write as follows
	//coils[0] -> write(1) (write 1 on pinA)
}

void StepperMotor::step(int direction) {
	current_step = (current_step + direction + 8) % 8;

	for (int i = 0; i < coils.size(); i++) {
		coils[i]->write(step_sequence[current_step][i]);
	}
	sleep_ms(step_delay_ms);
}

void StepperMotor::stop() {
	for (auto& coil : coils) {
		coil->write(0);
	}
}

void StepperMotor::set_speed(int delay_ms) {
	step_delay_ms = delay_ms;
}
