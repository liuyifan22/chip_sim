#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "../core/core.h"
#include "../prims/primitive.h"
#include "../prims/prim_load.h"
#include "../prims/prim_store.h"
#include "../prims/prim_jump.h"

#include "../trace_engine/Event_engine.h"
#include "../utils/file_compare.h"


class core_tb : public sc_module {
public:
	SC_HAS_PROCESS(core_tb);
	core_tb(const sc_module_name& nm, string test_name, core::Core* test_core) :
		sc_module(nm), test_core(test_core), test_name_(test_name)
	{
		// connect IOs for core 
		this->test_core->rst(reset);
		this->test_core->start(start);
		this->test_core->stop(stop);
		this->test_core->core_in_idle(core_in_idle);

		// generate data and instructions
		this->data_gen();

		// register threads
		SC_THREAD(test_cores);
	}

	void data_gen();
	void test_cores();
	void before_start();
	void after_stop();

public:
	sc_signal<bool> reset;
	sc_signal<bool> start;
	sc_signal<bool> stop;
	sc_signal<bool> core_in_idle;

	core::Core* test_core;
	vector<sc_bv<MEM_PORT_WIDTH> > instructions;
	map<uint64_t, vector<sc_bv<MEM_PORT_WIDTH> > > data_list;

	sc_signal<bool> CPU_send_signal;
	sc_signal<bool> CPU_wait_receive_signal;

	string test_name_;
};

uint64_t test_data_length = 0x110;
uint64_t init_sram_addr = 0x10;
uint64_t moved_sram_addr = 0x1001;
uint64_t dram_addr = 0x3000;

void core_tb::data_gen()
{
	// prepare prims
	assert(init_sram_addr + test_data_length < moved_sram_addr);
	using namespace prims;
	instructions.push_back(convertPrim2Code(
		PrimStore(init_sram_addr, dram_addr, test_data_length)
	));
	instructions.push_back(convertPrim2Code(
		PrimLoad(moved_sram_addr, dram_addr, test_data_length)
	));
	instructions.push_back(convertPrim2Code(
		PrimJump(+3, 1)
	));

	// prepare data
	vector<sc_bv<MEM_PORT_WIDTH> > init_data;
	for (auto i = 0; i < test_data_length; i++) {
		init_data.push_back(sc_bv<MEM_PORT_WIDTH>(rand()));
	}
	data_list.insert(pair<uint64_t, vector<sc_bv<MEM_PORT_WIDTH> > >(init_sram_addr, init_data));
}

void core_tb::before_start()
{
	bool test_pass_flag = true;
	sc_bv<MEM_PORT_WIDTH> temp;
	for (auto i = 0; i < test_data_length; i++) {
		this->test_core->core_memory->read(moved_sram_addr + i, temp);
		if (temp != data_list[init_sram_addr][i]) {
			test_pass_flag = false;
			cout << i << endl;
			break;
		}
	}
	cout << "before running: " << (test_pass_flag ? "pass" : "fail") << endl;
}

void core_tb::after_stop()
{
	bool test_pass_flag = true;
	sc_bv<MEM_PORT_WIDTH> temp;
	for (auto i = 0; i < test_data_length; i++) {
		this->test_core->core_memory->read(moved_sram_addr + i, temp);
		if (temp != data_list[init_sram_addr][i]) {
			test_pass_flag = false;
			cout << i << endl;
			break;
		}
	}
	cout << "after running: " << (test_pass_flag ? "pass" : "fail") << endl;
}

void core_tb::test_cores()
{
	reset.write(false);
	wait(10, SC_NS);
	reset.write(true);
	wait(10, SC_NS);
	reset.write(false);
	wait(10, SC_NS);

	test_core->writeData(data_list);
	test_core->writePrims(0, instructions);

	before_start();

	start.write(false);
	wait(10, SC_NS);
	start.write(true);
	wait(core_in_idle.posedge_event());

	after_stop();

	// print memory usage
	cout << "--------------------- memory usage report ----------------------" << endl;
	double simulater_time = sc_time_stamp().to_double();
	std::cout << "DRAM random access times: " << dram<sc_bv<MEM_PORT_WIDTH> >::random_access_times << std::endl;
	std::cout << "DRAM burst access times: " << dram<sc_bv<MEM_PORT_WIDTH> >::burst_access_times << std::endl;
	std::cout << "DRAM area: " << dram<sc_bv<MEM_PORT_WIDTH> >::area << std::endl;
	std::cout << "DRAM refresh energy: " << dram<sc_bv<MEM_PORT_WIDTH> >::area * simulater_time * DRAM_REFRESH_POWER << std::endl;
	std::cout << "DRAM total access latency: " << dram<sc_bv<MEM_PORT_WIDTH> >::total_access_latency << std::endl;
	std::cout << "DRAM energy consumption: " << dram<sc_bv<MEM_PORT_WIDTH> >::energy_consumption << std::endl;
	std::cout << "SRAM random access times: " << ram<sc_bv<MEM_PORT_WIDTH> >::random_access_times << std::endl;
	std::cout << "SRAM area: " << ram<sc_bv<MEM_PORT_WIDTH> >::area << std::endl;
	std::cout << "SRAM static energy: " << ram<sc_bv<MEM_PORT_WIDTH> >::area * simulater_time * RAM_STATIC_POWER << std::endl;
	std::cout << "SRAM total access latency: " << ram<sc_bv<MEM_PORT_WIDTH> >::total_access_latency << std::endl;
	std::cout << "SRAM energy consumption: " << ram<sc_bv<MEM_PORT_WIDTH> >::energy_consumption << std::endl;

	sc_stop();

}


int sc_main(int argc, char* argv[])
{
	Event_engine* event_engine_test = new Event_engine("my_event_engine");
	FunctionalNoC<sc_bv<MEM_PORT_WIDTH> >* noc = new FunctionalNoC<sc_bv<MEM_PORT_WIDTH> >("noc", event_engine_test);

	core::Core test_core("test_core", CoreLoc(0, 0), noc, event_engine_test);
	core_tb host("host", "single_core_test_load_store", & test_core);

	//printf("Start simulation\n");
	sc_start();

	delete event_engine_test;
	delete noc;

	return 0;
}

