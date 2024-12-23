#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <random>

#include "../core/core.h"
#include "../prims/primitive.h"
#include "../prims/prim_load.h"
#include "../prims/prim_store.h"
#include "../prims/prim_jump.h"
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

bool MEDIUM = false;
uint64_t left_input_addr=0x10;
uint64_t left_height=MEDIUM ? 16 : 3;
uint64_t left_width= MEDIUM ? 8 : 4;
uint64_t right_width = MEDIUM ? 24 : 5;
uint64_t right_input_addr=0x1001;
uint64_t bias_addr=0x2001;
uint64_t output_addr=0x3001;

//void core_tb::load_matrix(const std::string& file_path, uint64_t base_addr) {
//	std::ifstream file(file_path);
//	if (!file.is_open()) {
//		std::cerr << "Failed to open file: " << file_path << std::endl;
//		return;
//	}
//
//	std::string line;
//	std::vector<sc_bv<MEM_PORT_WIDTH>> matrix_data;
//	while (std::getline(file, line)) {
//		std::istringstream iss(line);
//		std::vector<BFloat16> row;
//		float value;
//		while (iss >> value) {
//			row.push_back(BFloat16(value));
//		}
//
//		
//		for (size_t i = 0; i < row.size()/8; ++i) {
//			sc_bv<MEM_PORT_WIDTH> temp;
//			for (size_t j = 0; j < 8; ++j) {
//				temp.range((16 * (j + 1) - 1), 16 * j) = to_bits(row[i*8+j]);
//			}
//			matrix_data.push_back(temp);
//		}
//	}
//
//	file.close();
//
//	// Push matrix data to data_list
//	data_list[base_addr] = matrix_data;
//	std::cout << "Matrix loaded at address: " << base_addr << std::endl;
//}

void core_tb::load_matrix(const std::string& file_path, uint64_t base_addr, uint64_t height, uint64_t width) {
	std::ifstream file(file_path);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << file_path << std::endl;
		return;
	}

	std::string line;
	std::vector<sc_bv<MEM_PORT_WIDTH>> matrix_data;
	std::vector<std::vector<BFloat16>> matrix;

	// 读取文件并解析为矩阵
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

	// 计算补零后的矩阵尺寸
	uint64_t padded_height = ((height + 7) / 8) * 8;
	uint64_t padded_width = ((width + 7) / 8) * 8;

	// 补零并转换为 sc_bv<MEM_PORT_WIDTH> 格式
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

	// 将矩阵数据推送到 data_list
	data_list[base_addr] = matrix_data;
	std::cout << "Matrix loaded at address: " << base_addr << std::endl;
}



//void core_tb::print_matrix(uint64_t base_addr, uint64_t height, uint64_t width) {
//	auto it = data_list.find(base_addr);
//	if (it == data_list.end()) {
//		std::cerr << "Matrix not found at address: " << base_addr << std::endl;
//		return;
//	}
//
//	const auto& matrix_data = it->second;
//	for (uint64_t i = 0; i < height; ++i) {
//		for (uint64_t j = 0; j < width; ++j) {
//			uint64_t index = i * width + j;
//			uint64_t data_index = index / (MEM_PORT_WIDTH / 16);
//			uint64_t element_index = index % (MEM_PORT_WIDTH / 16);
//			if (data_index < matrix_data.size()) {
//				sc_bv<16> element_data = matrix_data[data_index].range(16 * (element_index + 1) - 1, 16 * element_index);
//				BFloat16 value(element_data);
//				std::cout << value << " ";
//			}
//			else {
//				std::cout << "0 ";
//			}
//		}
//		std::cout << std::endl;
//	}
//}

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

	// 读取输出矩阵数据
	for (uint64_t i = 0; i < height; ++i) {
		for (uint64_t j = 0; j < width; j += MEM_PORT_WIDTH / 16) {
			this->test_core->core_memory->read(base_addr + (i * width + j) * 2, temp);
			matrix_data.push_back(temp);
		}
	}

	// 打印输出矩阵
	/*for (uint64_t i = 0; i < height; ++i) {
		for (uint64_t j = 0; j < width; ++j) {
			uint64_t index = i * width + j;
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
	}*/
	uint64_t padded_height = (height + 7) / 8 * 8;
	uint64_t padded_width = (width + 7) / 8 * 8;
	for (auto i = 0; i < padded_height*padded_width/8; i++) {
		this->test_core->core_memory->read(output_addr + i, temp);
		for (auto j = 0; j < MEM_PORT_WIDTH / 16; j++) {
			BFloat16 temp_bf16(temp.range((16 * (j + 1) - 1), 16 * j));
			cout << temp_bf16 << " ";
		}
		if (i % (padded_width / 8) == 0) {
			cout << endl;
		}
	}
	std::string output_file;
	if (MEDIUM) {
		output_file = "E:/chip_simulator/matrices_medium/sim_matrix_output.txt";
	}
	else {
		output_file = "E:/chip_simulator/matrices_small/sim_matrix_output.txt";
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
		this->test_core->core_memory->read(output_addr + i, temp);
		for (auto j = 0; j < MEM_PORT_WIDTH / 16; j++) {
			BFloat16 temp_bf16(temp.range((16 * (j + 1) - 1), 16 * j));
			if (j < width) ofs << temp_bf16 << " ";
		}
		
	}
}


/// @brief generate data and instructions
void core_tb::data_gen()
{
	// prepare prims
	assert(left_input_addr + left_height*left_width < right_input_addr);
	assert(right_input_addr + left_width*right_width < bias_addr);
	assert(bias_addr + left_height * right_width < output_addr);
	using namespace prims;
	instructions.push_back(convertPrim2Code(
		PrimMM(left_input_addr,left_height,left_width,right_input_addr,right_width,bias_addr,output_addr)
	));

	// prepare data
	//uniform_real_distribution<float> u(-1, 1);
	//default_random_engine e(time(NULL));

	cout << "--------------------- original data ---------------------" << endl;
	// Load matrix A from file and push to data_list
	if (MEDIUM) {
		load_matrix("E:/chip_simulator/matrices_medium/matrix_A.txt", left_input_addr,left_height,left_width);
		load_matrix("E:/chip_simulator/matrices_medium/matrix_B.txt", right_input_addr,left_width,right_width);
		load_matrix("E:/chip_simulator/matrices_medium/matrix_C.txt", bias_addr,left_height,right_width);
	}
	else{
		load_matrix("E:/chip_simulator/matrices_small/matrix_A.txt", left_input_addr, left_height, left_width);
		load_matrix("E:/chip_simulator/matrices_small/matrix_B.txt", right_input_addr, left_width, right_width);
		load_matrix("E:/chip_simulator/matrices_small/matrix_C.txt", bias_addr, left_height, right_width);
	}
	//// Print matrix A
	std::cout << "Matrix A:" << std::endl;
	print_matrix(left_input_addr, left_height, left_width);
	std::cout << std::endl;
	std::cout << "Matrix B:" << std::endl;
	print_matrix(right_input_addr, left_width, right_width);
	std::cout << std::endl;
	std::cout << "Matrix C:" << std::endl;
	print_matrix(bias_addr, left_height, right_width);
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
	std::cout << "Matrix OUTPUT:" << std::endl;
	read_and_print_output_matrix(output_addr, left_height, right_width);
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

