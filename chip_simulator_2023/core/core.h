#pragma once

#include "systemc.h"

#include "../config.h"
#include "core_control_unit.h"
#include "core_dendrite_unit.h"
#include "core_move_unit.h"
#include "core_soma_unit.h"
#include "core_router_unit.h"
#include "../components/functional_noc.h"

namespace core {
	typedef FunctionalNoC<sc_bv<MEM_PORT_WIDTH> > FNoC;

	class Core : public sc_module
	{
	public:
		Core(const sc_module_name& name, CoreLoc core_loc, FNoC* noc, Event_engine* _e_engine) :
			sc_module(name), e_engine_(_e_engine), noc(noc), core_loc(core_loc)
		{
			// mem ports
			core_dram = new dram<sc_bv<MEM_PORT_WIDTH> >("core_dram", 0, DRAM_DEPTH_PER_CORE, e_engine_);
			core_memory = new ram<sc_bv<MEM_PORT_WIDTH> >("core_memory", 0, CORE_MEM_DEPTH, e_engine_);

			// wires to/from move unit
			core_move_unit = new MoveUnit("core_move_unit", e_engine_);
			core_move_unit->dram_port(*core_dram);
			core_move_unit->core_mem_port(*core_memory);
			core_move_unit->rst(rst);
			core_move_unit->prim_in(prim_to_move);
			core_move_unit->move_prim_start(move_prim_start);
			core_move_unit->move_busy(move_busy);

			// wires to/from control unit
			core_ctrl = new ControlUnit("core_ctrl", e_engine_);
			core_ctrl->reset(rst);
			core_ctrl->core_memory_port(*core_memory);
			core_ctrl->start(start);
			core_ctrl->stop(stop);
			core_ctrl->in_idle(core_in_idle);

			core_ctrl->prim_to_move(prim_to_move);
			core_ctrl->prim_to_router(prim_to_router);
			core_ctrl->prim_to_dendrite(prim_to_dendrite);
			core_ctrl->prim_to_soma(prim_to_soma);

			core_ctrl->move_prim_start(move_prim_start);
			core_ctrl->router_prim_start(router_prim_start);
			core_ctrl->dendrite_prim_start(dendrite_prim_start);
			core_ctrl->soma_prim_start(soma_prim_start);

			core_ctrl->move_busy(move_busy);
			core_ctrl->router_busy(router_busy);
			core_ctrl->dendrite_busy(dendrite_busy);
			core_ctrl->soma_busy(soma_busy);

			// wires to/from router unit
			core_router_unit = new RouterUnit<sc_bv<MEM_PORT_WIDTH> >("core_router_unit", core_loc, this->noc, e_engine_);
			core_router_unit->prim_in(prim_to_router);
			core_router_unit->rst(rst);
			core_router_unit->router_prim_start(router_prim_start);
			core_router_unit->router_busy(router_busy);
			core_router_unit->core_mem_port(*core_memory);

			// wires to/from dendrite unit
			core_dendrite_unit = new DendriteUnit("core_dendrite_unit", this->e_engine_);
			core_dendrite_unit->prim_in(prim_to_dendrite);
			core_dendrite_unit->rst(rst);
			core_dendrite_unit->dendrite_prim_start(dendrite_prim_start);
			core_dendrite_unit->dendrite_busy(dendrite_busy);
			core_dendrite_unit->core_mem_port(*core_memory);

			// wires to/from soma unit
			core_soma_unit = new SomaUnit("core_soma_unit", this->e_engine_);
			core_soma_unit->prim_in(prim_to_soma);
			core_soma_unit->rst(rst);
			core_soma_unit->soma_prim_start(soma_prim_start);
			core_soma_unit->soma_busy(soma_busy);
			core_soma_unit->core_mem_port(*core_memory);
		}

		~Core() {
			delete core_memory;
			delete core_dram;
			delete core_soma_unit;
			delete core_ctrl;
			delete core_router_unit;
			delete core_dendrite_unit;
			delete core_move_unit;
		}

		vector<sc_bv<MEM_PORT_WIDTH> > readMem(uint64_t addr, uint64_t length);
		void writePrims(uint64_t addr, vector<sc_bv<MEM_PORT_WIDTH> >& prim_list);
		void writeData(uint64_t addr, vector<sc_bv<MEM_PORT_WIDTH> >& data);
		void writeData(map<uint64_t, vector<sc_bv<MEM_PORT_WIDTH> > >& data_list);


	public:
		// for simulator
		CoreLoc core_loc;
		Event_engine* e_engine_;
		FNoC* noc;

		// modules
		MoveUnit* core_move_unit;
		dram<sc_bv<MEM_PORT_WIDTH> >* core_dram;
		ram<sc_bv<MEM_PORT_WIDTH> >* core_memory;
		ControlUnit* core_ctrl;
		RouterUnit<sc_bv<MEM_PORT_WIDTH> >* core_router_unit;
		DendriteUnit* core_dendrite_unit;
		SomaUnit* core_soma_unit;


		// signals
		sc_signal<sc_bv<MEM_PORT_WIDTH> > prim_to_move;
		sc_signal<bool> move_prim_start;
		sc_signal<bool> move_busy;

		sc_signal<sc_bv<MEM_PORT_WIDTH> > prim_to_router;
		sc_signal<bool> router_prim_start;
		sc_signal<bool> router_busy;

		sc_signal<sc_bv<MEM_PORT_WIDTH> > prim_to_dendrite;
		sc_signal<bool> dendrite_prim_start;
		sc_signal<bool> dendrite_busy;

		sc_signal<sc_bv<MEM_PORT_WIDTH> > prim_to_soma;
		sc_signal<bool> soma_prim_start;
		sc_signal<bool> soma_busy;

		// ports
		sc_in<bool> rst;
		sc_in<bool> start;
		sc_in<bool> stop;
		sc_out<bool> core_in_idle;
	};
}