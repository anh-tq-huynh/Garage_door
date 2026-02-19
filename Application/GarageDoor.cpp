//
// Created by Yue on 18.2.2026.
//

#include "GarageDoor.h"

#include <iostream>

#include "pico/time.h"

GarageDoor:: GarageDoor(int motorA, int motorB, int motorC, int motorD,
    int limitSwitchLeft, int limitSwitchRight, int encoderA, int encoderB)
    : motor(motorA, motorB, motorC, motorD),
      limitSwitchLeft(limitSwitchLeft),
      limitSwitchRight(limitSwitchRight),
      encoder(encoderA, encoderB)
{
}

// std::string GarageDoor::get_state_string() const {
//     switch (state) {
//         case GarageDoorState::UNCALIBRATED: return "Uncalibrated";
//         case GarageDoorState::CALIBRATING: return "Calibrating";
//         case GarageDoorState::OPEN: return "Opened";
//         case GarageDoorState::CLOSED: return "Closed";
//         case GarageDoorState::OPENING: return "Opening";
//         case GarageDoorState::CLOSING: return "Closing";
//         case GarageDoorState::STOPPED: return "Stopped";
//         case GarageDoorState::ERROR: return "Error";
//         default: return "Unknown";
//     }
// }

void GarageDoor::drive_to_limit(LimitSwitch &limit, int direction) {
    while (!limit.isTriggered()) {
        motor.step(direction);
    }
    motor.stop();
}

bool GarageDoor::check_if_stuck(bool changed)
{
    if (changed) {
        stuck_counter = 0;
        return false;
    } else {
        stuck_counter++;
        if (stuck_counter > STUCK_THRESHOLD) {
            motor.stop();
            state = GarageDoorState::ERROR;
            calibrated = false;
            return true;
        }
        return false;
    }
}

int GarageDoor::start_calibration() {
    state = GarageDoorState::CALIBRATING;
    calibrated = false;
    total_steps_calibration = 0;
    current_step = 0;

    // first go to right limit, set right side as calibration starting point
    drive_to_limit(limitSwitchRight, -1);

    // clear encoder events if there are any before we start counting steps
    int dummy = 0;
    while (encoder.try_get_event(dummy)) {
    }

    //start from the close limit to count steps
    int encoder_steps = 0;
    int steps_taken = 0;

    // move forward until trigger the left limit.
    while (!limitSwitchLeft.isTriggered()) {
        motor.step(1);
        steps_taken++;

        // check encoder event in case door is stuck
        int event =0;
        if (encoder.try_get_event(event)) {
            encoder_steps += event;
        }
    }
    // door is at the left limit, now move back until the limit switch is not triggered to find the margin.
    margin =0;
    while (limitSwitchLeft.isTriggered()) {
        motor.step(-1);
        margin++;
    }
    motor.stop();

    // current_step=0 means CLOSED, current_step=total_steps_calibration means OPEN.
    total_steps_calibration = steps_taken - margin;
    current_step = total_steps_calibration;
    calibrated = true;
    state = GarageDoorState::CLOSED;
    std::cout<<"Total steps:"<<total_steps_calibration<<std::endl;

    return total_steps_calibration, margin;
}

void GarageDoor::open() {
    // if not calibrated or already open, do nothing
    if (!calibrated || state == GarageDoorState::OPEN) {
        return;
    }

    last_direction = -1;
    state = GarageDoorState::OPENING;
    stuck_counter = 0;
}

void GarageDoor::close() {
    if (!calibrated || state == GarageDoorState::CLOSED) {
        return;
    }

    last_direction = 1;
    state = GarageDoorState::CLOSING;
    stuck_counter = 0;
}

void GarageDoor::stop() {
    if (state == GarageDoorState::OPENING || state == GarageDoorState::CLOSING) {
        motor.stop();
        state = GarageDoorState::STOPPED;
    }
}

void GarageDoor::operate() {
    if (!calibrated) {
        return;
    }

    switch (state) {
        case GarageDoorState::CLOSED:
            //Door is closed → door starts to open
            open();
            break;

        case GarageDoorState::OPEN:
            //Door open → door starts to close
            close();
            break;

        case GarageDoorState::OPENING:
        case GarageDoorState::CLOSING:
            // Door is current opening or closing → door stops
            stop();
            state = GarageDoorState::STOPPED;
            break;

        case GarageDoorState::STOPPED:
            // Door was earlier stopped by pressing the button → door starts movement to the opposite direction
            if (last_direction == -1) {  // before was opening (towards right)
                last_direction = 1;      // now towards left, closing
                state = GarageDoorState::CLOSING;
            } else {                     // before was closing (towards left)
                last_direction = -1;     // now towards right, opening
                state = GarageDoorState::OPENING;
            }
            stuck_counter = 0 ;
            break;

        default:
            break;
    }
}

void GarageDoor::update() {
    int event = 0;
    bool encoder_changed = encoder.try_get_event(event);

    // helper code to test if the encoder event
    // we need to know approx how many steps the encoder will trigger when the door is moving, to determine the threshold for stuck detection.

    // static int encoder_steps = 0;
    // encoder_steps++;
    // if (encoder_changed) {
    //     std::cout<<encoder_steps<<std::endl;
    //     encoder_steps =0;
    // }

    if (state == GarageDoorState::OPENING) {
        if (current_step <= margin) {
            motor.stop();
            state = GarageDoorState::OPEN;
            return;
        }

        motor.step(-1); // toward right
        current_step--;

        if (check_if_stuck(encoder_changed)) return;

    } else if (state == GarageDoorState::CLOSING) {
        if (current_step >= total_steps_calibration) {
            motor.stop();
            state = GarageDoorState::CLOSED;
            return;
        }

        motor.step(1); // toward left
        current_step++;

        if (check_if_stuck(encoder_changed)) return;
    }
}

std::string GarageDoor::get_door_state_string() const {
    if (state == GarageDoorState::OPEN) return "Open";
    if (state == GarageDoorState::CLOSED) return "Closed";
    return "In between";
}

std::string GarageDoor::get_error_state_string() const {
    if (state == GarageDoorState::ERROR) return "Door stuck";
    return "Normal";
}

std::string GarageDoor::get_calibration_state_string() const {
    return calibrated ? "Calibrated" : "Not calibrated";
}