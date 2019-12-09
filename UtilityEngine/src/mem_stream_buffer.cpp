/**
* @file mem_stream_buffer.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "mem_stream_buffer.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
stream_buffer::stream_buffer(void)
: m_head(nullptr)
, m_tail(nullptr)
, m_reader(nullptr)
, m_writer(nullptr)
, m_readable(0)
, m_lastread(0)
#ifndef NDEBUG
, m_last_malloc(0)
#endif
, m_next(nullptr)
, m_offset(0)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
stream_buffer::~stream_buffer(void)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
stream_buffer::clear(void)
{
	m_factory.clear();
	
	m_head = m_factory.malloc();
	m_head->m_next = nullptr;
	m_tail = m_head;
	
	m_writer = m_head->m_buffer;
	m_reader = m_head->m_buffer;

	m_lastread = 0;
	m_readable = 0;
#ifndef NDEBUG
	m_last_malloc = 0;
#endif

	m_limit = 0;
	m_position = 0;
	m_next = m_head;
	m_offset = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
stream_buffer::init(std::size_t size)
{
	assert(size >= MAX_PACKET_LEN);

	size_t nchunk = size / MAX_PACKET_LEN;
	m_factory.init(size%MAX_PACKET_LEN ? nchunk +1 : nchunk);
	clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
net_size_t
stream_buffer::readable_size(net_size_t exp)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_lastread = m_readable;
	if (m_lastread < exp) m_lastread = 0;
	return static_cast<net_size_t>(m_lastread);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char*
stream_buffer::read(net_size_t& size)
{
	std::size_t len = m_head->m_buffer + MAX_PACKET_LEN - m_reader;
	std::lock_guard<std::mutex> lock(m_mutex);
	len = len > m_readable ? m_readable : len;
	size = size > len ? static_cast<net_size_t>(len) : size;
	m_lastread = size;
	return m_reader;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
stream_buffer::commit_read(net_size_t size)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	assert(size <= m_readable);
	m_readable -= size;
	net_size_t len = static_cast<net_size_t>(m_head->m_buffer + MAX_PACKET_LEN - m_reader);
	if (size < len) {		// 不需要跳转下一个node
		m_reader += size;
		return;
	}

	stream_node* tmp;
	do {
		tmp = m_head;
		m_head = m_head->m_next;
		m_factory.free(tmp);
		size -= len;
		if (size < MAX_PACKET_LEN) {
			m_reader = m_head->m_buffer + size;
			break;
		}
	} while (true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
net_size_t
stream_buffer::writable_size(void)
{
	return ULONG_MAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
char*
stream_buffer::write(net_size_t& size)
{
	std::size_t left = m_tail->m_buffer + MAX_PACKET_LEN - m_writer;
	size = (size == 0 || size > left) ? static_cast<net_size_t>(left) : size;
#ifndef NDEBUG
	m_last_malloc = size;
#endif
	return m_writer;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
stream_buffer::commit_write(net_size_t size)
{
	std::lock_guard<std::mutex> lock(m_mutex);
#ifndef NDEBUG
	assert(m_last_malloc >= size);
	m_last_malloc = 0;
#endif

	m_writer += size;
	m_readable += size;

	if (m_writer >= m_tail->m_buffer + MAX_PACKET_LEN) {
		m_tail->m_next = m_factory.malloc();
		m_tail = m_tail->m_next;
		m_tail->m_next = nullptr;
		m_writer = m_tail->m_buffer;
	}

	return (m_lastread == 0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// **** message iface
////////////////////////////////////////////////////////////////////////////////////////////////////
void
stream_buffer::reset(void)
{
	m_position = 0;
	m_offset = static_cast<net_size_t>(m_reader - m_head->m_buffer);
	m_next = m_head;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char*
stream_buffer::_next(net_size_t& size, net_size_t limit)
{
	net_size_t left = limit - m_position;

	if (m_offset >= MAX_PACKET_LEN)
	{
		m_offset = 0;
		m_next = m_next->m_next;
	}

	limit = static_cast<net_size_t>(MAX_PACKET_LEN) - m_offset;
	if (left > limit) left = limit;
	if (size == 0 || size > left) size = left;
	left = m_offset;
	m_position += size;
	m_offset += size;

	return m_next->m_buffer + left;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char*
stream_buffer::next(net_size_t& size)
{
	net_size_t limit = m_limit > 0 ? m_limit : readable_size(0);
	assert(m_limit <= limit);
	if (limit <= m_position) {
		size = 0;
		return nullptr;
	}

	return _next(size, limit);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
stream_buffer::skip(net_size_t size)
{
	net_size_t limit = m_limit > 0 ? m_limit : readable_size(0);
	assert(m_limit <= limit);

	if (size > limit - m_position)
		return false;

	net_size_t left = size;
	do {
		size = left;
		_next(size,limit);
		left -= size;
	} while (left > 0);

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
stream_buffer::back_up(net_size_t size)
{
	if (size > m_position)
		return false;

	if (size <= m_offset)
	{
		m_position -= size;
		m_offset -= size;
		return true;
	}

	// performs warning! 
	size = m_position - size;
	reset();
	if (size == 0) return true;
	net_size_t limit = m_limit;
	m_limit = 0;
	skip(size);
	m_limit = limit;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility