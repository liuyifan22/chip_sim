#pragma once
#include "config.h"
#include "systemc.h"
#include "mem_if.h"
#include "Event_engine.h"


template<class T>
class rom : public sc_channel, public rom_if<T>
{
private:
	T* mem;
	uint64_t m_start_address;
	uint64_t m_end_address;
public:
	Event_engine* e_engine_;
public:
	rom(sc_module_name name_, uint64_t start_address, uint64_t end_address, T*& data, Event_engine* _e_engine)
		:m_start_address(start_address)
		, m_end_address(end_address) {
		//sc_assert(m_end_address >= m_start_addresss);
		e_engine_ = _e_engine;
		mem = new T[m_end_address - m_start_address];
		
		for (uint64_t i = start_address; i < end_address; i++)
		{
			mem[i - start_address] = data[i - start_address];
		}
	}
	~rom() {
		if (mem) { delete[] mem; }
	}

	virtual void register_port(sc_port_base& port, const char* if_typename) {

	}

	transfer_status read(uint64_t address, T& data)
	{
		if (address<m_start_address || address>m_end_address) {
			data = 0;
			return TRANSFER_ERROR;
		}
		data = mem[address - m_start_address];
		Trace_event_util _util;
		_util.m_bar_name = __func__;
		e_engine_->add_event(this->name(), __func__, "B", _util);
		wait(RAM_READ_LATENCY, SC_NS);
		e_engine_->add_event(this->name(), __func__, "E", _util);

		return TRANSFER_OK;
	}

	bool reset() {
		for (uint64_t i = 0; i < (m_end_address - m_start_address); i++)
			mem[i] = (T)0;
		return true;
	}

	inline uint64_t start_address()const {
		return m_start_address;
	}

	inline uint64_t end_address()const {
		return m_end_address;
	}

};