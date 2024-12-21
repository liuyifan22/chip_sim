#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <random>

#include "../core/core.h"
#include "../prims/primitive.h"
#include "../prims/prim_relu.h"
#include "../prims/prim_mm.h"

#include "../trace_engine/Event_engine.h"
#include "../utils/file_compare.h"
#include "../utils/BFloat16.h"


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

		// generate data and instructions before simulation
		this->data_gen();

		// register threads
		SC_THREAD(test_cores);
	}
	void load_matrix(const std::string& file_path, uint64_t base_addr, uint64_t height, uint64_t width);
	void load_vector(const std::string& file_path, uint64_t base_addr, uint64_t height, uint64_t width);
	void print_matrix(uint64_t base_addr, uint64_t height, uint64_t width);
	void read_and_print_output_matrix(uint64_t base_addr, uint64_t height, uint64_t width);
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

const uint64_t input_addr = 0x0001; // input 16(2)*784
const uint64_t fc1w_addr 	= 0x0620; // fc1.w 784(98)*128
const uint64_t fc1b_addr 	= 0x3720; // fc1.b 16(2)*128
const uint64_t fc2w_addr 	= 0x3820; // fc2.w 128(8)*16
const uint64_t fc2b_addr 	= 0x3920; // fc2.b 16*16

const uint64_t batch_size = 16;
const uint64_t layer_one = 784;
const uint64_t layer_two = 128;
const uint64_t output_unpadded = 10;
const uint64_t output_padded = 16;

void core_tb::load_matrix(const std::string& file_path, uint64_t base_addr, uint64_t height, uint64_t width) {
	std::ifstream file(file_path);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << file_path << std::endl;
		return;
	}

	std::string line;
	std::vector<sc_bv<MEM_PORT_WIDTH>> matrix_data;
	std::vector<std::vector<BFloat16>> matrix;

	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::vector<BFloat16> row;
		float value;
		while (iss >> value) {
			row.push_back(BFloat16(value));
		}
		matrix.push_back(row);
	}

	file.close();

	uint64_t padded_height = ((height + 7) / 8) * 8;
	uint64_t padded_width = ((width + 7) / 8) * 8;

	for (uint64_t i = 0; i < padded_height; ++i) {
		for (uint64_t j = 0; j < padded_width; j += 8) {
			sc_bv<MEM_PORT_WIDTH> temp;
			for (uint64_t k = 0; k < 8; ++k) {
				if (i < height && j + k < width && i < matrix.size() && j + k < matrix[i].size()) {
					temp.range((16 * (k + 1) - 1), 16 * k) = to_bits(matrix[i][j + k]);
				}
				else {
					temp.range((16 * (k + 1) - 1), 16 * k) = to_bits(BFloat16(0));
				}
			}
			matrix_data.push_back(temp);
		}
	}

	data_list[base_addr] = matrix_data;
	std::cout << "Matrix loaded at address: " << base_addr << std::endl;
}

void core_tb::load_vector(const std::string& file_path, uint64_t base_addr, uint64_t height, uint64_t width) {
	std::ifstream file(file_path);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << file_path << std::endl;
		return;
	}

	std::string line;
	std::vector<sc_bv<MEM_PORT_WIDTH>> matrix_data;
	std::vector<std::vector<BFloat16>> matrix;

	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::vector<BFloat16> row;
		float value;
		while (iss >> value) {
			row.push_back(BFloat16(value));
		}
		matrix.push_back(row);
	}

	file.close();

	uint64_t padded_height = ((height + 7) / 8) * 8;
	uint64_t padded_width = ((width + 7) / 8) * 8;

	for (uint64_t i = 0; i < padded_height; ++i) {
		for (uint64_t j = 0; j < padded_width; j += 8) {
			sc_bv<MEM_PORT_WIDTH> temp;
			for (uint64_t k = 0; k < 8; ++k) {
				if (i < height && j + k < width && i < matrix.size() && j + k < matrix[i].size()) {
					temp.range((16 * (k + 1) - 1), 16 * k) = to_bits(matrix[0][j + k]); // only one row, repeated
				}
				else {
					temp.range((16 * (k + 1) - 1), 16 * k) = to_bits(BFloat16(0));
				}
			}
			matrix_data.push_back(temp);
		}
	}

	data_list[base_addr] = matrix_data;
	std::cout << "Matrix loaded at address: " << base_addr << std::endl;
}

void core_tb::print_matrix(uint64_t base_addr, uint64_t height, uint64_t width) {
	auto it = data_list.find(base_addr);
	if (it == data_list.end()) {
		std::cerr << "Matrix not found at address: " << base_addr << std::endl;
		return;
	}

	const auto& matrix_data = it->second;
	uint64_t padded_height = ((height + 7) / 8) * 8;
	uint64_t padded_width = ((width + 7) / 8) * 8;

	for (uint64_t i = 0; i < padded_height; ++i) {
		for (uint64_t j = 0; j < padded_width; ++j) {
			uint64_t index = i * padded_width + j;
			uint64_t data_index = index / (MEM_PORT_WIDTH / 16);
			uint64_t element_index = index % (MEM_PORT_WIDTH / 16);
			if (data_index < matrix_data.size()) {
				sc_bv<16> element_data = matrix_data[data_index].range(16 * (element_index + 1) - 1, 16 * element_index);
				BFloat16 value(element_data);
				std::cout << value << " ";
			}
			else {
				std::cout << "0 ";
			}
		}
		std::cout << std::endl;
	}
}


