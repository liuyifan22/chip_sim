#pragma once
#include <map>
#include <vector>
#include <queue>
#include <stdlib.h>

#include "systemc.h"

#include "../config.h"
#include "router_event.hpp"
#include "receive_buffer.hpp"


template <class T>
class FunctionalNoC : public sc_module
{
public:
	SC_HAS_PROCESS(FunctionalNoC);
	FunctionalNoC(sc_module_name name, Event_engine* _e_engine)
		:sc_module(name), e_engine_(_e_engine)
	{
	}

	~FunctionalNoC() {}


	void add_router(CoreLoc router_location, ReceiveBuffer<T>* router_buffer_handler, sc_event* receipt_event)
	{
		this->router_buffer_map[router_location] = router_buffer_handler;
		this->router_receipt_map[router_location] = receipt_event;
	}

	void send_message(const RouterEvent<T>& send_event)
	{
		CoreLoc source_loc = send_event.source_loc;
		CoreDis distance = send_event.distance;
		int destination_x = source_loc.first + distance.first;
		int destination_y = source_loc.second + distance.second;
		CoreLoc destination_loc(destination_x, destination_y);
		
		sc_assert(router_buffer_map.count(destination_loc) != 0);

		unsigned router_time = abs(distance.first) + abs(distance.second);
		unsigned router_num = unsigned(sizeof(T) * send_event.message.length / DATA_WIDTH);
		wait(router_time + router_num, SC_NS);

		router_buffer_map[destination_loc]->receiveMessage(source_loc, send_event);
	}


	void message_received(const CoreLoc source_loc)
	{
		sc_assert(router_receipt_map.count(source_loc) != 0);
		// 此处如果需要wait，还需传入destination坐标
		router_receipt_map[source_loc]->notify();
	}


public:
	map<CoreLoc, ReceiveBuffer<T>* > router_buffer_map;
	map<CoreLoc, sc_event*> router_receipt_map;
	Event_engine* e_engine_;
};