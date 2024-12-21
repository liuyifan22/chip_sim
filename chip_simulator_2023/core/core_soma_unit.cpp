#include "core_soma_unit.h"
#include "../prims/prim_relu.h"
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
			else if (prim_op_code == sc_bv<4>(0x5)) {
				/************************* LIF *************************/
			}
			else {
				assert(0);
			}
			this->soma_prim_finish.write(true);
		}
		wait();
	}
}


