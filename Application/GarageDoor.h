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


enum class GarageDoorState {
    UNCALIBRATED,CALIBRATING,CALIBRATED,OPEN,CLOSED,OPENING,CLOSING,STOPPED,ERROR
};

enum class DoorCommand
{
	CLOSE, OPEN, STOP, CALIBRATE, IDLE
};

class GarageDoor {
public:
    GarageDoor(int motorA, int motorB, int motorC, int motorD,
        int limitSwitchLeft, int limitSwitchRight, int encoderA, int encoderB);

    void start_calibration();
    void open();
    void close();
    void stop();
    void operate();

    bool        is_calibrated() const { return calibrated; };
    bool        update(); // Call this periodically to update the state of the door
    std::string get_door_state_string() const;
    std::string get_error_state_string() const;
    std::string get_calibration_state_string() const;
    std::string get_full_state_string() const;
    void        print_states() const;
    bool        is_error_state() const {return state == GarageDoorState::ERROR;}
		void    reset_state();
		void    free_encoder_events();
		void    set_state(DoorCommand cmd);
private:
    StepperMotor motor;
    // when I say "Left", it represents the side has a nail on the left.
    // also the side has a stepper motor underneath
    LimitSwitch   limitSwitchLeft;
    LimitSwitch   limitSwitchRight;
    RotaryEncoder encoder;


    GarageDoorState state=GarageDoorState::UNCALIBRATED;
    bool calibrated=false;
    int total_steps_calibration=0;
    int current_step=0;
    int margin = 0;

    int last_direction = 1;
    int stuck_counter = 0;
    static const int STUCK_THRESHOLD = 1000; // Number of steps to consider the door stuck

    void drive_to_limit(LimitSwitch& limit, int direction);
    bool check_if_stuck(bool changed);

};

#endif //GARAGE_DOOR_GARAGEDOOR_H