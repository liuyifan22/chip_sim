#include "prim_relu.h"

prims::PrimRelu::PrimRelu(uint64_t input_addr, uint64_t neuron_num_in_32B, uint64_t threshold, uint64_t output_addr, uint64_t float_mode)
	: input_addr_(input_addr), neuron_num_in_32B_(neuron_num_in_32B), threshold_(threshold), output_addr_(output_addr), float_mode_(float_mode)
{
	assert(input_addr < (1 << 16));
	assert(neuron_num_in_32B < (1 << 16));
	assert(threshold < (1 << 16));
	assert(output_addr < (1 << 16));
	assert(float_mode < (1 << 1));

	this->prim_name_ = "ReLU";

	this->convertParams2PrimIDCode();
}

prims::PrimRelu::PrimRelu(sc_bv<MEM_PORT_WIDTH> prim_id_code)
	: Primitive(prim_id_code)
{
	assert(prim_id_code.range(3, 0) == sc_bv<4>(4));

	this->input_addr_ = prim_id_code.range(19, 4).to_uint();
	this->neuron_num_in_32B_ = prim_id_code.range(35, 20).to_uint();
	this->threshold_ = prim_id_code.range(67, 52).to_uint();
	this->output_addr_ = prim_id_code.range(115, 100).to_uint();
	this->float_mode_ = prim_id_code.range(116, 116).to_uint();

	this->prim_name_ = "ReLU";
}

void prims::PrimRelu::convertParams2PrimIDCode()
{
	this->prim_id_code_.range(3, 0) = sc_bv<4>(4);

	this->prim_id_code_.range(19, 4) = sc_bv<16>(this->input_addr_);
	this->prim_id_code_.range(35, 20) = sc_bv<16>(this->neuron_num_in_32B_);
	this->prim_id_code_.range(67, 52) = sc_bv<16>(this->threshold_);
	this->prim_id_code_.range(115, 100) = sc_bv<16>(this->output_addr_);
	this->prim_id_code_.range(116, 116) = sc_bv<1>(this->float_mode_);
}
