//
// Created by Anh Huynh on 3.3.2026.
//

#include "LocalMemory.h"
#define PAGE_SIZE 64
#define EEPROM_LIMIT 4096 //max 64 pages
#define MAX_START 4032
#define START_EEPROM 0
#include <cstring>
#include <iostream>
using namespace std;
#include "GarageDoor.h"

#define UNCALIBRATED_STATUS "Door: Uncalibrated"
#define CALIBRATED_STATUS "Door: Calibrated"
#define OPEN_STATUS "Door: Open"
#define CLOSED_STATUS "Door: Closed"
#define STOPPED_STATUS "Door: Stopped"
#define ERROR_STATUS "Door: Error"
#define OPENING_STATUS "Door: Opening"
#define CLOSING_STATUS "Door: Closed"

#define ERROR_YES "Error: Yes"
#define ERROR_NO "Error: No"

#define CALIBRATED_YES "Calibrated: Yes"
#define CALIBRATED_NO "Calibrated: No"
void LocalMemory::write_new_entry(const char *array)
{
	uint8_t to_write[PAGE_SIZE];
	int payload_size = static_cast<int>(strlen(array));
	convert_str_to_uint(array, to_write, payload_size +1);
	encapsulate_i2c_frame(to_write,payload_size);
	//printf("To write %s\n", to_write);
	/*printf("To write to Memory %d: [", wr_mem_addr);
	for (int i = 0; i < payload_size+3; i++)
	{
		printf("%c ",to_write[i]);
	}
	printf("]\n");*/

	if (wr_mem_addr <= MAX_START)
	{
		eeprom.write_byte(to_write,payload_size +3,wr_mem_addr);
		wr_mem_addr = update_addr(wr_mem_addr, PAGE_SIZE);
	}

	else
	{
		//printf("Erasing memory\n");
		cout << "Erasing memory" << endl;
		erase_memory();
		eeprom.write_byte(to_write,payload_size +3,wr_mem_addr);
		wr_mem_addr = update_addr(wr_mem_addr, PAGE_SIZE);
	}
	//printf("Next write memory %d\n", wr_mem_addr);
}


uint16_t LocalMemory::crc16(const uint8_t *data_p, size_t length) {
	uint8_t x;
	uint16_t crc = 0xFFFF;
	while (length--) {
		x = crc >> 8 ^ *data_p++;
		x ^= x >> 4;
		crc = (crc << 8) ^ static_cast<uint16_t>(x << 12) ^ static_cast<uint16_t>(x << 5) ^ static_cast<uint16_t>(x);
	}
	return crc;
}

int LocalMemory::update_addr (int mem_addr, int add_how_much)
{
	int new_mem_addr = mem_addr + add_how_much;
	return new_mem_addr;
}

int LocalMemory:: erase_memory ()
{
	printf("Clearing memory\n");

	constexpr int number_of_states_to_save = 5;
	int rd = EEPROM_LIMIT - PAGE_SIZE * number_of_states_to_save;
	wr_mem_addr = START_EEPROM;

	// Copy last N pages to the start
	for (int i = 0; i < number_of_states_to_save; i++) {
		char state[PAGE_SIZE];
		eeprom.read_byte(reinterpret_cast<uint8_t *>(state), PAGE_SIZE, rd);
		rd += PAGE_SIZE;
		write_new_entry(state);
	}

	// Erase ALL memory EXCEPT the saved region
	uint8_t clear[PAGE_SIZE] = {0xFF};
	for (int addr = wr_mem_addr; addr < EEPROM_LIMIT; addr += PAGE_SIZE) {
		eeprom.write_byte(clear, PAGE_SIZE, addr);
	}
	return wr_mem_addr; // next free memory location
}


