/**
* @file mem_rotative_buffer.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MEM_ROTATIVE_BUFFER_HPP__
#define __MEM_ROTATIVE_BUFFER_HPP__

#include "mem_buffer.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<std::size_t block_size>
class rotative_buffer : public buffer_iface
{
public:
	rotative_buffer(void);
	virtual ~rotative_buffer(void);

	//! Set size and initialize 
	void init(std::size_t size);
	//! Reset initialize 
	void clear(void);

	//! Reg read operation
	//! Param:(in-out)size [0~PER_BLOCK_LEN]
	//! Always change member [m_lastread]
	const char* read(net_size_t& size);
	//! Commit read operation
	//! Release space for write operation
	void commit_read(net_size_t size);

	//! Reg write operation
	//! Param:(in-out)size [0~PER_BLOCK_LEN]
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

	static constexpr std::size_t max_message_len = block_size;
	static constexpr std::size_t pre_block_size = block_size;
private:
	char*	m_buffer;
	char*	m_reader;
	char*	m_writer;
	char*	m_final;
	std::size_t m_lastcopy;
	std::size_t m_lastread;
	std::size_t m_size;

#ifndef NDEBUG
	std::size_t m_last_malloc;
#endif
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mem_rotative_buffer.inl"
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MEM_ROTATIVE_BUFFER_HPP__