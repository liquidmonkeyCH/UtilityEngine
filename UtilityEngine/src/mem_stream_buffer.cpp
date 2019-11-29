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
, m_next(nullptr)
, m_reader(nullptr)
, m_writer(nullptr)
, m_out(nullptr)
, m_readable(0)
, m_lastread(0)
#ifndef NDEBUG
, m_last_malloc(0)
#endif
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
	m_next = m_head;

	m_writer = m_head->m_buffer;
	m_reader = m_head->m_buffer;
	m_out = m_reader;

	m_pos = 0;
	m_limit = 0;
	m_lastread = 0;
	m_readable = 0;
#ifndef NDEBUG
	m_last_malloc = 0;
#endif
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
	assert(size <= m_readable);
	std::lock_guard<std::mutex> lock(m_mutex);
	m_readable -= size;
	net_size_t len = static_cast<net_size_t>(m_head == m_tail ? m_writer - m_reader : m_head->m_buffer + MAX_PACKET_LEN - m_reader);
	stream_node* tmp;
	do
	{
		m_reader += size;
		if (m_reader < m_head->m_buffer + MAX_PACKET_LEN) break;
		
		size -= len;
		tmp = m_head;
		m_head = m_head->m_next;
		m_reader = m_head->m_buffer;
		m_factory.free(tmp);
		len = static_cast<net_size_t>(m_head == m_tail ? m_writer - m_reader : MAX_PACKET_LEN);
		
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
	size = size > left ? static_cast<net_size_t>(left) : size;
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
void
stream_buffer::reset(void)
{
	m_pos = 0;
	m_next = m_head;
	m_out = m_reader;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char*
stream_buffer::next(net_size_t& size)
{
	net_size_t limit = m_limit > 0 ? m_limit : readable_size(0);
	if (limit <= m_pos)			// 正常不会出现此状况,应在调用next之前已检查可读总数
	{
		size = 0;
		return nullptr;
	}

	net_size_t left = limit - m_pos;
	if (size > left)			// 正常不会出现此状况,应在调用next之前已检查可读总数
	{
		size = left;
		return nullptr;
	}
	
	const char* p = m_out;

	limit = static_cast<net_size_t>(m_next->m_buffer + MAX_PACKET_LEN - m_out);
	limit = limit > left ? left : limit;
	size = (size == 0 || size > limit) ? limit : size;

	m_pos += size;
	m_out += size;
	if (m_out - m_next->m_buffer == MAX_PACKET_LEN)
	{
		m_next = m_next->m_next;
		m_out = m_next->m_buffer;
	}
	
	return p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility