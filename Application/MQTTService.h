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
class StateMachine;

class MQTTService
{
	public:
		//Establish connection
		MQTTService(const string &ssid, const string &pwd);
		void connect_mqtt();
		void connect_tcp();
		bool mqtt_is_connected() const;
		void client_yield();

		//MQTT interaction: send, subscribe, publish
		void send_message(const string &msg, const char* topic);
		void subscribe(const char* topic);
		void publish(const string &msg, const char* topic);

		//Interaction with state machine
		static void set_state_machine(StateMachine* sm);
		void send_status(const string &door_state, const string &error, const string &calib);
		void cmd_response(bool success, bool door_stuck, bool need_calibrated);

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

		//char* topic;
		int rc;

		absolute_time_t mqtt_send  = make_timeout_time_ms(2000);
		absolute_time_t yield_timer = make_timeout_time_ms(50);
		int mqtt_qos = 0;
		int msg_count = 0;

		static StateMachine* state_machine;
		static void static_message_arrived(MQTT::MessageData& md);
};



#endif //GARAGE_DOOR_MQTT_H