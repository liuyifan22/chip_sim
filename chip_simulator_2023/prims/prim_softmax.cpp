#include "prim_softmax.h"

prims::PrimSoftmax::PrimSoftmax(
	uint64_t input_addr,
	uint64_t single_length,
	uint64_t batch_size,
	uint64_t output_addr
) : input_addr_(input_addr), single_length_(single_length), batch_size_(batch_size), output_addr_(output_addr)
{
	assert(input_addr < (1 << 16));
	assert(single_length < (1 << 16));
	assert(batch_size < (1 << 16));
	assert(output_addr < (1 << 16));

	this->prim_name_ = "Softmax";

	this->convertParams2PrimIDCode();
}

prims::PrimSoftmax::PrimSoftmax(sc_bv<MEM_PORT_WIDTH> prim_id_code)
	: Primitive(prim_id_code)
{
	assert(prim_id_code.range(3, 0) == sc_bv<4>(8));

	this->input_addr_ = prim_id_code.range(19, 4).to_uint();
	this->single_length_ = prim_id_code.range(35, 20).to_uint();
	this->batch_size_ = prim_id_code.range(51, 36).to_uint();
	this->output_addr_ = prim_id_code.range(115, 100).to_uint();

	this->prim_name_ = "Softmax";
}

void prims::PrimSoftmax::convertParams2PrimIDCode()
{
	this->prim_id_code_.range(3, 0) = sc_bv<4>(8);

	this->prim_id_code_.range(19, 4) = sc_bv<16>(this->input_addr_);
	this->prim_id_code_.range(35, 20) = sc_bv<16>(this->single_length_);
	this->prim_id_code_.range(51, 36) = sc_bv<16>(this->batch_size_);
	this->prim_id_code_.range(115, 100) = sc_bv<16>(this->output_addr_);
}
