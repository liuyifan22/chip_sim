#pragma once

#include <map>
#include <vector>
#include <string>

#include "systemc.h"

#include "../config.h"

namespace prims
{
	/// @brief Base class of all primitives.
	class Primitive
	{
	public:
		Primitive();
		Primitive(sc_bv<MEM_PORT_WIDTH> prim_id_code);
		~Primitive();
		Primitive& operator=(const Primitive& rhs);
		bool operator==(const Primitive& rhs) const;
		friend ostream& operator<<(ostream& file, const Primitive& prim);
		friend void sc_trace(sc_trace_file*& tf, const Primitive& prim, std::string nm);
		friend sc_bv<MEM_PORT_WIDTH> convertPrim2Code(const Primitive& p);

		// prim generation
		virtual void convertParams2PrimIDCode() = 0;

	public:
		std::string prim_name_;

	protected:
		sc_bv<MEM_PORT_WIDTH> prim_id_code_;
	};

	sc_bv<MEM_PORT_WIDTH> convertPrim2Code(const Primitive& p);
}


