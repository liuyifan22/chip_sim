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
	class PrimRelu :
		public Primitive
	{
	public:
		PrimRelu(
			uint64_t input_addr,
			uint64_t neuron_num_in_32B,
			uint64_t threshold,
			uint64_t output_addr,
			uint64_t float_mode
		);
		PrimRelu(sc_bv<MEM_PORT_WIDTH> prim_id_code);

		void convertParams2PrimIDCode();

	public:
		// prims params

		uint64_t input_addr_;
		uint64_t neuron_num_in_32B_;
		uint64_t threshold_;
		uint64_t output_addr_;
		uint64_t float_mode_;
	};
}

