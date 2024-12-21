#pragma once
#include"systemc.h"
#include "Trace_event_queue.h"
#include <map>
#include <vector>
#include "Trace_event.h"

class Event_engine : public sc_module
{
public:
	SC_HAS_PROCESS(Event_engine);
	Event_engine(sc_module_name name)
		:sc_module(name)
	{
		SC_THREAD(engine_run);
		sensitive << sync_events;
	}

	~Event_engine();
	void engine_run();
	void add_event(string _module_name, string _thread_name, string _type, Trace_event_util _util);
	void dump_traced_file(const string& filepath);

private:
	void writeJsonHeader();
	void writeEvents();
	void writeJsonTail();

public:
	vector<Trace_event> traced_event_list;
	Trace_event_queue Trace_event_queue_clock_engine;
	sc_event_queue sync_events;
	std::ofstream json_stream;
};

