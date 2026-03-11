//
// Created by Anh Huynh on 3.3.2026.
//

#ifndef GARAGE_DOOR_MEMORY_H
#define GARAGE_DOOR_MEMORY_H
#include "../Hardware/EEPROM.h"
using namespace std;
enum class MachineState;

class LocalMemory
{
	public:
		LocalMemory() = default;
		LocalMemory(const LocalMemory &) = delete;

		//Write
		void write_new_entry(const char *array);
		void write_addr_zero(const char *array) const;

		//Read
		bool read_an_entry (uint8_t *array) const;
		int  read_all_entries(uint8_t *array); //keep reading until invalid entry or full memory (rd_mem_addr == EEPROM_LIMIT)
		void read_addr_zero(uint8_t *array) const;
		bool get_steps(uint8_t *array, int &total_steps, int &current_steps) const;
		static int convert_str_to_int(const string &str);

		//Get latest state
		MachineState read_all_and_parse(uint8_t * array);
		static MachineState update_latest_state(const string &last_state, const string &error, const string &calib);

	private:
		mutable EEPROM eeprom;
		int wr_mem_addr = 0;
		int rd_mem_addr = 0;

		//Preparation for writing
		static void encapsulate_i2c_frame (uint8_t *array, int payload_size);
		static uint16_t crc16(const uint8_t *data_p, size_t length);
		static int update_addr (int mem_addr, int add_how_much);
		int erase_memory();
		static void convert_str_to_uint (const char *array_to_convert, uint8_t *converted_array, int payload_size);

		//Check validity of read
		static bool check_first_char (const uint8_t *array);
		static bool check_null (const uint8_t *array, int payload_size);
		static bool check_crc (const uint8_t *array, int payload_size);
		static bool check_read (const char *array, int payload_size);

};


#endif //GARAGE_DOOR_MEMORY_H