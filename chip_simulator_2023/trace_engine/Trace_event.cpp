#include "Trace_event.h"



Trace_event::Trace_event(string _module_name, string _thread_name, string _type, Trace_event_util _util)
{
	module_name = _module_name;
	thread_name = _thread_name;
	type = _type;
	util = _util;
}


void Trace_event::record_time(double sim_time)
{
	time = sim_time;
}


Trace_event::~Trace_event()
{

	/*if (clock_event != NULL)
	{

		delete clock_event;

	}*/
}
