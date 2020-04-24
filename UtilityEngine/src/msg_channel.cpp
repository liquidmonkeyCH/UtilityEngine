/**
* @file msg_channel.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "msg_channel.hpp"
#include "msg_controler.hpp"

#define ADD_TO_WAIT_LIST(node)					\
	if (m_wait_root){							\
		node->m_next = m_wait_root;				\
		node->m_prev = m_wait_root->m_prev;		\
		m_wait_root->m_prev = node;				\
	}											\
	m_wait_root = node;

#define DEL_FROM_WAIT_LIST(node) 				\
	if(node == m_wait_root){					\
		m_wait_root = node->m_next;				\
		if(m_wait_root)							\
			m_wait_root->m_prev = nullptr;		\
	}else{										\
		node->m_prev->m_next = node->m_next;	\
		if(node->m_next)						\
			node->m_next->m_prev = node->m_prev;\
	}	

#define ADD_TO_POST_LIST(node)					\
	node->m_prev = m_post_tail;					\
	node->m_next = nullptr;						\
	m_post_tail ? m_post_tail->m_next = node : m_post_root = m_post_tail = node;

#define DEL_FORM_POST_LIST(node)				\
	channel_node* node = m_post_root;			\
	m_post_root = m_post_root->m_next;			\
	m_post_root ? m_post_root->m_prev = nullptr : m_post_tail = nullptr;
		
namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
void channel_node::clear(void)
{
	m_posting = false;

	if (m_prev)
		m_prev->m_next = m_next;

	if (m_next)
		m_next->m_prev = m_prev;

	m_parent = nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void channel_node::post_message(dispatcher* _dispatcher, bool good)
{
	if (m_parent)
	{
		m_parent->post_node(this);
		m_parent->post_message(_dispatcher, good);
		return;
	}

	_dispatcher->dispatch({this,})
}
////////////////////////////////////////////////////////////////////////////////////////////////////
channel::channel(void)
: m_post_root(nullptr)
, m_post_tail(nullptr)
, m_wait_root(nullptr)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
channel::~channel(void)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void channel::attach(channel_node* node)
{
	if (node->m_parent)
		Clog::error_throw(errors::logic, "node already have channel!");

	ADD_TO_WAIT_LIST(node);
	node->m_parent = this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void channel::detach(channel_node* node)
{
	if (node->m_parent != this)
		Clog::error_throw(errors::logic, "node not belong to channel!");

	if (node->m_posting)
	{
		DEL_FORM_POST_LIST(node);
	}
	else
	{
		DEL_FROM_WAIT_LIST(node);
	}

	node->m_parent = nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool channel::post_node(channel_node* node)
{
	if (node->m_parent != this)
		Clog::error_throw(errors::logic, "node is not child!");

	if (node->m_posting)
		Clog::error_throw(errors::logic, "node post duplicate!");

	DEL_FROM_WAIT_LIST(node);
	ADD_TO_POST_LIST(node);
	node->m_posting = true;

	if (m_parent && m_post_root == node)
		return m_parent->post_node(this);

	return m_post_root == node;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
channel_node* channel::pop_post(void)
{
	if (!m_post_root)
		return nullptr;

	DEL_FORM_POST_LIST(node);
	ADD_TO_WAIT_LIST(node);
	return node;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 