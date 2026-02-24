//
// Created by Yue on 17.2.2026.
//

#ifndef GARAGE_DOOR_LIMITSWITCH_H
#define GARAGE_DOOR_LIMITSWITCH_H

#include "GPIOPin.h"

class LimitSwitch {
public:
    explicit LimitSwitch(int pin):pin(pin,true,true,true){};
    bool isTriggered() const
    {
        return pin.read();
    }
private:
    GPIOPin pin;
};


#endif //GARAGE_DOOR_LIMITSWITCH_H