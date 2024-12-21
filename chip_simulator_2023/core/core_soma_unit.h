#pragma once
#include "systemc.h"
#include "../components/ram.h"
#include "../components/dram.h"
#include "../prims/primitive.h"
#include "../components/functional_noc.h"

namespace core {
	enum soma_unit_status
	{
		SOMA_IDLE = 0, SOMA_PRIM
	};



	class SomaUnit : public sc_module
	{
	public:
		SC_HAS_PROCESS(SomaUnit);
		SomaUnit(const sc_module_name& name, Event_engine* _e_engine) :
			sc_module(name), e_engine_(_e_engine)
		{
			SC_THREAD(somaStateTrans);
			sensitive << rst.pos() << soma_prim_finish.posedge_event() << soma_prim_start.pos();
			dont_initialize();

			SC_THREAD(somaPrimExecution);
			sensitive << soma_status << rst.pos();
			dont_initialize();
		}

		void somaStateTrans();
		void somaPrimExecution();

	public:
		sc_in<sc_bv<MEM_PORT_WIDTH> > prim_in;
		sc_in<bool> rst;
		sc_in<bool> soma_prim_start;
		sc_out<bool> soma_busy;

		sc_port<ram_if<sc_bv<MEM_PORT_WIDTH> > > core_mem_port;

		sc_signal<bool> soma_prim_finish;
		sc_buffer<soma_unit_status> soma_status;

		Event_engine* e_engine_;

	};
}
