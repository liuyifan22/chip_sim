#pragma once
#include "../config.h"
#include "../components/mem_if.h"
#include "../trace_engine/Event_engine.h"

namespace core
{
	enum core_control_status
	{
		CORE_IDLE = 0, PRIM_PROCESS, EXECUTION
	};


	class ControlUnit : sc_module
	{
	public:
		SC_HAS_PROCESS(ControlUnit);
		ControlUnit(const sc_module_name& nm, Event_engine* _e_engine)
			: sc_module(nm)
		{
			e_engine_ = _e_engine;

			SC_THREAD(ctrlStatusTrans);
			sensitive << reset.pos();
			sensitive << start.pos();
			sensitive << execution_finish.posedge_event();
			sensitive << process_finish.posedge_event();
			dont_initialize();

			SC_THREAD(run);
			sensitive << ctrl_status << reset.pos();
			dont_initialize();

		}

		~ControlUnit();
		void ctrlStatusTrans();
		void run();

	public:
		Event_engine* e_engine_;


	public:
		sc_port<ram_if<sc_bv<MEM_PORT_WIDTH> > > core_memory_port;
		sc_in<bool> reset;
		sc_in<bool> start;
		sc_in<bool> stop;
		sc_out<bool> in_idle;

		sc_out<sc_bv<MEM_PORT_WIDTH> > prim_to_move;
		sc_out<sc_bv<MEM_PORT_WIDTH> > prim_to_dendrite;
		sc_out<sc_bv<MEM_PORT_WIDTH> > prim_to_soma;
		sc_out<sc_bv<MEM_PORT_WIDTH> > prim_to_router;

		sc_out<bool> move_prim_start;
		sc_out<bool> soma_prim_start;
		sc_out<bool> router_prim_start;
		sc_out<bool> dendrite_prim_start;

		sc_in<bool> move_busy;
		sc_in<bool> dendrite_busy;
		sc_in<bool> soma_busy;
		sc_in<bool> router_busy;


	public:
		sc_signal<bool> next_prim;
		sc_signal<sc_bv<4> > prim_op_code;
		sc_signal<sc_bv<MEM_PORT_WIDTH> > prim_code;

		sc_signal<uint64_t> pc;

		sc_signal<bool> to_idle;
		sc_signal<bool> process_finish;
		
		sc_signal<bool> execution_finish;

		sc_buffer<core_control_status> ctrl_status;

	};
}