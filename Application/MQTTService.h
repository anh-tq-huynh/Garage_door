//
// Created by Anh Huynh on 25.2.2026.
//

#ifndef GARAGE_DOOR_MQTT_H
#define GARAGE_DOOR_MQTT_H
#include <string>

#include "Countdown.h"
#include "IPStack.h"
#include "paho.mqtt.embedded-c/MQTTClient/src/MQTTClient.h"
using namespace std;


class MQTTService
{
	private:
		string ssid;
		string pwd;
		IPStack ipstack;
		 MQTT::Client<IPStack, Countdown> client;
};



#endif //GARAGE_DOOR_MQTT_H