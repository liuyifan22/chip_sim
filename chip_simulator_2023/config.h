#pragma once

using namespace std;

// global
constexpr auto MEM_PORT_WIDTH = 128;  // bits

// memory capacity
constexpr auto DRAM_DEPTH_PER_CORE = 786432;  // in MEM_PORT_WIDTH
constexpr auto CORE_MEM_DEPTH = 16384;  // in MEM_PORT_WIDTH
// constexpr auto PRIM_BUFFER_DEPTH_PER_CORE = 1024;

// memory specification
constexpr auto RAM_READ_LATENCY = 1;
constexpr auto RAM_WRITE_LATENCY = 1;
constexpr auto RAM_STATIC_POWER = 0.001;
constexpr auto RAM_READ_ENERGY = 0.1;
constexpr auto RAM_WRITE_ENERGY = 0.2;


constexpr auto DRAM_READ_LATENCY = 50;
constexpr auto DRAM_WRITE_LATENCY = 50;
constexpr auto DRAM_BURST_WRITE_LATENCY = 3;
constexpr auto DRAM_BURST_READ_LATENCY = 3;
constexpr auto DRAM_REFRESH_POWER = 0.1;
constexpr auto DRAM_READ_ENERGY = 10;
constexpr auto DRAM_WRITE_ENERGY = 20;
constexpr auto DRAM_TRANSFER_ENERGY = 50;
constexpr auto DRAM_LINE = 10;
constexpr auto DRAM_ENERGY_FACTOR = 0.9;

// control unit
constexpr auto DEP_NUMBER = 8;

// NoC
constexpr auto DATA_WIDTH = 8;  // Byte

// PE unit
constexpr auto I_BYTE = 32;

// Visual

constexpr auto Mem_usage_thre = 2;
