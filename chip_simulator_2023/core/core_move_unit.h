#pragma once
#include "../components/ram.h"
#include "../components/dram.h"
#include "../prims/primitive.h"
#include "../prims/prim_load.h"
#include "../trace_engine/Event_engine.h"

namespace core {
	enum move_unit_status { MOVE_IDLE = 0, MOVE_PRIM};

	/// @brief Move unit in a core
	class MoveUnit : public sc_module
	{
	public:
		SC_HAS_PROCESS(MoveUnit);
		MoveUnit(sc_module_name nm, Event_engine* _e_engine)
			: sc_module(nm), e_engine_(_e_engine)
		{
			SC_THREAD(moveStateTrans);
			sensitive << rst.pos() << move_prim_finish.posedge_event() << move_prim_start.pos();
			dont_initialize();

			SC_THREAD(movePrimExecution);
			sensitive << move_status << rst.pos();
			dont_initialize();
		}

		void moveStateTrans();
		void movePrimExecution();


	private:
		map<string, vector<sc_bv<MEM_PORT_WIDTH>> > readCoreMemory(map<string, pair<uint64_t, uint64_t> >);
		void writeCoreMemory(map<uint64_t, vector<sc_bv<MEM_PORT_WIDTH>> > data_list);
		map<string, vector<sc_bv<MEM_PORT_WIDTH>> > readDram(map<string, pair<uint64_t, uint64_t> >);
		void writeDram(map<uint64_t, vector<sc_bv<MEM_PORT_WIDTH>> > data_list);

	public:
		sc_port<burst_ram_if<sc_bv<MEM_PORT_WIDTH>> > dram_port;
		sc_port<ram_if<sc_bv<MEM_PORT_WIDTH> > > core_mem_port;

		sc_in<sc_bv<MEM_PORT_WIDTH> > prim_in;
		sc_in<bool> rst;
		sc_in<bool> move_prim_start;
		sc_out<bool> move_busy;

		sc_signal<move_unit_status> move_status;
		sc_signal<bool> move_prim_finish;


	public:
		Event_engine* e_engine_;

	};

} // namespace core
