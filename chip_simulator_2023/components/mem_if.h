#ifndef _MEM_IF_H
#define _MEM_IF_H
#include "systemc.h"

enum transfer_status {TRANSFER_OK = 0, TRANSFER_ERROR};


template<class T>
class mem_clear_if : virtual public sc_interface {
public:
	virtual transfer_status clear(uint64_t address, T& data) = 0;
};

template<class T>
class mem_read_if : virtual public sc_interface {
public:
	virtual transfer_status read(uint64_t address, T& data) = 0;
};

template<class T>
class mem_write_if : virtual public sc_interface {
public:
	virtual transfer_status write(uint64_t address, T& data, uint64_t force_write) = 0;
};

class reset_if : virtual public sc_interface
{
public:
	virtual bool reset() = 0;
};

template<class T>
class ram_if : public mem_write_if<T>, public mem_read_if<T>, public reset_if, public mem_clear_if<T> {
public:
	virtual uint64_t start_address() const = 0;
	virtual uint64_t end_address() const = 0;
};


template<class T>
class rom_if : public mem_read_if<T>, public reset_if {
public:
	virtual uint64_t start_address() const = 0;
	virtual uint64_t end_address() const = 0;
};

template<class T>
class burst_ram_if : public ram_if<T> {
public:
	virtual transfer_status burst_read(uint64_t address, uint64_t length, T* &data) = 0;
	virtual transfer_status burst_write(uint64_t address, uint64_t length, T* &data, uint64_t force_write) = 0;
};


#endif

