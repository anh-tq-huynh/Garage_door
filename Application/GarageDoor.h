//
// Created by Yue on 18.2.2026.
//

#ifndef GARAGE_DOOR_GARAGEDOOR_H
#define GARAGE_DOOR_GARAGEDOOR_H

#include <string>

#include "../Hardware/Leds.h"
#include "../Hardware/StepperMotor.h"
#include "../Hardware/LimitSwitch.h"
#include "../Hardware/RotaryEncoder.h"
using namespace std;


enum class GarageDoorState
{
	UNCALIBRATED, CALIBRATING, CALIBRATED, OPEN, CLOSED, OPENING, CLOSING, STOPPED, ERROR
};

enum class DoorCommand
{
	CLOSE, OPEN, STOP, CALIBRATE, IDLE, MOVE_TO_TARGET
};

class GarageDoor
{
	public:
		GarageDoor(int motorA, int          motorB, int           motorC, int   motorD,
		           int limitSwitchLeft, int limitSwitchRight, int encoderA, int encoderB);

		//Operation of the door
		void start_calibration();
		void open();
		void close();
		void stop();
		void move_to_target();
		bool execute();// Call this periodically to update the state of the door

		//Get info
		bool is_calibrated() const { return calibrated; };
		int  get_total_step() const { return total_steps_calibration; }
		int  get_current_step() const { return current_step; }
		bool is_error_state() const { return is_error; }
		int get_last_dir() const;
		GarageDoorState get_last_state() const;
		string get_steps_data() const;

		//Set door data
		void set_error(bool is_error_state);
		void set_calibration(bool calib_done);
		void set_target_steps(int percentage);
		void set_current_steps(const int &current_steps);
		void set_total_steps(const int &total_steps);

	private:
		StepperMotor motor;
		// when I say "Left", it represents the side has a nail on the left.
		// also the side has a stepper motor underneath
		LimitSwitch   limitSwitchLeft;
		LimitSwitch   limitSwitchRight;
		RotaryEncoder encoder;
		int           target;


		GarageDoorState state                   = GarageDoorState::UNCALIBRATED;
		bool            calibrated              = false;
		bool is_error = false;
		int             total_steps_calibration = 0;
		int             current_step            = 0;
		int             margin                  = 0;

		int              last_direction  = 1;
		int              stuck_counter   = 0;
		static const int STUCK_THRESHOLD = 500; // Number of steps to consider the door stuck

		void drive_to_limit(LimitSwitch &limit, int direction);
		bool check_if_stuck(bool changed);
		void free_encoder_events();
};

#endif //GARAGE_DOOR_GARAGEDOOR_H
