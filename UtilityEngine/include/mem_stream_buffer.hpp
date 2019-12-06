/**
* @file mem_stream_buffer.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MEM_STREAM_BUFFER_HPP__
#define __MEM_STREAM_BUFFER_HPP__

#include "mem_buffer.hpp"
#include "mem_data_factory.hpp"
#include "mem_stream_node.hpp"
#include <mutex>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class stream_buffer : public buffer_iface
{
public:
	stream_buffer(void);
	virtual ~stream_buffer(void);

	//! Set size and initialize 
	void init(std::size_t size);
	//! Reset initialize 
	void clear(void);

	//! Returns: total writable size
	net_size_t writable_size(void);
	//! Reg write operation
	//! Param:(in-out)size [0~MAX_PACKET_LEN]
	char* write(net_size_t& size);
	//! Commit write operation
	//! Returns: |true [m_lastread = 0]|false [m_lastread != 0]|
	//!	Next:	 |Need notify readable |No need notify readable|
	bool commit_write(net_size_t size);

	//! Returns: total readable size, 0 when readable size less-than exp.
	//! Change member [m_lastread] to 0 when readable size less-than exp.
	//! Next: readable size less-than exp [m_lastread = 0]. Need wait for readable notify.
	net_size_t readable_size(net_size_t exp = 0);
	//! Reg read operation
	//! Param:(in-out)size [0~MAX_PACKET_LEN]
	//! Always change member [m_lastread]
	const char* read(net_size_t& size);
	//! Commit read operation
	//! Release space for write operation
	void commit_read(net_size_t size);

	//! message iface
	// Read a number of bytes.
	const char* next(net_size_t& size) override;
	// Skip a number of bytes.
	bool skip(net_size_t size) override;
	// Backs up a number of bytes.
	bool back_up(net_size_t size) override;

	// Reset total number of bytes read since this object was created to zero.
	void reset(void) override;
private:
	const char* _next(net_size_t& size, net_size_t limit);
public:
	using factory_t = mem::data_factory_ex<stream_node, 0, mem::factory_cache_type::DYNAMIC>;
private:
	stream_node*	m_head;
	stream_node*	m_tail;
	factory_t		m_factory;

	char*	m_reader;
	char*	m_writer;
	std::size_t	m_readable;

	std::size_t m_lastread;
#ifndef NDEBUG
	std::size_t m_last_malloc;
#endif

	std::mutex m_mutex;

	stream_node* m_next;
	net_size_t m_offset;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MEM_STREAM_BUFFER_HPP__