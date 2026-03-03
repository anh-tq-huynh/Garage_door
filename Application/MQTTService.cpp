//
// Created by Anh Huynh on 25.2.2026.
//

#include "MQTTService.h"
#include "../private.h"
#include <string>
#include <iostream>

#define MAX_RETRIES 5

MQTTService::MQTTService(const string &ssid, const string &pwd) : ssid(ssid), pwd(pwd), ipstack(ssid.c_str(), pwd.c_str()),
                                                    client(ipstack), data(), rc()
{
	broker_ip             = BROKER_IP;
	port                  = PORT;
	data                  = MQTTPacket_connectData_initializer;
	data.MQTTVersion      = 3;
	data.clientID.cstring = const_cast<char *>("PicoW-sample");
	//topic                 = const_cast<char *>("Garage-door-status");
}

void MQTTService::connect_mqtt()
{
	int retry_count = 1;
	rc = client.connect(data);
	while (rc != 0 && retry_count <= MAX_RETRIES)
	{
		cout << "Connect to MQTT failed. Attempt " << retry_count;
		retry_count++;
		tight_loop_contents();
		rc = client.connect(data);
		sleep_ms(2);
	}
	if (retry_count > MAX_RETRIES)
	{
		cout << "MQTT connect failed after 5 attempts.";
		while (true)
		{
			tight_loop_contents();
		}
	}
	cout << "MQTT is connected." << endl;
	mqtt_is_connect = true;
}

void MQTTService::connect_tcp()
{
	// 	// at serial rc from TCP connect is 0;
	// if (rc != 1)
	// {
	// 	cout << "rc from TCP connect is " << rc << endl;
	// }

	int retry_count = 0;
	// defined MAX_RETRIES to avoid magic number
	while (retry_count < MAX_RETRIES)
	{
		// everycall to ipstack.connect will allocate new tcp struct;
		// we have to close the previous one first
		if (retry_count > 0) {
			ipstack.disconnect();
			sleep_ms(500);
		}

		rc = ipstack.connect(broker_ip.c_str(), port);
		// from lwip/err.h return 0 is ERR_OK, which means success.
		// rc!=0 equals to error
		if (rc == ERR_OK) {
			cout << "TCP initiating connection..." << endl;
			tcp_is_connect = true;
			return;
		}

		cout << "TCP connect failed. Attempt " << retry_count + 1 << endl;
		retry_count++;
	}

	cout << "TCP connect failed after " << MAX_RETRIES
		 << " attempts. Check broker IP and Wi-Fi." << endl;
	while (true)
	{
		tight_loop_contents();
	}
}



// it seems this function hasn't been used at code
// void MQTTService::set_qos(int qos)
// {
// 	mqtt_qos = qos;
// }

void messageArrived(const MQTT::MessageData &md)
{
	MQTT::Message &message = md.message;

	cout << "Message arrived: qos " << message.qos << ", ";
	cout << "retained " << message.retained << ", ";
	cout << "dup " << message.dup <<", ";
	cout << "packetid" << message.id << "\n";
	// covert to string
	cout << "Payload " << static_cast<char*>(message.payload) << "\n";
}

void MQTTService::subscribe(const char* topic)
{
	//rc = client.subscribe(topic, MQTT::QOS2, reinterpret_cast<MQTT::Client<IPStack, Countdown>::messageHandler>(messageArrived));
	rc = client.subscribe(topic, MQTT::QOS1, reinterpret_cast<MQTT::Client<IPStack, Countdown>::messageHandler>(messageArrived));
	if (rc != 0)
	{
		cout << "rc from MQTT subscribe is " << rc << endl;
	}
	cout << "MQTT subsribed" << endl;
}

void MQTTService::send_message(const string &msg, const char* topic)
{
	if (time_reached(mqtt_send))
	{
		mqtt_send = delayed_by_ms(mqtt_send, 2000);

		char buf[100];
		strncpy(buf, msg.c_str(), sizeof(buf));
		buf[sizeof(buf) - 1] = '\0';

		MQTT::Message message{};
		message.retained   = false;
		message.dup        = false;
		message.qos        = MQTT::QOS1;
		message.payload    = static_cast<void*>(buf);
		message.payloadlen = strlen(buf) + 1;

		rc = client.publish(topic, message);
		cout << "Publish rc = " << rc << endl;
	}
}

void MQTTService::client_yield()
{
	client.yield(100); // socket that client uses calls cyw43_arch_poll()
}



/*


void MQTTService::send_message(const string &msg, const char* topic)
{
	if (time_reached(mqtt_send))
	{
		mqtt_send = delayed_by_ms(mqtt_send, 2000);
		if (!client.isConnected())
		{
			cout << "Not connected..." << endl;
			rc = client.connect(data);
			if (rc != 0)
			{
				cout << "rc from MQTT connect is " << rc << endl;
				printf("rc from MQTT connect is z%d\n", rc);
			}
		}
		char          buf[100];
		//int           rc = 0;
		MQTT::Message message{};
		message.retained = false;
		message.dup      = false;
		strncpy(buf, msg.c_str(), sizeof(buf));
		message.payload  = static_cast<void *>(buf);
		switch (mqtt_qos)
		{
			case 0:
				// Send and receive QoS 0 message
				cout << "Msg nr: " << ++msg_count << "QoS 0 message" << endl;
				cout << "\n";

				message.qos        = MQTT::QOS0;
				message.payloadlen = strlen(buf) + 1;
				rc                 = client.publish(topic, message);

				cout << "Publish rc = " << rc << endl;
				++mqtt_qos;
				break;
			case 1:
				// Send and receive QoS 1 message
				cout << "Msg nr: " << ++msg_count << "; QoS 1 message" << endl;
				cout << "\n";

				message.qos        = MQTT::QOS1;
				message.payloadlen = strlen(buf) + 1;
				rc                 = client.publish(topic, message);

				cout << "Publish rc = " << rc << endl;
				++mqtt_qos;
				break;

			case 2:
				// Send and receive QoS 2 message
				cout << "Msg nr: " << ++msg_count << "; QoS 2 message" << endl;
				cout << "\n";
				sprintf(buf, "Msg nr: %d QoS 2 message", ++msg_count);
				printf("%s\n", buf);
				message.qos        = MQTT::QOS2;
				message.payloadlen = strlen(buf) + 1;
				rc                 = client.publish(topic, message);

				cout << "Publish rc = " << rc << endl;
				++mqtt_qos;
				break;
			default:
				mqtt_qos = 0;
				break;
		}
	}
}
*/


