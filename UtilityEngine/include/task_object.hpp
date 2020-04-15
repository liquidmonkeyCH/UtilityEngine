/**
* @file task_object.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __TASK_OBJECT_HPP__
#define __TASK_OBJECT_HPP__

#include <atomic>
#include <cstdint>
#include <mutex>

#include "mem_message.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class object_iface
{
public:
	enum msg_state
	{
		ms_off = -2,
		ms_error = -1,
		ms_ok = 0,
		ms_wait = 1,
	};

	object_iface(void);
	~object_iface(void) = default;

	object_iface(const object_iface&) = delete;
	object_iface& operator=(const object_iface&) = delete;

	virtual void handle_error(void) = 0;
	virtual void do_close(void* ptr) = 0;
protected:
	object_iface* m_parent;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class message_t>
class object : public object_iface
{
public:
	int get_message(mem::message** msg)
	{
		net_size_t len = 0;
		if (!m_message.comfirm(len))
			return len ? ms_error : ms_wait;

		*msg = &m_message;
		return ms_ok;
	}
private:
	message_t m_message;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_OBJECT_HPP__