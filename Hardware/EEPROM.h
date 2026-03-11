//
// Created by Anh Huynh on 3.3.2026.
//

#ifndef GARAGE_DOOR_EEPROM_H
#define GARAGE_DOOR_EEPROM_H
#include <memory>
#include "PicoI2CBus.h"
#include "PicoI2CDevice.h"


class EEPROM
{
	public:
		EEPROM();
		EEPROM(const EEPROM &) = delete;
		int read_byte( uint8_t *dest, int payload_size, int rd_mem_addr) const;
		int write_byte(const uint8_t *src, int payload_size, int wr_mem_addr) const;


	private:
		std::shared_ptr<PicoI2CBus> eeprom_i2c_bus;
		std::shared_ptr<i2c_device> eeprom_i2c_device;

};


#endif //GARAGE_DOOR_EEPROM_H