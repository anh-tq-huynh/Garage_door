//
// Created by Anh Huynh on 17.2.2026.
//

#include "StepperMotor.h"
#include <memory>
#include <vector>

StepperMotor::StepperMotor(int pinA, int pinB, int pinC, int pinD)
{
	coils.push_back(std::make_unique<GPIOPin>(pinA, false, false, false));
	coils.push_back(std::make_unique<GPIOPin>(pinB, false, false, false));
	coils.push_back(std::make_unique<GPIOPin>(pinC, false, false, false));
	coils.push_back(std::make_unique<GPIOPin>(pinD, false, false, false));
	//Note - to write on the coil we can write as follow
	//coils[0] -> write(1) (write 1 on pinA)
}
