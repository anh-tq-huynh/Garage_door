//
// Created by Anh Huynh on 25.2.2026.
//

#ifndef GARAGE_DOOR_MQTT_H
#define GARAGE_DOOR_MQTT_H
#include <string>
#include "IPStack.h"
using namespace std;


class MQTTService
{
	private:
		string ssid;
		string pwd;
		IPStack ipstack;
};



#endif //GARAGE_DOOR_MQTT_H