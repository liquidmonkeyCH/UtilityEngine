/**
* @file msg_controler.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_CONTROLER_HPP__
#define __MSG_CONTROLER_HPP__

#include "msg_dispatcher.hpp"
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
	friend class dispatcher;
	virtual void handle_wrap(object_iface* obj, mem::message* message, void* ptr, bool good) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class message_wrap,class handler_manager>
class controler_wrap : public handler_manager,public controler_iface
{
public:
	friend class dispatcher;
	using message_t = message_wrap;
public:
	controler_wrap(void) : m_dispatcher(nullptr){}
	~controler_wrap(void) = default;

	controler_wrap(const controler_wrap&) = delete;
	controler_wrap& operator=(const controler_wrap&) = delete;
public:
	void init(dispatcher* _dispatcher)
	{
		if (m_dispatcher)
			Clog::error_throw(errors::logic, "controler initialized!");

		m_dispatcher = _dispatcher;
	}

	void post_request(object_iface* obj, mem::message* message, bool is_dispatcher = false)
	{
		switch (dynamic_cast<message_t*>(message)->comfirm())
		{
		case state::ok:
			obj->post_message(m_dispatcher);
			break;
		case state::pending:
			break;
		case state::error:
			obj->handle_error();
			break;
		case state::bad:
			if (is_dispatcher) obj->do_close();
			else obj->post_message(m_dispatcher);
			break;
		default:
			break;
		}
	}
private:
	void handle_wrap(object_iface* obj)
	{
		object_iface* real_obj;
		mem::message* message = obj->get_message(real_obj);
		message_t* msg = dynamic_cast<message_t*>(message);
		std::uint32_t reason;

		if (!msg->is_good()){
			obj->do_close();
			return;
		}

		handler_t handle = this->get_handle(msg);
		if (!handle || handle(obj, message, ptr) != 0)
		{
			obj->handle_error();
			return;
		}

		msg->commit();
		post_request(real_obj, message, true);
	}
private:
	dispatcher* m_dispatcher;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_CONTROLER_HPP__