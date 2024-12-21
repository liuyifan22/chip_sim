#include "core.h"

vector<sc_bv<MEM_PORT_WIDTH>> core::Core::readMem(uint64_t addr, uint64_t length)
{
    vector<sc_bv<MEM_PORT_WIDTH> > data_read_out;
    for (auto i = 0; i < length; i++) {
        sc_bv<MEM_PORT_WIDTH> data_temp;
        this->core_memory->read(addr + i, data_temp);
        data_read_out.push_back(data_temp);
    }
    return data_read_out;
}

void core::Core::writePrims(uint64_t addr, vector<sc_bv<MEM_PORT_WIDTH>>& prim_list)
{
    uint64_t length = prim_list.size();
    for (auto i = 0; i < length; i++) {
        sc_bv<MEM_PORT_WIDTH> data_temp = prim_list[i];
        this->core_memory->write(addr + i, data_temp, 1);
    }
}

void core::Core::writeData(uint64_t addr, vector<sc_bv<MEM_PORT_WIDTH>>& data)
{
    uint64_t length = data.size();
    for (auto i = 0; i < length; i++) {
        sc_bv<MEM_PORT_WIDTH> data_temp = data[i];
        this->core_memory->write(addr + i, data_temp, 1);
    }
}

void core::Core::writeData(map<uint64_t, vector<sc_bv<MEM_PORT_WIDTH>>>& data_list)
{
    for (auto iter = data_list.begin(); iter != data_list.end(); iter++)
    {
        uint64_t addr = iter->first;
        vector<sc_bv<MEM_PORT_WIDTH>> data = iter->second;
        uint64_t length = data.size();
        for (auto i = 0; i < length; i++) {
            sc_bv<MEM_PORT_WIDTH> data_temp = data[i];
            this->core_memory->write(addr + i, data_temp, 1);
        }
    }
}
