//
// Created by Anh Huynh on 3.3.2026.
//

#include "LocalMemory.h"
#define PAGE_SIZE 64
#define EEPROM_LIMIT 4096 //max 64 pages
#define MAX_START 4032
#define START_EEPROM 64
#define ADDR_ZERO 0
#include <cstring>
#include <iostream>
#include <sstream>
using namespace std;
#include "GarageDoor.h"
#include "StateMachine.h"

void LocalMemory::write_new_entry(const char *array)
{
	uint8_t to_write[PAGE_SIZE];
	int payload_size = static_cast<int>(strlen(array));
	convert_str_to_uint(array, to_write, payload_size +1);
	encapsulate_i2c_frame(to_write,payload_size);

	if (wr_mem_addr > MAX_START)
	{
		cout << "Erasing memory" << endl;
		erase_memory();
	}
	int success = eeprom.write_byte(to_write,payload_size +3,wr_mem_addr);
	wr_mem_addr = update_addr(wr_mem_addr, PAGE_SIZE);
	if (!success)
	{
		cout << "Write failed, please check!" << endl;
	}
}

void LocalMemory::write_addr_zero(const char *array) const
{
	uint8_t to_write[PAGE_SIZE];
	int payload_size = static_cast<int>(strlen(array));
	convert_str_to_uint(array, to_write, payload_size +1);
	encapsulate_i2c_frame(to_write,payload_size);

	int success = eeprom.write_byte(to_write,payload_size +3,ADDR_ZERO);
	if (!success)
	{
		cout << "Write failed, please check!" << endl;
	}
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
	wr_mem_addr = rd_mem_addr; //Next available slot for writing

	if (rd_mem_addr > EEPROM_LIMIT)
	{
		rd_mem_addr = EEPROM_LIMIT;
	}

	else if (rd_mem_addr != START_EEPROM)
	{
		rd_mem_addr -= PAGE_SIZE; //Latest read memory address
	}

	if (array != nullptr)
	{
		printf("\nLast state: %s\n", reinterpret_cast<char *>(array));
		return 1;
	}
	return 0;
}


void LocalMemory::read_addr_zero(uint8_t *array) const
{
	char temp_buffer[PAGE_SIZE];
	eeprom.read_byte(reinterpret_cast<uint8_t *>(temp_buffer),PAGE_SIZE,ADDR_ZERO);

	int read_size = static_cast<int>(strlen(temp_buffer)) + 3;

	if (check_read(temp_buffer, read_size))
	{
		strcpy(reinterpret_cast<char *>(array), temp_buffer);
	}
	//cout << array;

}

bool LocalMemory::get_steps(uint8_t *array, int &total_steps, int &current_steps) const
{
	read_addr_zero(array);
	ostringstream convert;
	for (int i = 0; array[i] != '\0'; i++)
	{
		convert << array[i];
	}
	convert << '\0';

	string s_array = convert.str();
	size_t total_steps_start{0};
	string::size_type total_steps_end = s_array.find(' ', total_steps_start);
	size_t total_steps_size = total_steps_end - total_steps_start;
	string s_total_steps = s_array.substr(total_steps_start, total_steps_size);

	if ((total_steps = convert_str_to_int(s_total_steps)) != -1) //only continue if success
	{
		string s_current_steps = s_array.substr(total_steps_end +1);
		current_steps = convert_str_to_int(s_current_steps);
	}
	if (total_steps == -1 || current_steps == -1)
	{
		total_steps = 0;
		current_steps = 0;
		return false;
	}

	return true;

}

int LocalMemory::convert_str_to_int(const string &str)
{
	char* endptr;
	int converted = strtol(str.c_str(), &endptr, 10);

	if (endptr != str.c_str()) {
		return converted;
	}
	cout << "Invalid data." << endl;
	return -1;
}

MachineState LocalMemory::read_all_and_parse(uint8_t * array)
{
	if (read_all_entries(array)) //If there are data in EEPROM
	{
		ostringstream convert;
		for (int i = 0; array[i] != '\0'; i++)
		{
			convert << array[i];
		}
		convert << '\0';
		string s_array = convert.str();

		size_t state_start{0};
		string::size_type state_end = s_array.find('|', state_start);

		if (state_end != string::npos)
		{
			size_t state_size = state_end - state_start;
			string last_state = s_array.substr(state_start, state_size);

			size_t error_start{state_end + 1};
			size_t error_end = s_array.find('|',error_start);
			size_t error_size = error_end - error_start;
			string error = s_array.substr(error_start,error_size);

			size_t calib_start{error_end + 1};
			size_t calib_end = s_array.find('\0',calib_start);
			size_t calib_size = calib_end - calib_start;
			string calib = s_array.substr(calib_start,calib_size);

			return update_latest_state(last_state,error,calib);
		}
		else
		{
			return MachineState::IDLE;
		}
	}
	return MachineState::IDLE;
}

MachineState LocalMemory::update_latest_state(const string &last_state, const string &error, const string &calib)
{
	if (calib == "Not calibrated")
	{
		return MachineState::UNCALIBRATED;
	}
	else if (error == "Door stuck")
	{
		return MachineState::ERROR;
	}
	else
	{
		if (last_state == "In between - closing")
		{
			return MachineState::STOPPED_CLOSING;

		} else if (last_state == "In between - opening")
		{
			return MachineState::STOPPED_OPENING;

		}else if (last_state == "Open")
		{
			return MachineState::OPEN;

		}else if (last_state == "Closed")
		{
			return MachineState::CLOSE;

		}
		else //last_state == "Unknown"
		{
			return MachineState::UNCALIBRATED; //need to recalibrate to know which direction to run
		}
	}
}




