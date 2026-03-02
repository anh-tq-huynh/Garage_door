//
// Created by Anh Huynh on 25.2.2026.
//

#ifndef GARAGE_DOOR_MQTT_H
#define GARAGE_DOOR_MQTT_H
#include <string>

#include "Countdown.h"
#include "IPStack.h"
#include "MQTTPacket.h"
#include "paho.mqtt.embedded-c/MQTTClient/src/MQTTClient.h"
using namespace std;


class MQTTService
{
	public:
		MQTTService(const string &ssid, const string &pwd);
		void connect_mqtt();
		void connect_tcp();

		void send_message(const string &msg);

		void subscribe();

		void set_qos(int qos);

	private:
		string ssid;
		string pwd;
		IPStack ipstack;
		MQTT::Client<IPStack,Countdown> client;
		string broker_ip;
		MQTTPacket_connectData data;
		int port;

		bool tcp_is_connect = false;
		bool mqtt_is_connect = false;

		char* topic;
		int rc;

		absolute_time_t mqtt_send = make_timeout_time_ms(2000);
		int mqtt_qos = 0;
		int msg_count = 0;
};



#endif //GARAGE_DOOR_MQTT_H