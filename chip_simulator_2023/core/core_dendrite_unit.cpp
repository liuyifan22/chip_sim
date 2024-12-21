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

                // 计算补零后的矩阵尺寸，确保是8的倍数
                auto padded_lh = ((lh + 7) / 8) * 8;
                auto padded_lw = ((lw + 7) / 8) * 8;
                auto padded_rw = ((rw + 7) / 8) * 8;

                // 遍历输出矩阵块
                // 遍历输出矩阵的行和列块
                for (auto i = 0; i < padded_lh; i += 8) {
                    for (auto j = 0; j < padded_rw; j += 8) {
                        // 初始化 c_chunk
                        BFloat16 c_chunk[8][8] = { BFloat16(0) };
                        // 遍历中间维度
                        for (auto k = 0; k < padded_lw; k += 8) {
                            // 定义块矩阵
                            BFloat16 a_chunk[8][8] = { BFloat16(0) };
                            BFloat16 b_chunk[8][8] = { BFloat16(0) };
							sc_bv<128> data; // 8个BFloat16

                            // 读取 A 矩阵块
                            for (auto d = 0; d < 8; d++) { // 竖着的
                                uint64_t row = i + d;
                                if (row < lh) {
                                    uint64_t addr = laddr + row * padded_lw/8 + k/8;
                                    core_mem_port->read(addr, data);
                                    // 解析8个BF16
                                    for (auto e = 0; e < 8; e++) {
                                        uint64_t col = k + e;
                                        if (col < lw) {
                                            sc_bv<16> element_data = data.range(16 * (e + 1) - 1, 16 * e);
                                            a_chunk[d][e] = BFloat16(element_data);
                                        }
                                        else {
                                            a_chunk[d][e] = BFloat16(0);
                                        }
                                    }
                                }
                                else {
                                    for (auto e = 0; e < 8; e++) {
                                        a_chunk[d][e] = BFloat16(0);
                                    }
                                }
                            }

                            // 读取 B 矩阵块
                            for (auto d = 0; d < 8; d++) {
                                uint64_t row = k + d;
                                if (row < lw) {
                                    uint64_t addr = raddr + row * padded_rw/8 + j/8;
                                    core_mem_port->read(addr, data);
                                    for (auto e = 0; e < 8; e++) {
                                        uint64_t col = j + e;
                                        if (col < rw) {
                                            sc_bv<16> element_data = data.range(16 * (e + 1) - 1, 16 * e);
                                            b_chunk[d][e] = BFloat16(element_data);
                                        }
                                        else {
                                            b_chunk[d][e] = BFloat16(0);
                                        }
                                    }
                                }
                                else {
                                    for (auto e = 0; e < 8; e++) {
                                        b_chunk[d][e] = BFloat16(0);
                                    }
                                }
                            }

                            // 读取C矩阵块（偏置），并进行补零，与k无关
                            for (auto d = 0; d < 8; d++) {
                                uint64_t row = i + d;
                                if (row < lh) {
                                    uint64_t addr = baddr + row * padded_rw/8 + j/8;
                                    core_mem_port->read(addr, data);
                                    for (auto e = 0; e < 8; e++) {
                                        uint64_t col = j + e;
                                        if (col < rw) {
                                            sc_bv<16> element_data = data.range(16 * (e + 1) - 1, 16 * e);
                                            c_chunk[d][e] = BFloat16(element_data);
                                        }
                                        else {
                                            c_chunk[d][e] = BFloat16(0);
                                        }
                                    }
                                }
                                else {
                                    // 超出矩阵范围，整行补零
                                    for (auto e = 0; e < 8; e++) {
                                        c_chunk[d][e] = BFloat16(0);
                                    }
                                }
                            }

                            // 进行块矩阵乘法
                            for (auto d = 0; d < 8; d++) {
                                for (auto e = 0; e < 8; e++) {
                                    BFloat16 sum = BFloat16(0);
                                    for (auto f = 0; f < 8; f++) {
                                        sum = sum + a_chunk[d][f] * b_chunk[f][e];
                                    }// 新设计：直接累加在c上，k循环的求和不必显式写出
                                    c_chunk[d][e] = c_chunk[d][e] + sum;
                                }
                            }
                        }

                        // 将结果写回内存
                        for (auto d = 0; d < 8; d++) {
                            uint64_t row = i + d;
                            if (row < lh) {
                                sc_bv<128> write_data;
                                for (auto e = 0; e < 8; e++) {
                                    write_data.range(16 * (e + 1) - 1, 16 * e) = to_bits(c_chunk[d][e]);
                                }
                                uint64_t addr = oaddr + row * padded_rw/8 + j/8;
                                core_mem_port->write(addr, write_data, 1);
                            }
                        }
                    }
                }
            }
            else if (prim_temp.range(3, 0).to_uint() == 7) {
                /************************* Conv *************************/
                // 卷积操作的实现
            }
            else {
                assert(0);
            }
            this->dendrite_prim_finish.write(true);
        }
        wait();
    }
}
