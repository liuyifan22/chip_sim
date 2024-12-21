#pragma once


typedef std::pair<int, int> CoreLoc;
typedef std::pair<int, int> CoreDis;


template <class T>
class RouterMessage
{
public:
	RouterMessage(bool content_type, bool interruption, unsigned receive_addr, unsigned length, T* data, bool to_start = 0, bool to_idle = 0):
		message_content_type(content_type), 
		to_cause_interruption(interruption), 
		addr_at_destination(receive_addr), 
		length(length), 
		data(data),
		to_start(to_start),
		to_idle(to_idle)
	{
		if (!to_cause_interruption)
		{
			this->to_start = false;
			this->to_idle = false;
		}
	}

	RouterMessage(const RouterMessage& rm)
	{
		message_content_type = rm.message_content_type;
		to_cause_interruption = rm.to_cause_interruption;
		addr_at_destination = rm.addr_at_destination;
		length = rm.length;
		data = rm.data;
		to_start = rm.to_start;
		to_idle = rm.to_idle;
	}

	RouterMessage& operator=(const RouterMessage& rm)
	{
		if (this == &rm)
		{
			return *this;
		}

		if (this->data != NULL)
		{
			delete[] this->data;
			this->data = NULL;
		}
		
		this->message_content_type = rm.message_content_type;
		this->to_cause_interruption = rm.to_cause_interruption;
		this->addr_at_destination = rm.addr_at_destination;
		this->length = rm.length;
		this->data = rm.data;
		this->to_start = rm.to_start;
		this->to_idle = rm.to_idle;

		return *this;
	}

	~RouterMessage()
	{
		// 注意这里的指针是否需要析构？
	}

public:
	bool message_content_type;  // 1: data, 0: instruction
	bool to_cause_interruption;
	bool to_start, to_idle;
	unsigned addr_at_destination;
	unsigned length;
	T* data;
};


template <class T>
class RouterEvent
{
public:
	RouterEvent(CoreLoc source_loc, CoreDis send_distance, RouterMessage<T> message_to_send)
		:source_loc(source_loc), distance(send_distance), message(message_to_send)
	{}

	~RouterEvent() {}

public:
	CoreLoc source_loc;
	CoreDis distance;
	RouterMessage<T> message;
};
