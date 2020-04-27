/**
* @file msg_channel.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_CHANNEL_HPP__
#define __MSG_CHANNEL_HPP__

#include "logger.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class channel;
////////////////////////////////////////////////////////////////////////////////////////////////////
class channel_node
{
public:
	friend class channel;
	friend class controler_iface;

	channel_node(void) = delete;
	channel_node(bool flag) :m_is_channel(flag), m_prev(nullptr), m_next(nullptr), m_parent(nullptr) {}
	virtual ~channel_node(void) { clear(); }
protected:
	void clear(void);
protected:
	const bool m_is_channel;
	channel_node* m_prev;
	channel_node* m_next;
	channel* m_parent;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class channel : public channel_node
{
public:
	channel(void);
	virtual ~channel(void);
public:
	void attach(channel_node* node);
	void detach(channel_node* node);

	bool post_node(channel_node* node);
	channel_node* front(void);
	bool pop_front(void);
private:
	std::mutex m_mutex;
	channel_node* m_post_root;
	channel_node* m_post_tail;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_CHANNEL_HPP__