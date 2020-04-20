/**
* @file msg_controler.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_CONTROLER_HPP__
#define __MSG_CONTROLER_HPP__

#include "task_dispatcher.hpp"
#include "msg_message.hpp"
#include "logger.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class controler_iface
{
	friend class task::dispatcher;
	virtual void handle_wrap(task::object_iface* obj, mem::message* message, void* ptr, bool good) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class message_wrap,class handler_manager>
class controler_wrap : public handler_manager,public controler_iface
{
public:
	friend class task::dispatcher;
	using message_t = message_wrap;
	using dispatcher_t = task::dispatcher;
public:
	controler_wrap(void) : m_dispatcher(nullptr){}
	~controler_wrap(void) = default;

	controler_wrap(const controler_wrap&) = delete;
	controler_wrap& operator=(const controler_wrap&) = delete;
public:
	void init(dispatcher_t* _dispatcher)
	{
		if (m_dispatcher)
			Clog::error_throw(errors::logic, "controler initialized!");

		m_dispatcher = _dispatcher;
	}

	void post_request(task::object_iface* obj, void* ptr, bool dispatcher = false)
	{
		mem::message* message = obj->get_message();
		if (!message) return;
		switch (dynamic_cast<message_t*>(message)->comfirm())
		{
		case state::ok:
			m_dispatcher->dispatch({ this, obj, message, ptr, true });
			break;
		case state::pending:
			break;
		case state::error:
			obj->handle_error();
			break;
		case state::bad:
			if (dispatcher) obj->do_close(ptr);
			else m_dispatcher->dispatch({ this, obj, message, ptr, false });
			break;
		default:
			break;
		}
	}
private:
	void handle_wrap(task::object_iface* obj, mem::message* message, void* ptr, bool good)
	{
		if (!good){
			obj->do_close(ptr);
			return;
		}

		message_t* msg = dynamic_cast<message_t*>(message);
		handler_t handle = this->get_handle(msg);
		if (!handle || handle(obj, message, ptr) != 0)
		{
			obj->handle_error();
			return;
		}

		msg->commit();
		post_request(obj, ptr, true);
	}
private:
	dispatcher_t* m_dispatcher;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_CONTROLER_HPP__