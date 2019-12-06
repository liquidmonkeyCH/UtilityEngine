/**
* @file mem_rotative_buffer.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "base_defines.hpp"
#include "mem_rotative_buffer.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
rotative_buffer::rotative_buffer(void)
: m_buffer(nullptr)
, m_reader(nullptr)
, m_writer(nullptr)
, m_final(nullptr)
, m_lastcopy(0)
, m_lastread(0)
, m_size(0)
#ifndef NDEBUG
, m_last_malloc(0)
#endif
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
rotative_buffer::~rotative_buffer(void)
{
	delete[] m_buffer;
	m_buffer = nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
rotative_buffer::clear(void)
{
	m_lastcopy = 0;
	m_lastread = 0;
	m_writer = m_buffer;
	m_reader = m_buffer;
	m_final = m_buffer + m_size;
#ifndef NDEBUG
	m_last_malloc = 0;
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
rotative_buffer::init(std::size_t size)
{
	assert(size >= MAX_PACKET_LEN);
	if (m_buffer != nullptr)
	{
		if (size == m_size)
			return;

		delete[] m_buffer;
	}

	m_buffer = new char[size + MAX_PACKET_LEN + 1];
	m_buffer[size + MAX_PACKET_LEN] = 0;
	m_size = size;
	clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
net_size_t
rotative_buffer::readable_size(net_size_t exp)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	//! ↓******↓*******************↓******↓**********↓
	//! head reader             writer  final       tail
	m_lastread = (m_writer >= m_reader) ? m_writer - m_reader : m_size - (m_reader - m_writer);
	if (m_lastread < exp) m_lastread = 0;
	return static_cast<net_size_t>(m_lastread);
	//!	|   len	  |                   |    read     |
	//! | copy |                      | left | copy |
	//! ↓*********↓*******************↓******↓**********↓
	//! head     writer            reader  final       tail
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char*
rotative_buffer::read(net_size_t& size)
{
	if (size > MAX_PACKET_LEN || size == 0) 
		size = MAX_PACKET_LEN;

	std::lock_guard<std::mutex> lock(m_mutex);
	//! ↓******↓*******************↓******↓**********↓
	//! head  reader			   writer  final     tail
	if (m_writer >= m_reader){
		m_lastread = m_writer - m_reader;
		(m_lastread > size) ? (m_lastread = size) : (size = static_cast<net_size_t>(m_lastread));
		return m_reader;
	}
	//!	|   len	  |                   |    read     |
	//! | copy |                      | left | copy |
	//! ↓*********↓*******************↓******↓**********↓
	//! head     writer            reader  final       tail
	std::size_t left = m_final - m_reader;
	m_lastread = left + m_lastcopy;
	if (size <= m_lastread){
		m_lastread = size;
		return m_reader;
	}
	
	left = (m_writer - m_buffer) + left;
	if (left < size)
	{
		size = static_cast<net_size_t>(m_lastread);
		return m_reader;
	}
		
	//! copy
	left = size - m_lastread;
	memcpy(m_final + m_lastcopy, m_buffer + m_lastcopy, left);
	m_lastcopy += left;
	m_lastread = size;

	return m_reader;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
rotative_buffer::commit_read(net_size_t size)
{
	assert(size <= m_lastread);
	std::lock_guard<std::mutex> lock(m_mutex);
	m_reader += size;
	//! reader touch finial
	//!	                        |   size  |
	//! ↓******↓*******************↓******↓**********↓
	//! head  writer			  final  reader     tail
	if (m_reader >= m_final) {
		m_reader = m_buffer + (m_reader - m_final);
		m_lastcopy = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
net_size_t
rotative_buffer::writable_size(void)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	//! ↓******↓*******************↓******↓**********↓
	//! head  writer            reader  final       tail
	return static_cast<net_size_t>((m_reader > m_writer) ? m_reader - m_writer - 1 : m_size - (m_writer - m_reader) - 1);
	//!	|   len	  |                   |    read     |
	//! | copy |                      | left | copy |
	//! ↓*********↓*******************↓******↓**********↓
	//! head    reader             writer  final       tail
}
////////////////////////////////////////////////////////////////////////////////////////////////////
char*
rotative_buffer::write(net_size_t& size)
{
	if (size > MAX_PACKET_LEN || size == 0)
		size = MAX_PACKET_LEN;

	std::lock_guard<std::mutex> lock(m_mutex);
	//!	|   len	  |                   |    size     |
	//! | copy |                      | left | copy |
	//! ↓*********↓*******************↓******↓**********↓
	//! head    reader             writer  final       tail
	std::size_t left = (m_writer >= m_reader) ? m_size - (m_writer - m_reader) : m_reader - m_writer;
	(--left > size) ? (left = size) : (size = static_cast<net_size_t>(left));
#ifndef NDEBUG
	m_last_malloc = size;
#endif
	return m_writer;
	//!
	//! ↓*********↓**********↓******↓*********↓
	//! head   writer      reader final      tail
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
rotative_buffer::commit_write(net_size_t size)
{
	std::lock_guard<std::mutex> lock(m_mutex);
#ifndef NDEBUG
	assert(m_last_malloc >= size);
	m_last_malloc = 0;
#endif
	m_writer += size;

	//! writer touch finial
	//!	                          |  size  |
	//! ↓*********↓*******************↓****↓********↓
	//! head    reader              final writer    tail
	if (m_writer >= m_final) {
		m_lastcopy = m_writer - m_final;
		m_writer = m_buffer + m_lastcopy;

		//! copy
		memcpy(m_buffer, m_final, m_lastcopy);
	}

	return (m_lastread == 0); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// **** message iface
////////////////////////////////////////////////////////////////////////////////////////////////////
const char*
rotative_buffer::next(net_size_t& size)
{
	net_size_t limit = m_limit > 0 ? m_limit : readable_size(0);
	assert(m_limit <= limit);
	if (limit <= m_position){
		size = 0;
		return nullptr;
	}

	net_size_t left = limit - m_position;
	if (size > left) {					// 未达到预期期望读取数量
		size = left;
		return nullptr;
	}
	else if (size == 0)
		size = left;

	left = m_position;
	m_position += size;
	return read(limit) + left;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
rotative_buffer::skip(net_size_t size)
{
	net_size_t limit = m_limit > 0 ? m_limit : readable_size(0);
	assert(m_limit <= limit);

	if (size > limit - m_position)
		return false;

	do {
		limit = size;
		next(limit);
		size -= limit;
	} while (size > 0);

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
rotative_buffer::back_up(net_size_t size)
{
	if (size > m_position)
		return false;

	m_position -= size;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 