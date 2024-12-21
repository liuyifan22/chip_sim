#pragma once
#include "systemc.h"
#include "../components/ram.h"
#include "../components/dram.h"
#include "../prims/primitive.h"
#include "../components/functional_noc.h"

namespace core {
	enum router_unit_status
	{
		ROUTER_IDLE = 0, SEND, RECEIVE
	};


	template <class T>
	class RouterUnit : public sc_module
	{
	public:
		SC_HAS_PROCESS(RouterUnit);
		RouterUnit(const sc_module_name& name, CoreLoc core_loc, FunctionalNoC<T>* noc, Event_engine* _e_engine) :
			sc_module(name), e_engine_(_e_engine), noc(noc), loc(core_loc)
		{
			receive_buffer = new ReceiveBuffer<T>(receive_event, irq_events);
			noc->add_router(loc, receive_buffer, &message_received_event);

			SC_THREAD(router_state_trans);
			sensitive << rst.pos() << router_prim_finish.posedge_event() << router_prim_start.pos();
			dont_initialize();

			SC_THREAD(router_prim_execution);
			sensitive << router_status << rst.pos();
			dont_initialize();
		}

		~RouterUnit();
		void router_state_trans();
		void router_prim_execution();


	public:
		CoreLoc loc;

		sc_in<sc_bv<MEM_PORT_WIDTH> > prim_in;

		sc_in<bool> rst;
		sc_in<bool> router_prim_start;
		sc_out<bool> router_busy;

		sc_port<ram_if<T> > core_mem_port;

		sc_signal<bool> router_prim_finish;
		sc_buffer<router_unit_status> router_status;

		sc_event message_received_event;
		sc_event receive_event;
		sc_event_queue irq_events;
		ReceiveBuffer<T>* receive_buffer;

		Event_engine* e_engine_;
		FunctionalNoC<T>* noc;

	};

	template<class T>
	inline RouterUnit<T>::~RouterUnit()
	{
		delete receive_buffer;
	}

	/// @brief Router unit status transition
	/// Driven signals: router_status, router_busy
	template<class T>
	void RouterUnit<T>::router_state_trans()
	{
		while (true) {
			if (this->rst.read()) {
				this->router_status.write(ROUTER_IDLE);
				this->router_busy.write(false);
			}
			else switch (router_status.read())
			{
			case ROUTER_IDLE:
			{
				// 执行已经发送的原语更加优先
				if (router_prim_start.read()) {
					sc_bv<MEM_PORT_WIDTH> prim_temp = prim_in.read();
					if (prim_temp.range(7, 4) == "0001") {
						this->router_status.write(SEND);
						this->router_busy.write(true);
					}
					else if (prim_temp.range(7, 4) == "0010") {
						this->router_status.write(RECEIVE);
						this->router_busy.write(true);
					}
				}
				break;
			}
			default:
				if (this->router_prim_finish.read()) {
					this->router_status.write(ROUTER_IDLE);
					this->router_busy.write(false);
				}
				break;
			}

			wait();
		}
	}

	/// @brief Router unit execution logic
	/// Driven signals: 
	template<class T>
	void RouterUnit<T>::router_prim_execution()
	{
		while (true) {

			wait();
		}
	}
}
