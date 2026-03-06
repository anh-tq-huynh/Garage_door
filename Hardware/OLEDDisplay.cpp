//
// Created by Yue on 1.3.2026.
//

#include "OLEDDisplay.h"

// I2C1, SDA=GPIO14, SCL=GPIO15, 400kHz
// SSD1306 default I2C address: 0x3C
OLEDDisplay::OLEDDisplay() :
    i2c_bus(std::make_shared<PicoI2CBus>(1, 14, 15, 400000)),
    i2c_device(std::make_shared<PicoI2CDevice>(i2c_bus, 0x3C)),
    display(i2c_device){clear();}

void OLEDDisplay::show_status(const std::string &door_state, const std::string &error_state, const std::string &calib_state) {
    display.fill(0);
    display.text("Door: " + door_state,  0, 4);
    display.text("Error:" + error_state, 0, 24);
    display.text(calib_state, 0, 44);
    display.show();
}

void OLEDDisplay::clear() {
    display.fill(0);
    display.show();
}