void LocalMemory:: encapsulate_i2c_frame (uint8_t *array, int payload_size)
{
	array [payload_size]   = '\0';
	uint16_t crc           = crc16(array, payload_size +1);
	auto  crc_high_byte = static_cast<uint8_t>(crc >> 8);
	auto  crc_low_byte  = static_cast<uint8_t>(crc);


	array [payload_size + 1] = crc_high_byte;
	array [payload_size + 2] = crc_low_byte;

	//printf("CRC high byte: %u\n", crc_high_byte);
	//printf("CRC low byte: %u\n", crc_low_byte);
}

void LocalMemory:: convert_str_to_uint (const char *array_to_convert, uint8_t *converted_array, const int payload_size)
{
	int i = 0;
	while (i < payload_size)
	{
		converted_array[i] = static_cast<uint8_t>(array_to_convert[i]);
		i++;
	}
}

bool LocalMemory::check_read (const char *array, int payload_size)
{
	//printf("Array to check: %s \n", array);

	const bool array_not_empty = check_first_char(reinterpret_cast<uint8_t *>(const_cast<char *>(array)));
	const bool null_exist      = check_null(reinterpret_cast<uint8_t *>(const_cast<char *>(array)), payload_size);
	const bool crc_is_valid    = check_crc(reinterpret_cast<uint8_t *>(const_cast<char *>(array)), payload_size);

	return array_not_empty && null_exist && crc_is_valid;
}

bool LocalMemory:: check_first_char (const uint8_t *array)
{
	return array[0] != 0xFF;
}

bool LocalMemory::check_null (const uint8_t *array, int payload_size)
{

	for (int i = 0; i < payload_size; i++)
	{
		if (array[i] == 0x00)
		{
			return true;
		}
	}
	return false;
}

bool LocalMemory::check_crc (const uint8_t *array, int payload_size)
{
	return crc16( (uint8_t*)array, payload_size) == 0;
}

void print_entry (const uint8_t *array)
{
	for (int i = 0; array[i] != 0x00; i++)
	{
		printf("%c", array[i]);
	}
}

bool LocalMemory:: read_an_entry (uint8_t *array) const
{
	char temp_buffer[PAGE_SIZE];
	eeprom.read_byte(reinterpret_cast<uint8_t *>(temp_buffer),PAGE_SIZE,rd_mem_addr);

	int read_size = static_cast<int>(strlen(temp_buffer)) + 3;

	strcpy(reinterpret_cast<char *>(array), temp_buffer);
	const bool read_is_valid = check_read(temp_buffer, read_size);
	return read_is_valid;
}

int LocalMemory:: read_all_entries(uint8_t *array) //keep reading until invalid entry or full memory (rd_mem_addr == EEPROM_LIMIT)
{
	bool read_is_valid = true;
	uint8_t temp_buffer[PAGE_SIZE];

	while (read_is_valid && rd_mem_addr < EEPROM_LIMIT)
	{
		eeprom.read_byte(temp_buffer, PAGE_SIZE,rd_mem_addr);
		read_is_valid = read_an_entry(temp_buffer);
		if (read_is_valid)
		{
			strcpy(reinterpret_cast<char *>(array), reinterpret_cast<char *>(temp_buffer));
			rd_mem_addr = update_addr(rd_mem_addr, PAGE_SIZE);
		}
	} //at this point rd_mem_addr stores the next empty memory slot
	//we want to return the latest read memory
	if (rd_mem_addr > EEPROM_LIMIT)
	{
		rd_mem_addr = EEPROM_LIMIT;
	}

	else if (rd_mem_addr != START_EEPROM)
	{
		rd_mem_addr -= PAGE_SIZE;
	}

	//As the latest state is always "Boot" from the start of the program, we check the value right before it
	//i2c_read_byte(array, PAGE_SIZE,rd_mem_addr - PAGE_SIZE);
	//printf("Stop reading at %d\n", rd_mem_addr);
	if (array != nullptr)
	{
		printf("\nLast state: %s\n", reinterpret_cast<char *>(array));
	}
	return rd_mem_addr;
}

