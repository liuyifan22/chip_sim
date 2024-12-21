#include "primitive.h"

prims::Primitive::Primitive()
{
	prim_name_ = "Null";
	this->prim_id_code_ = 0;
}

prims::Primitive::Primitive(sc_bv<MEM_PORT_WIDTH> prim_id_code)
{
	this->prim_id_code_ = prim_id_code;
}


prims::Primitive::~Primitive()
{

}


prims::Primitive& prims::Primitive::operator=(const prims::Primitive& rhs)
{
	this->prim_name_ = rhs.prim_name_;
	this->prim_id_code_ = rhs.prim_id_code_;
	return *this;
}

bool prims::Primitive::operator==(const Primitive& rhs) const
{
	return this->prim_id_code_ == rhs.prim_id_code_;
}

ostream& prims::operator<<(ostream& file, const prims::Primitive& prim)
{
	file << "Prim code: " << prim.prim_id_code_ << std::endl;
	return file;
}

void prims::sc_trace(sc_trace_file*& tf, const prims::Primitive& prim, std::string nm)
{
	sc_trace(tf, prim.prim_id_code_, nm + ".id_code");
}

sc_bv<MEM_PORT_WIDTH> prims::convertPrim2Code(const prims::Primitive& p)
{
	return p.prim_id_code_;
}
