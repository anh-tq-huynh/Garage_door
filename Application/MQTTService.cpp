//
// Created by Anh Huynh on 25.2.2026.
//

#include "MQTTService.h"
#include "../private.h"
#include <string>


#include <iostream>

MQTTService::MQTTService(const string &ssid, const string &pwd) : ssid(ssid), pwd(pwd), ipstack(ssid.c_str(), pwd.c_str()),
                                                    client(ipstack), data(), topic(), rc()
{
	broker_ip             = BROKER_IP;
	port                  = PORT;
	data                  = MQTTPacket_connectData_initializer;
	data.MQTTVersion      = 3;
	topic                 = const_cast<char *>("Garage-door-status");
	data.clientID.cstring = topic;
}

void MQTTService::connect_mqtt()
{
	int retry_count = 1;
	rc = client.connect(data);
	while (rc != 0 && retry_count <= 5)
	{
		cout << "Connect to MQTT failed. Attempt " << retry_count;
		retry_count++;
		tight_loop_contents();
		rc = client.connect(data);
		sleep_ms(2);
	}
	if (retry_count > 5)
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
	if (rc != 1)
	{
		cout << "rc from TCP connect is " << rc << endl;
	}
	int retry_count = 1;
	rc = ipstack.connect(broker_ip.c_str(), port);
	while ( rc != 1 && retry_count <= 5
	)
	{
		cout << "Connect to TCP failed. Attempt " << retry_count;
		retry_count++;
		tight_loop_contents();
		rc = ipstack.connect(broker_ip.c_str(), port);
		sleep_ms(2);
	}
	if (retry_count > 10)
	{
		cout << "TCP connect failed after 10 attempts. Cannot reach Broker, check your IP address or Wi-Fi connection";
		while (true)
		{
			tight_loop_contents();
		}
	}
	cout << "TCP is connected." << endl;
	tcp_is_connect = true;
}

void MQTTService::set_qos(int qos)
{
	mqtt_qos = qos;
}

void messageArrived(const MQTT::MessageData &md)
{
	MQTT::Message &message = md.message;

	cout << "Message arrived: qos " << message.qos << ", ";
	cout << "retained " << message.retained << ", ";
	cout << "dup " << message.dup <<", ";
	cout << "packetid" << message.id << "\n";

	cout << "Payload " << message.payload << "\n";
}

void MQTTService::subscribe()
{
	rc = client.subscribe(topic, MQTT::QOS2, reinterpret_cast<MQTT::Client<IPStack, Countdown>::messageHandler>(messageArrived));
	if (rc != 0)
	{
		cout << "rc from MQTT subscribe is " << rc << endl;
	}
	cout << "MQTT subsribed" << endl;
}

void MQTTService::send_message(const string &msg)
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
				printf("rc from MQTT connect is %d\n", rc);
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
				cout << "Msg nr: " << ++msg_count << "QoS 1 message" << endl;
				cout << "\n";

				message.qos        = MQTT::QOS1;
				message.payloadlen = strlen(buf) + 1;
				rc                 = client.publish(topic, message);

				cout << "Publish rc = " << rc << endl;
				++mqtt_qos;
				break;

			case 2:
				// Send and receive QoS 2 message
				cout << "Msg nr: " << ++msg_count << "QoS 2 message" << endl;
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
