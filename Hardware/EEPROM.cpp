//
// Created by Anh Huynh on 3.3.2026.
//

#include "EEPROM.h"
#include <iterator>

EEPROM::EEPROM() :
eeprom_i2c_bus(std::make_shared<PicoI2CBus>(0,16,17,100000)),
eeprom_i2c_device(std::make_shared<PicoI2CDevice>(eeprom_i2c_bus,0x50))
{}

int EEPROM::write_byte(const uint8_t *src, int payload_size, int wr_mem_addr) const
{
	uint8_t data_reg[payload_size + 2];
	data_reg[0] = static_cast<uint8_t>(wr_mem_addr >> 8);
	data_reg[1] = static_cast<uint8_t>(wr_mem_addr);

	int index = 2;
	int src_index = 0;
	for (; index < payload_size + 2; index ++)
	{
		data_reg[index] = src[src_index];
		src_index++;
	}

	unsigned int bytes_sent = eeprom_i2c_device->write(data_reg,payload_size +2);
	sleep_ms(10);

	if (bytes_sent != payload_size)
	{
		return 1;
	}
	return 0;
}

int EEPROM::read_byte(uint8_t *dest, int payload_size, int rd_mem_addr) const
{
	uint8_t data_reg[2];
	data_reg[0] = static_cast<uint8_t>(rd_mem_addr >> 8);
	data_reg[1] = static_cast<uint8_t>(rd_mem_addr);

	unsigned int result = eeprom_i2c_device->transaction(data_reg,2,dest,payload_size);

	if (result < payload_size)
	{
		return 1;
	}
	return 0;
}


