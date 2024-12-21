#include "core_dendrite_unit.h"
#include "../prims/prim_mm.h"
#include "../utils/BFloat16.h"

/// @brief Exe unit status transition
/// Driven signals: dendrite_status, dendrite_busy
void core::DendriteUnit::dendriteStateTrans()
{
	while (true) {
		if (this->rst.read()) {
			this->dendrite_status.write(DENDRITE_IDLE);
			this->dendrite_busy.write(false);
		}
		else if (this->dendrite_status.read() == DENDRITE_PRIM) {
			if (this->dendrite_prim_finish.read()) {
				this->dendrite_status.write(DENDRITE_IDLE);
				this->dendrite_busy.write(false);
			}
		}
		else if (this->dendrite_status.read() == DENDRITE_IDLE) {
			if (dendrite_prim_start.read()) {
				this->dendrite_status.write(DENDRITE_PRIM);
				this->dendrite_busy.write(true);
			}
		}
		wait();
	}
}

void core::DendriteUnit::dendritePrimExecution()
{
	while (true) {
		if ((this->dendrite_status.read() == DENDRITE_IDLE) || this->rst.read()) {
			this->dendrite_prim_finish.write(false);
		}
		else if (this->dendrite_status == DENDRITE_PRIM) {
			sc_bv<MEM_PORT_WIDTH> prim_temp = prim_in.read();

			if (prim_temp.range(3, 0).to_uint() == 6) {
				/************************* MatrixMultiply *************************/
				prims::PrimMM prim_mm(prim_temp);
				auto lh = prim_mm.left_height_;
				auto lw = prim_mm.left_width_;
				auto rw = prim_mm.right_width_;
				auto laddr = prim_mm.left_input_addr_;
				auto raddr = prim_mm.right_input_addr_;
				auto baddr = prim_mm.bias_addr_;
				auto oaddr = prim_mm.output_addr_;
				for (auto i = 0; i < lh >> 3; i++) {
					for (auto j = 0; j < lw; j++) {
						for (auto k = 0; k < rw; k++) {
							// processing 8*8 chunk at A[i*8, j], B[j*8, k], C[i*8, k]
							sc_bv<MEM_PORT_WIDTH> a_chunk[8], b_chunk[8], c_chunk[8];

							for (auto d = 0; d < 8; d++) {
								core_mem_port->read(laddr + ((i << 3) + d) * lw + j, a_chunk[d]);
								core_mem_port->read(raddr + ((j << 3) + d) * rw + k, b_chunk[d]);
								core_mem_port->read(baddr + ((i << 3) + d) * rw + k, c_chunk[d]);
							}
							// 8*8 matmul
							for (auto d = 0; d < 8; d++) {
								for (auto e = 0; e < 8; e++) {
									BFloat16 sum = BFloat16(0);
									for (auto f = 0; f < 8; f++) {
										BFloat16 a(a_chunk[d].range(16 * (f + 1) - 1, 16 * f));
										BFloat16 b(b_chunk[f].range(16 * (e + 1) - 1, 16 * e));
										sum = sum + a * b;
									}
									BFloat16 c(c_chunk[d].range(16 * (e + 1) - 1, 16 * e));
									c = c + sum;
									c_chunk[d].range(16 * (e + 1) - 1, 16 * e) = to_bits(c);
								}
							}
							for (auto d = 0; d < 8; d++) {
								core_mem_port->write(oaddr + ((i << 3) + d) * rw + k, c_chunk[d], 1);
							}
						}
					}
				}
			}
			else if (prim_temp.range(3, 0).to_uint() == 7) {
				/************************* Conv *************************/
			}
			else {
				assert(0);
			}
			this->dendrite_prim_finish.write(true);
		}
		wait();
	}
}
