#pragma once
#include "systemc.h"
#include<iostream>



/*


Event Type

	Sram_rdwt_type

	Dram_rdwt_type

	core_busy_type

	Dram_burst_rdwt_type

	Jump_prim_type
	


 




 */

using  namespace std;
class Trace_event_util
{
public:
	string m_bar_name;
	string m_color = "None";

};


class Trace_event
{
public:
	Trace_event(string _module_name, string _thread_name, string _type, Trace_event_util _util);
	~Trace_event();

	void record_time(double sim_time);

	string module_name;
	string thread_name;
	string type;
	Trace_event_util util;
	double time;
};

