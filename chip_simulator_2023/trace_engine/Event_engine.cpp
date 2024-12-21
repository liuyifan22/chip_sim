#include "Event_engine.h"


void Event_engine::engine_run()
{
	while(1)
	{
		wait();
		if (Trace_event_queue_clock_engine.trace_event_queue.size() != 0)
		{	
			Trace_event e_temp = Trace_event_queue_clock_engine.trace_event_queue.front();
			e_temp.record_time(sc_time_stamp().to_double());
			this->traced_event_list.push_back(e_temp);
			Trace_event_queue_clock_engine.trace_event_queue.pop();
		}
	}

}

void Event_engine::add_event(string _module_name, string _thread_name, string _type, Trace_event_util _util)
{
	Trace_event e_temp(_module_name, _thread_name, _type, _util);
	Trace_event_queue_clock_engine.trace_event_queue.push(e_temp);
	sync_events.notify(SC_ZERO_TIME);
}


void Event_engine::dump_traced_file(const string& filepath = "events.json")
{
	json_stream.open(filepath);
	writeJsonHeader();
	writeEvents();
	writeJsonTail();
	json_stream.close();
}


void Event_engine::writeJsonHeader()
{
	json_stream << "{\n\"otherData\": {}, \n\"traceEvents\": [";
	json_stream.flush();
}


void Event_engine::writeJsonTail()
{
	json_stream << "]\n}";
	json_stream.flush();
}


void Event_engine::writeEvents()
{
	// collect all unique module names and give a pid
	typedef pair<string, unsigned> module_name_and_idx;
	map<string, unsigned> module_idx;
	unsigned pid_count = 1;
	for (auto iter = traced_event_list.begin(); iter != traced_event_list.end(); iter++)
	{
		string m_name = iter->module_name;
		if (module_idx.end() == module_idx.find(m_name))
		{
			module_idx.insert(module_name_and_idx(m_name, pid_count));
			pid_count++;
		}
	}

	for (auto iter = module_idx.begin(); iter != module_idx.end(); iter++)
	{
		json_stream << "{\"name\": \"process_name\", \"ph\": \"M\", \"pid\":";
		json_stream << iter->second;
		json_stream << ", \"args\": {\"name\": \"";
		json_stream << iter->first;
		json_stream << "\"}},\n";
	}

	// collect all thread names and give a tid
	typedef pair<string, string> module_name_and_thread_name;
	map<module_name_and_thread_name, unsigned> thread_idx;
	
	map<string, unsigned> thread_count_in_module = module_idx; // just for initialization
	for (auto iter = thread_count_in_module.begin(); iter != thread_count_in_module.end(); iter++)
	{
		iter->second = 0;
	}

	for (auto iter = traced_event_list.begin(); iter != traced_event_list.end(); iter++)
	{
		string m_name = iter->module_name;
		string t_name = iter->thread_name;
		module_name_and_thread_name m_t_name(m_name, t_name);
		if (thread_idx.end() == thread_idx.find(m_t_name))
		{
			thread_idx.insert(pair<module_name_and_thread_name, unsigned>(m_t_name, thread_count_in_module.at(m_name)));
			(thread_count_in_module.at(m_name)) ++;
		}
	}

	for (auto iter = thread_idx.begin(); iter != thread_idx.end(); iter++)
	{
		string m_name = iter->first.first;
		string t_name = iter->first.second;
		unsigned tid = iter->second;
		json_stream << "{\"name\": \"thread_name\", \"ph\": \"M\", \"pid\": ";
		json_stream << module_idx.at(m_name);
		json_stream << ", \"tid\": ";
		json_stream << thread_idx.at(module_name_and_thread_name(m_name, t_name));
		json_stream << ", \"args\": {\"name\": \"";
		json_stream << t_name;
		json_stream << "\"}},\n";
	}


	// write event
	for (auto iter = traced_event_list.begin(); iter != traced_event_list.end(); iter++)
	{
		string m_name = iter->module_name;
		string t_name = iter->thread_name;
		string bar_name = iter->util.m_bar_name;
		string event_type = iter->type;
		string color_type = iter->util.m_color;
		double ts = iter->time;

		module_name_and_thread_name m_t_name(m_name, t_name);
		
		json_stream << "{\"name\": \"";
		json_stream << bar_name;
		json_stream << "\", \"cat\": \"";
		json_stream << m_name;
		if (color_type != "None")
		{
			json_stream << "\", \"cname\": \"";
			json_stream << color_type;
		}
		json_stream << "\", \"ph\": \"";
		json_stream << event_type;
		json_stream << "\", \"ts\": ";
		json_stream << ts;
		json_stream << ", \"pid\": ";
		json_stream << module_idx.at(m_name);
		json_stream << ", \"tid\": ";
		json_stream << thread_idx.at(module_name_and_thread_name(m_name, t_name));
		json_stream << "}";

		if (iter != traced_event_list.end() - 1)
		{
			json_stream << ",\n";
		}
	}


}


Event_engine::~Event_engine()
{
}
