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

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<std::size_t block_size>
class stream_buffer : public mem::buffer_iface
{
public:
	stream_buffer(void);
	virtual ~stream_buffer(void);

	//! Set size and initialize 
	void init(std::size_t size);
	//! Reset initialize 
	void clear(void);

	//! Reg read operation
	//! Param:(in-out)size [0~block_size]
	//! Always change member [m_lastread]
	const char* read(net_size_t& size);
	//! Commit read operation
	//! Release space for write operation
	void commit_read(net_size_t size);

	//! Reg write operation
	//! Param:(in-out)size [0~block_size]
	char* write(net_size_t& size);
	//! Returns: total writable size
	net_size_t writable_size(void);
protected:
	//! Returns: total readable size, 0 when readable size less-than exp.
	//! Change member [m_lastread] to 0 when readable size less-than exp.
	//! Next: readable size less-than exp [m_lastread = 0]. Need wait for readable notify.
	net_size_t _readable_size(net_size_t exp);
	//! Commit write operation
	//! Returns: |true [m_lastread = 0]|false [m_lastread != 0]|
	//!	Next:	 |Need notify readable |No need notify readable|
	bool _commit_write(net_size_t size);
public:
	//! message iface
	// Read a number of bytes.
	const char* next(net_size_t& size) override;
	// Skip a number of bytes.
	bool skip(net_size_t size) override;
	// Backs up a number of bytes.
	bool back_up(net_size_t size) override;

	// Reset total number of bytes readed to zero since this object was created .
	void reset(void) override;
private:
	const char* _next(net_size_t& size, net_size_t limit);
public:
	using stream_node_t = stream_node<block_size>;
	using factory_t = mem::data_factory_ex<stream_node_t, 0, mem::factory_cache_type::DYNAMIC>;
	static constexpr std::size_t max_message_len = ULONG_MAX;
	static constexpr std::size_t pre_block_size = block_size;
private:
	stream_node_t*	m_head;
	stream_node_t*	m_tail;
	factory_t		m_factory;

	char*	m_reader;
	char*	m_writer;
	std::size_t	m_readable;

	std::size_t m_lastread;
#ifndef NDEBUG
	std::size_t m_last_malloc;
#endif
	stream_node_t* m_next;
	net_size_t m_offset;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mem_stream_buffer.inl"
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MEM_STREAM_BUFFER_HPP__