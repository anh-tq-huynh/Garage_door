//
// Created by Anh Huynh on 16.2.2026.
//

#ifndef GARAGE_DOOR_GPIO_H
#define GARAGE_DOOR_GPIO_H
#include <cstdint>


class GPIOPin
{
	public:
		explicit GPIOPin(int pin, bool input = true, bool pullup = true, bool invert = false);
		GPIOPin(const GPIOPin &) = delete;
		~GPIOPin();
		bool read() const;
		void write(bool value) const;
		explicit operator bool() const;
	private:
		static uint32_t pins_in_use;
		int pin;
		bool is_dormant;
		bool is_input;
};


#endif //GARAGE_DOOR_GPIO_H