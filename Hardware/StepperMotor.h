//
// Created by Anh Huynh on 17.2.2026.
//

#ifndef GARAGE_DOOR_STEPPERMOTOR_H
#define GARAGE_DOOR_STEPPERMOTOR_H
#include <memory>
#include <vector>
#include "GPIOPin.h"


class StepperMotor
{
	public:
		StepperMotor(int pinA, int pinB, int pinC, int pinD);

		void step(int direction);
		void stop();
	// using delay_ms to control the speed of the motor, smaller delay means faster speed
		void set_speed(int delay_ms);

	private:
		std::vector<std::unique_ptr<GPIOPin>> coils;
		static const int step_sequence[8][4];

		int current_step;
		int step_delay_ms = 1;
};


#endif //GARAGE_DOOR_STEPPERMOTOR_H