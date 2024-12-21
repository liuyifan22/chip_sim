#pragma once

#include <map>
#include <vector>
#include <string>

#include "systemc.h"

#include "../config.h"
#include "primitive.h"

namespace prims
{
	/// @brief The Load primitive.
	class PrimStore :
		public Primitive
	{
	public:
		PrimStore(
			uint64_t sram_addr,
			uint64_t dram_addr,
			uint64_t data_length
		);
		PrimStore(sc_bv<MEM_PORT_WIDTH> prim_id_code);

		void convertParams2PrimIDCode();

	public:
		// prims params
		uint64_t sram_addr_;
		uint64_t dram_addr_;
		uint64_t data_length_;
	};
}
