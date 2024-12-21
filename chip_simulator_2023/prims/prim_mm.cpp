#include "prim_mm.h"

prims::PrimMM::PrimMM(uint64_t left_input_addr,
	uint64_t left_height,
	uint64_t left_width,
	uint64_t right_input_addr,
	uint64_t right_width,
	uint64_t bias_addr,
	uint64_t output_addr)
	: left_input_addr_(left_input_addr),
	left_height_(left_height),
	left_width_(left_width),
	right_input_addr_(right_input_addr),
	right_width_(right_width),
	bias_addr_(bias_addr),
	output_addr_(output_addr)
{
	assert(left_input_addr < (1 << 16));
	assert(left_height < (1 << 16));
	assert(left_width < (1 << 16));
	assert(right_input_addr < (1 << 16));
	assert(right_width < (1 << 16));
	assert(bias_addr < (1 << 16));
	assert(output_addr < (1 << 16));

	this->prim_name_ = "MM";

	this->convertParams2PrimIDCode();
}

prims::PrimMM::PrimMM(sc_bv<MEM_PORT_WIDTH> prim_id_code)
	: Primitive(prim_id_code)
{
	assert(prim_id_code.range(3, 0) == sc_bv<4>(6));

	this->left_input_addr_ = prim_id_code.range(19, 4).to_uint();
	this->left_height_ = prim_id_code.range(35, 20).to_uint();
	this->left_width_ = prim_id_code.range(51, 36).to_uint();
	this->right_input_addr_ = prim_id_code.range(67, 52).to_uint();
	this->right_width_ = prim_id_code.range(83, 68).to_uint();
	this->bias_addr_ = prim_id_code.range(99, 84).to_uint();
	this->output_addr_ = prim_id_code.range(115, 100).to_uint();

	this->prim_name_ = "MM";
}

void prims::PrimMM::convertParams2PrimIDCode()
{
	this->prim_id_code_.range(3, 0) = sc_bv<4>(6);

	this->prim_id_code_.range(19, 4) = sc_bv<16>(this->left_input_addr_);
	this->prim_id_code_.range(35, 20) = sc_bv<16>(this->left_height_);
	this->prim_id_code_.range(51, 36) = sc_bv<16>(this->left_width_);
	this->prim_id_code_.range(67, 52) = sc_bv<16>(this->right_input_addr_);
	this->prim_id_code_.range(83, 68) = sc_bv<16>(this->right_width_);
	this->prim_id_code_.range(99, 84) = sc_bv<16>(this->bias_addr_);
	this->prim_id_code_.range(115, 100) = sc_bv<16>(this->output_addr_);
}
