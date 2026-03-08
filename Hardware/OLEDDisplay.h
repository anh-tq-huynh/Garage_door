//
// Created by Yue on 1.3.2026.
//

#ifndef GARAGE_DOOR_OLEDDISPLAY_H
#define GARAGE_DOOR_OLEDDISPLAY_H

#include <string>
#include <memory>
#include "ssd1306.h"
#include "PicoI2CBus.h"
#include "PicoI2CDevice.h"

class OLEDDisplay {
public:
    OLEDDisplay();
    OLEDDisplay(const OLEDDisplay &) = delete;
    void show_status(const std::string &door_state, const std::string &error_state, const std::string &calib_state);
    void clear();
		void show_sw2_sw0();

    void waiting_screen();

    void show_sw1();
		void show_error();
private:
    std::shared_ptr<PicoI2CBus> i2c_bus;
    std::shared_ptr<PicoI2CDevice> i2c_device;
    ssd1306 display;
};

#endif //GARAGE_DOOR_OLEDDISPLAY_H
