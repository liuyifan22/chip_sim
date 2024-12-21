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
	class PrimJump :
		public Primitive
	{
	public:
		PrimJump(
			int64_t instruction_addr,
			uint64_t relative_mode
		);
		PrimJump(sc_bv<MEM_PORT_WIDTH> prim_id_code);

		void convertParams2PrimIDCode();

	public:
		// prims params
		int64_t instruction_addr_;
		uint64_t relative_mode_;
	};
}

