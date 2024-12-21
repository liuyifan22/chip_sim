#include "core_soma_unit.h"
#include "../prims/prim_relu.h"
#include "../prims/prim_softmax.h"
#include "../utils/BFloat16.h"

void core::SomaUnit::somaStateTrans()
{
	while (true) {
		if (this->rst.read()) {
			this->soma_status.write(SOMA_IDLE);
			this->soma_busy.write(false);
		}
		else if (this->soma_status.read() == SOMA_PRIM) {
			if (this->soma_prim_finish.read()) {
				this->soma_status.write(SOMA_IDLE);
				this->soma_busy.write(false);
			}
		}
		else if (this->soma_status.read() == SOMA_IDLE) {
			if (soma_prim_start.read()) {
				this->soma_status.write(SOMA_PRIM);
				this->soma_busy.write(true);
			}
		}
		wait();
	}
}///

void core::SomaUnit::somaPrimExecution()
{
	while (true) {
		if ((this->soma_status.read() == SOMA_IDLE) || this->rst.read()) {
			this->soma_prim_finish.write(false);
		}
		else if (this->soma_status == SOMA_PRIM) {
			sc_bv<MEM_PORT_WIDTH> prim_temp = this->prim_in.read();
			sc_bv<4> prim_op_code = this->prim_in.read().range(3, 0);
			if (prim_op_code == sc_bv<4>(0x4)) {
				/************************* ReLU *************************/
				prims::PrimRelu prim_relu(prim_temp);  // decode relu prim
				for (auto i = 0; i < prim_relu.neuron_num_in_32B_; i++) {
					sc_bv<MEM_PORT_WIDTH> mem_temp, result_temp;
					core_mem_port->read(prim_relu.input_addr_ + i, mem_temp);  // read data from memory
					if (prim_relu.float_mode_ == 1) {
						BFloat16 thres(sc_bv<16>(prim_relu.threshold_));
						for (auto j = 0; j < MEM_PORT_WIDTH / 16; j++) {
							BFloat16 temp_bfloat16(mem_temp.range(16 * (j + 1) - 1, 16 * j));
							temp_bfloat16 = (temp_bfloat16 > thres) ? temp_bfloat16 : BFloat16(0);
							result_temp.range(16 * (j + 1) - 1, 16 * j) = to_bits(temp_bfloat16);
						}
					}
					else {
						cout << "not implemented yet" << endl;
						assert(0);
					}
					core_mem_port->write(prim_relu.output_addr_ + i, result_temp, 1);  // write data to memory
				}
			}
			else if (prim_op_code == sc_bv<4>(8)) {
				/************************* Softmax *************************/
				cout << "HAHA, using Softmax NOW!" << endl;
				prims::PrimSoftmax prim_softmax(prim_temp);  // decode softmax prim
				auto input_addr = prim_softmax.input_addr_;
				auto single_length = prim_softmax.single_length_;
				auto batch_size = prim_softmax.batch_size_;
				auto output_addr = prim_softmax.output_addr_;
				auto batch_room = batch_size / 8;
				assert (batch_size % 8 == 0);
				std::vector<std::vector<BFloat16>> softmax_array(single_length, std::vector<BFloat16>(batch_size));
				// read data from memory
				// get an array of BFloat16 with shape (single_length, batch_size)
				for (auto i = 0; i < single_length; i++) {
					for (auto j = 0; j < batch_room; j++) {
						sc_bv<MEM_PORT_WIDTH> mem_temp;
						core_mem_port->read(input_addr + i * batch_room + j, mem_temp);
						for (auto k = 0; k < MEM_PORT_WIDTH / 16; k++) {
							softmax_array[i][j * MEM_PORT_WIDTH / 16 + k] = BFloat16(mem_temp.range(16 * (k + 1) - 1, 16 * k));
						}
					}
				}
				// softmax operation
				// get a column. For each column, do softmax operation
				for (auto i = 0; i < batch_size; i++) {
					std::vector<BFloat16> column(single_length);
					for (auto j = 0; j < single_length; j++) {
						column[j] = softmax_array[j][i] / 4.0; // channel-wise scaling
					}
					// do softmax operation
					BFloat16 sum = BFloat16(0);
					for (auto j = 0; j < single_length; j++) {
						sum = sum + exp(column[j]);
					}
					for (auto j = 0; j < single_length; j++) {
						softmax_array[j][i] = exp(column[j]) / sum;
					}
				}
				// write data to memory
				// write an array of BFloat16 with shape (single_length, batch_size)
				for (auto i = 0; i < single_length; i++) {
					for (auto j = 0; j < batch_room; j++) {
						sc_bv<MEM_PORT_WIDTH> result_temp;
						for (auto k = 0; k < MEM_PORT_WIDTH / 16; k++) {
							result_temp.range(16 * (k + 1) - 1, 16 * k) = to_bits(softmax_array[i][j * MEM_PORT_WIDTH / 16 + k]);
						}
						core_mem_port->write(output_addr + i * batch_room + j, result_temp, 1);
					}
				}
			}
			else {
				assert(0);
			}
			this->soma_prim_finish.write(true);
		}
		wait();
	}
}