void core_tb::read_and_print_output_matrix(uint64_t base_addr, uint64_t height, uint64_t width) {
	std::vector<sc_bv<MEM_PORT_WIDTH>> matrix_data;
	sc_bv<MEM_PORT_WIDTH> temp;

	uint64_t padded_height = (height + 7) / 8 * 8;
	uint64_t padded_width = (width + 7) / 8 * 8;
	for (auto i = 0; i < padded_height*padded_width/8; i++) {
		this->test_core->core_memory->read(base_addr + i, temp);
		for (auto j = 0; j < MEM_PORT_WIDTH / 16; j++) {
			BFloat16 temp_bf16(temp.range((16 * (j + 1) - 1), 16 * j));
			cout << temp_bf16 << " ";
		}
		if (i % (padded_width / 8) == 0) {
			cout << endl;
		}
	}
	std::string output_file;
	{
		output_file = "E:/chip_simulator/matrices_transformer/sim_matrix_output.txt";
	}
	std::ofstream ofs(output_file);
	if (!ofs.is_open()) {
		std::cerr << "Failed to open file: " << output_file << std::endl;
		return;
	}

	for (auto i = 0; i < height * padded_width / 8; i++) {
		if (i % (padded_width / 8) == 0) {
			ofs << endl;
		}
		this->test_core->core_memory->read(base_addr + i, temp);
		for (auto j = 0; j < MEM_PORT_WIDTH / 16; j++) {
			BFloat16 temp_bf16(temp.range((16 * (j + 1) - 1), 16 * j));
			if ((i % (padded_width / 8)*8+ j) < width) ofs << temp_bf16 << " ";
		}
		
	}
}


/// @brief generate data and instructions
void core_tb::data_gen()
{
	// prepare prims
	using namespace prims;

	instructions.push_back(convertPrim2Code(
		PrimMM(input_addr, layer_one, batch_size, fc1w_addr, layer_two, fc1b_addr, fc1b_addr)
	));
	instructions.push_back(convertPrim2Code(
		PrimRelu(fc1b_addr, batch_size*layer_two/8, 0, fc1b_addr, 1)
	));
	instructions.push_back(convertPrim2Code(
		PrimMM(fc1b_addr, layer_two, batch_size, fc2w_addr, output_padded, fc2b_addr, fc2b_addr)
	));

	// prepare data
	//uniform_real_distribution<float> u(-1, 1);
	//default_random_engine e(time(NULL));

	cout << "--------------------- original data ---------------------" << endl;
	// Load matrix A from file and push to data_list
	load_matrix("./notebooks/data.txt", input_addr, layer_one, batch_size);
	load_matrix("./notebooks/fc1w.txt", fc1w_addr, layer_two, layer_one);
	load_vector("./notebooks/fc1b.txt", fc1b_addr, layer_two, batch_size);
	load_matrix("./notebooks/fc2w.txt", fc2w_addr, output_unpadded, layer_two);
	load_vector("./notebooks/fc2b.txt", fc2b_addr, output_unpadded, batch_size);

	std::cout << "input data" << std::endl;
	print_matrix(input_addr, layer_one, batch_size);
	std::cout << std::endl;
	std::cout << "fc1.weight" << std::endl;
	print_matrix(fc1w_addr, layer_two, layer_one);
	std::cout << std::endl;
	std::cout << "fc1.bias" << std::endl;
	print_matrix(fc1b_addr, layer_two, batch_size);
	std::cout << std::endl;
	std::cout << "fc2.weight" << std::endl;
	print_matrix(fc2w_addr, output_padded, layer_two);
	std::cout << std::endl;
	std::cout << "fc2.bias" << std::endl;
	print_matrix(fc2b_addr, output_padded, batch_size);
	std::cout << std::endl;
}

/// @brief do something before start signal
void core_tb::before_start()
{

}

/// @brief do something after core stop, usually print output data and test the results
void core_tb::after_stop()
{
	bool test_pass_flag = true;
	sc_bv<MEM_PORT_WIDTH> temp;

	cout << "--------------------- output data ---------------------" << endl;
	/*std::cout << "Matrix MID_OUTPUT:" << std::endl;
	read_and_print_output_matrix(mid_output_addr, left_height, right_width);*/
	std::cout << "Matrix OUTPUT:" << std::endl;
	read_and_print_output_matrix(fc2b_addr, output_padded, batch_size);
}

/// @brief test core (simulation)
void core_tb::test_cores()
{
	// reset signals
	reset.write(false);
	wait(10, SC_NS);
	reset.write(true);
	wait(10, SC_NS);
	reset.write(false);
	wait(10, SC_NS);

	// write data and instructions into core memory
	test_core->writeData(data_list);
	test_core->writePrims(0, instructions);

	before_start();

	// start core
	start.write(false);
	wait(10, SC_NS);
	start.write(true);
	wait(core_in_idle.posedge_event());  // wait until core stops by itself

	after_stop();  // do something after core stops

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

	sc_stop();  // stop simulation

}


int sc_main(int argc, char* argv[])
{
	Event_engine* event_engine_test = new Event_engine("my_event_engine");
	FunctionalNoC<sc_bv<MEM_PORT_WIDTH> >* noc = new FunctionalNoC<sc_bv<MEM_PORT_WIDTH> >("noc", event_engine_test);

	// instantiate core
	core::Core test_core("test_core", CoreLoc(0, 0), noc, event_engine_test);

	// instantiate host (testbench) and connect to core
	core_tb host("host", "single_core_test_mm", &test_core);

	sc_start();  // start simulation

	delete event_engine_test;
	delete noc;

	return 0;
}

