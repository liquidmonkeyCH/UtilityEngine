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
public:
	friend class dispatcher;
	controler_iface(void);
	~controler_iface(void);

	controler_iface(const controler_iface&) = delete;
	controler_iface& operator=(const controler_iface&) = delete;

	void init(dispatcher* _dispatcher);
protected:
	void post_node(channel_node* node);
	void dispatch_node(channel_node* node);
	bool dispatch_channel(channel* p_channel);
	virtual bool dispatch_obj(object_iface* obj) = 0;
protected:
	dispatcher* m_dispatcher;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class message_wrap,class handler_manager>
class controler_wrap : public handler_manager,public controler_iface
{
public:
	using message_t = message_wrap;
public:
	controler_wrap(void) = default;
	~controler_wrap(void) = default;

	controler_wrap(const controler_wrap&) = delete;
	controler_wrap& operator=(const controler_wrap&) = delete;
public:
	void post_request(object_iface* obj, mem::message* message)
	{
		switch (dynamic_cast<message_t*>(message)->comfirm())
		{
		case state::ok:
		case state::bad:
			this->post_node(obj);
			break;
		case state::pending:
			break;
		case state::error:
			obj->handle_error();
			break;
		default:
			break;
		}
	}
private:
	bool dispatch_obj(object_iface* obj)
	{
		mem::message* message = obj->get_message();
		message_t* msg = dynamic_cast<message_t*>(message);

		if (!msg->is_good()){
			obj->do_close();
			return false;
		}

		handler_t handle = this->get_handle(msg);
		if (!handle || handle(obj, message) != 0)
		{
			obj->handle_error();
			return false;
		}

		msg->commit();
		switch (msg->comfirm())
		{
		case state::ok:
			return true;
		case state::pending:
			break;
		case state::error:
			obj->handle_error();
			break;
		case state::bad:
			obj->do_close();
			break;
		default:
			break;
		}

		return false;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_CONTROLER_HPP__