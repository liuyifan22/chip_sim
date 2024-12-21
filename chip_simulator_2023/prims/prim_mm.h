#pragma once

#include <map>
#include <vector>
#include <string>

#include "systemc.h"

#include "../config.h"
#include "primitive.h"


namespace prims
{
	/// @brief The Matrix Multiply primitive.
	class PrimMM :
		public Primitive
	{
	public:
		PrimMM(
			uint64_t left_input_addr,
			uint64_t left_height,
			uint64_t left_width,
			uint64_t right_input_addr,
			uint64_t right_width,
			uint64_t bias_addr,
			uint64_t output_addr
		);
		PrimMM(sc_bv<MEM_PORT_WIDTH> prim_id_code);

		void convertParams2PrimIDCode();

	public:
		// prims params

		uint64_t left_input_addr_;
		uint64_t left_height_;
		uint64_t left_width_;
		uint64_t right_input_addr_;
		uint64_t right_width_;
		uint64_t bias_addr_;
		uint64_t output_addr_;
	};
}