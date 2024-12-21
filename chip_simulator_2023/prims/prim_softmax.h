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
	class PrimSoftmax :
		public Primitive
	{
	public:
		PrimSoftmax(
			uint64_t input_addr,
			uint64_t single_length,
			uint64_t batch_size,
			uint64_t output_addr
		);
		PrimSoftmax(sc_bv<MEM_PORT_WIDTH> prim_id_code);

		void convertParams2PrimIDCode();

	public:
		// prims params

		uint64_t input_addr_;
		uint64_t single_length_;
		uint64_t batch_size_;
		uint64_t output_addr_;
	};
}

