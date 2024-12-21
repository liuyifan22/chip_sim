#include "core_move_unit.h"
#include "../prims/prim_load.h"
#include "../prims/prim_store.h"


namespace core {
	
	/// @brief Mem unit status transition
	/// Driven signals: move_status, move_busy
	void MoveUnit::moveStateTrans()
	{
		while (true) {
			if (this->rst.read()) {
				this->move_status.write(MOVE_IDLE);
				this->move_busy.write(false);
			}
			else if (this->move_status.read() == MOVE_PRIM) {
				if (this->move_prim_finish.read()) {
					this->move_status.write(MOVE_IDLE);
					this->move_busy.write(false);
				}
			}
			else if (this->move_status.read() == MOVE_IDLE) {
				if (move_prim_start.read()) {
					this->move_status.write(MOVE_PRIM);
					this->move_busy.write(true);
				}
			}
			wait();
		}
	}


	/// @brief Mem unit execution logic
	/// Driven signals: move_prim_finish
	void MoveUnit::movePrimExecution()
	{
		while (true) {
			if ((this->move_status.read() == MOVE_IDLE) || this->rst.read()) {
				this->move_prim_finish.write(false);
			}
			else if (this->move_status == MOVE_PRIM) {
				sc_bv<MEM_PORT_WIDTH> prim_temp = prim_in.read();  // read prim from prim_in
				if (prim_temp.range(3, 0) == "0010") {
					/************************* Store *************************/
					prims::PrimStore prim_store(prim_temp);  // decode prim
					//cout << "Store: " << prim_store.sram_addr_ << " " << prim_store.dram_addr_ << " " << prim_store.data_length_ << endl;
					for (auto i = 0; i < prim_store.data_length_; i++) {  // write data to dram one by one
						sc_bv<MEM_PORT_WIDTH> temp;
						this->core_mem_port->read(prim_store.sram_addr_ + i, temp);
						this->dram_port->write(prim_store.dram_addr_ + i, temp, 1);
					}
				}
				else if (prim_temp.range(3, 0) == "0011") {
					/************************* Load *************************/
					prims::PrimLoad prim_load(prim_temp);
					//cout << "Load: " << prim_load.sram_addr_ << " " << prim_load.dram_addr_ << " " << prim_load.data_length_ << endl;
					for (auto i = 0; i < prim_load.data_length_; i++) {  // read data from dram one by one
						sc_bv<MEM_PORT_WIDTH> temp;
						this->dram_port->read(prim_load.dram_addr_ + i, temp);
						this->core_mem_port->write(prim_load.sram_addr_ + i, temp, 1);
					}
				}
				this->move_prim_finish.write(true);
			}
			wait();
		}
	}


	/// @brief Read data blocks from sram
	/// @param data_request_list a map from data name to <address, length>
	/// @return Data read from sram, in a form of <name, data>
	inline map<string, vector<sc_bv<MEM_PORT_WIDTH>>> MoveUnit::readCoreMemory(map<string, pair<uint64_t, uint64_t>> data_request_list)
	{
		map<string, vector<sc_bv<MEM_PORT_WIDTH>>> data_read_out;
		for (auto iter = data_request_list.begin(); iter != data_request_list.end(); iter++) {
			string data_name = iter->first;
			uint64_t addr = iter->second.first;
			uint64_t data_length = iter->second.second;
			for (auto i = 0; i < data_length; i++) {  // read data from sram one by one
				sc_bv<MEM_PORT_WIDTH> temp;
				this->core_mem_port->read(addr + i, temp);
				data_read_out[data_name].push_back(temp);
			}
		}
		return data_read_out;
	}

	/// @brief Write data to sram
	/// @param data_list Data to be written to sram, in a form of <name, data>
	inline void MoveUnit::writeCoreMemory(map<uint64_t, vector<sc_bv<MEM_PORT_WIDTH>>> data_list)
	{
		for (auto iter = data_list.begin(); iter != data_list.end(); iter++) {
			uint64_t addr = iter->first;
			vector<sc_bv<MEM_PORT_WIDTH>> data = iter->second;
			uint64_t length = data.size();
			for (auto i = 0; i < length; i++) {  // write data to sram one by one
				this->core_mem_port->write(addr + i, data[i], 1);
			}
		}
	}

	/// @brief Read data blocks from dram
	/// @param data_request_list a map from data name to <address, length>
	/// @return Data read from dram, in a form of <name, data>
	inline map<string, vector<sc_bv<MEM_PORT_WIDTH>>> MoveUnit::readDram(map<string, pair<uint64_t, uint64_t>> data_request_list)
	{
		map<string, vector<sc_bv<MEM_PORT_WIDTH>>> data_read_out;
		for (auto iter = data_request_list.begin(); iter != data_request_list.end(); iter++) {
			string data_name = iter->first;
			uint64_t addr = iter->second.first;
			uint64_t data_length = iter->second.second;

			sc_bv<MEM_PORT_WIDTH>* load_data = new sc_bv<MEM_PORT_WIDTH>[data_length];
			this->dram_port->burst_read(addr, data_length, load_data);  // read data from dram in a burst read 
			for (auto i = 0; i < data_length; i++) {
				data_read_out[data_name].push_back(load_data[i]);
			}
		}
		return data_read_out;
	}

	/// @brief Write data to dram
	/// @param data_list Data to be written to dram, in a form of <name, data>
	inline void MoveUnit::writeDram(map<uint64_t, vector<sc_bv<MEM_PORT_WIDTH>>> data_list)
	{
		for (auto iter = data_list.begin(); iter != data_list.end(); iter++) {
			uint64_t addr = iter->first;
			vector<sc_bv<MEM_PORT_WIDTH>> data = iter->second;
			uint64_t length = data.size();
			sc_bv<MEM_PORT_WIDTH>* temp = new sc_bv<MEM_PORT_WIDTH>[length];
			memcpy(temp, &data[0], data.size() * sizeof(data[0]));
			this->dram_port->burst_write(addr, length, temp, 1);
		}
	}

}