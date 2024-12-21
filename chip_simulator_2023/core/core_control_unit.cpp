#include "core_control_unit.h"

/// @brief destructor of core controller
core::ControlUnit::~ControlUnit()
{
}

/// @brief Core status transition logic
/// Driven signals: ctrl_status, in_idle
void core::ControlUnit::ctrlStatusTrans()
{
	while (true)
	{
		if (this->reset.read())
		{ // reset
			this->ctrl_status.write(CORE_IDLE);
			this->in_idle.write(true);
		}
		else if (this->ctrl_status.read() == CORE_IDLE) // start: IDLE to PRIM_PROCESS
		{
			if (start.read())
			{
				this->ctrl_status.write(PRIM_PROCESS);
				this->in_idle.write(false);
			}
		}
		else if (this->ctrl_status.read() == PRIM_PROCESS)
		{
			if (this->process_finish.read())
			{
				this->ctrl_status.write(EXECUTION);
				this->in_idle.write(false);
			}
		}
		else if (this->ctrl_status.read() == EXECUTION)
		{
			if (this->execution_finish.read())
			{
				if (stop.read() || to_idle.read())
				{
					this->ctrl_status.write(CORE_IDLE);
					this->in_idle.write(true);
				}
				else
				{
					this->ctrl_status.write(PRIM_PROCESS);
					this->in_idle.write(false);
				}
			}
		}
		wait();
	}
}

/// @brief Control core's behavior in different statuses
/// Driven signals: *_prim_start, prim_to_*, exe_para_to_*, pc, process_finish, execution_finish, to_idle
void core::ControlUnit::run()
{
	while (true)
	{
		if (this->reset.read())
		{
			move_prim_start.write(false);
			router_prim_start.write(false);
			dendrite_prim_start.write(false);
			soma_prim_start.write(false);

			prim_to_dendrite.write(0);
			prim_to_move.write(0);
			prim_to_soma.write(0);
			prim_to_router.write(0);

			pc.write(0);
			process_finish.write(false);
			execution_finish.write(false);
			to_idle.write(false);
		}
		else if (this->ctrl_status.read() == CORE_IDLE)
		{
			move_prim_start.write(false);
			router_prim_start.write(false);
			dendrite_prim_start.write(false);
			soma_prim_start.write(false);

			process_finish.write(false);
			execution_finish.write(false);
			to_idle.write(false);
			// pc is not resetted in idle status
		}
		else if (this->ctrl_status.read() == PRIM_PROCESS)
		{
			move_prim_start.write(false);
			router_prim_start.write(false);
			dendrite_prim_start.write(false);
			soma_prim_start.write(false);

			execution_finish.write(false);
			to_idle.write(false);

			sc_bv<MEM_PORT_WIDTH> prim_temp;
			core_memory_port->read(pc.read(), prim_temp); // read a prim
			cout << "reading prim@ " << pc.read() << endl;
			prim_code.write(prim_temp);

			prim_op_code.write(prim_temp.range(3, 0));

			pc.write(pc.read() + 1); // increase instruction-fetching address
			process_finish.write(true);
			wait(SC_ZERO_TIME);
		}
		else if (this->ctrl_status.read() == EXECUTION)
		{
			process_finish.write(false);

			uint64_t type_code = prim_op_code.read().to_uint();
			cout << "executing prim: " << type_code << " @" << sc_time_stamp() << endl;
			switch (type_code)
			{
			case 0x6:
			case 0x7: // dendrite
			{
				move_prim_start.write(false);
				router_prim_start.write(false);
				dendrite_prim_start.write(true);
				soma_prim_start.write(false);

				prim_to_dendrite.write(prim_code.read());
				wait(dendrite_busy.negedge_event());
				break;
			}

			case 0x4:
			case 0x5: // soma
			{
				move_prim_start.write(false);
				router_prim_start.write(false);
				dendrite_prim_start.write(false);
				soma_prim_start.write(true);

				prim_to_soma.write(prim_code.read());
				wait(soma_busy.negedge_event());
				break;
			}

			case 0x2:
			case 0x3: // move
			{
				move_prim_start.write(true);
				router_prim_start.write(false);
				dendrite_prim_start.write(false);
				to_idle.write(false);

				prim_to_move.write(prim_code.read());
				wait(move_busy.negedge_event());
				break;
			}

			case 0x0:
			case 0x1: // control
			{
				move_prim_start.write(false);
				router_prim_start.write(false);
				dendrite_prim_start.write(false);
				soma_prim_start.write(false);

				if (0b0000 == prim_op_code.read()) // stop
				{
					pc.write(0);
					this->to_idle.write(true);
					break;
				}
				else if (0b0001 == prim_op_code.read()) // jump
				{
					sc_bv<MEM_PORT_WIDTH> prim_temp = prim_code.read();
					if (prim_temp.range(116, 116) == 1)
					{
						pc.write(pc.read() - 1 + prim_temp.range(67, 52).to_int());
					}
					else
					{
						pc.write(prim_temp.range(67, 52).to_uint());
					}

					this->to_idle.write(false);
					break;
				}
			}

			default: // other exe prims
				assert(0);
			}
			execution_finish.write(true);
			wait(SC_ZERO_TIME);
		}
		wait();
	}
}
