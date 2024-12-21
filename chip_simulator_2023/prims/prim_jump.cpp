#include "prim_jump.h"

prims::PrimJump::PrimJump(int64_t instruction_addr, uint64_t relative_mode)
	: instruction_addr_(instruction_addr), relative_mode_(relative_mode)
{
	if (!relative_mode) assert(instruction_addr < (1 << 16) && instruction_addr >= 0);
	else assert(instruction_addr < (1 << 15) - 1 && instruction_addr >= -(1 << 15));
	assert(relative_mode < (1 << 1));

	this->prim_name_ = "Jump";

	this->convertParams2PrimIDCode();
}

prims::PrimJump::PrimJump(sc_bv<MEM_PORT_WIDTH> prim_id_code)
	:Primitive(prim_id_code)
{
	assert(prim_id_code.range(3, 0) == sc_bv<4>(1));

	this->relative_mode_ = prim_id_code.range(116, 116).to_uint64();
	if (!relative_mode_) this->instruction_addr_ = prim_id_code.range(67, 52).to_uint();
	else this->instruction_addr_ = prim_id_code.range(67, 52).to_int64();

	this->prim_name_ = "Jump";
}

void prims::PrimJump::convertParams2PrimIDCode()
{
	this->prim_id_code_.range(3, 0) = sc_bv<4>(1);

	this->prim_id_code_.range(67, 52) = sc_bv<16>(this->instruction_addr_);
	this->prim_id_code_.range(116, 116) = sc_bv<1>(this->relative_mode_);
}
