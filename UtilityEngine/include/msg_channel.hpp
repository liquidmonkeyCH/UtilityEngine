/**
* @file msg_channel.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_CHANNEL_HPP__
#define __MSG_CHANNEL_HPP__

#include "msg_object.hpp"
#include "logger.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class dispatcher;
////////////////////////////////////////////////////////////////////////////////////////////////////
class channel_node : public object_iface
{
public:
	friend class channel;
	template<class message_wrap, class handler_manager> friend class controler_wrap;

	channel_node(void) :m_posting(false), m_prev(nullptr), m_next(nullptr), m_parent(nullptr) {}
	virtual ~channel_node(void) { clear(); }

	virtual void post_message(dispatcher* _dispatcher, bool good);
protected:
	void clear(void);
	virtual bool post_node(channel_node* node) { return false; }
protected:
	bool m_posting;
	channel_node* m_prev;
	channel_node* m_next;
	channel_node* m_parent;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class channel 
{
public:
	channel(void);
	~channel(void);
public:
	void attach(channel_node* node);
	void detach(channel_node* node);

	bool post_node(channel_node* node);
	channel_node* pop_post(void);
private:
	std::mutex m_post_lock;
	std::mutex m_wait_lock;
	channel_node* m_post_root;
	channel_node* m_post_tail;
	channel_node* m_wait_root;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_CHANNEL_HPP__