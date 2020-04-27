/**
* @file msg_controler.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "msg_controler.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
controler_iface::controler_iface(void) : m_dispatcher(nullptr) {}
controler_iface::~controler_iface(void) { m_dispatcher = nullptr; }
////////////////////////////////////////////////////////////////////////////////////////////////////
void controler_iface::init(dispatcher* _dispatcher)
{
	if (m_dispatcher)
		Clog::error_throw(errors::logic, "controler initialized!");

	m_dispatcher = _dispatcher;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void controler_iface::post_node(channel_node* node)
{
	channel* p_channel = node->m_parent;
	if (p_channel) {
		if (p_channel->post_node(node))
			post_node(p_channel);

		return;
	}

	m_dispatcher->dispatch({ node,this });
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void controler_iface::dispatch_node(channel_node* node)
{
	if (node->m_is_channel ? dispatch_channel(dynamic_cast<channel*>(node)) 
		: dispatch_obj(dynamic_cast<object_iface*>(node)))
		m_dispatcher->dispatch({ node,this });
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool controler_iface::dispatch_channel(channel* p_channel)
{
	channel_node::thread_id_guard guard(p_channel);
	channel_node* node = p_channel->front();
	bool need_post = false;
	if (node->m_is_channel ? dispatch_channel(dynamic_cast<channel*>(node))
		: dispatch_obj(dynamic_cast<object_iface*>(node)))
		return p_channel->post_node(node);
	
	return p_channel->pop_front();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 