#pragma once
#include <queue>
#include <vector>
#include <map>
#include "router_event.hpp"


template <class T>
class ReceiveBuffer
{
public:
	ReceiveBuffer(sc_event& receive_events, sc_event_queue& irq_events): 
		receive_events(receive_events), irq_events(irq_events)
	{}
	~ReceiveBuffer() {}
	void receiveMessage(CoreLoc source_core, const RouterEvent<T>& event)
	{
		RouterMessage<T> message(event.message);
		if (message.to_cause_interruption)
		{
			irq_queue.push(*(& event));
			irq_events.notify(SC_ZERO_TIME);
		}
		else
		{
			regular_messages.emplace(source_core, message);
			receive_events.notify(SC_ZERO_TIME);
		}
	}

public:
	//TODO: 解决没有默认构造函数的问题
	map<CoreLoc, RouterMessage<T> > regular_messages;
	queue<RouterEvent<T> > irq_queue;
	sc_event_queue& irq_events;
	sc_event& receive_events;
};