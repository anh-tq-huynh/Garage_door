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

	private:
		std::vector<std::unique_ptr<GPIOPin>> coils;
};


#endif //GARAGE_DOOR_STEPPERMOTOR_H