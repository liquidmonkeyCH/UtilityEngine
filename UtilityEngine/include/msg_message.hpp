/**
* @file msg_message.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_MESSAGE_HPP__
#define __MSG_MESSAGE_HPP__

#include "msg_defines.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum class state { ok, pending, error, bad };
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class buffer_type, net_size_t MAX_MSG_LEN>
class _message_impl : public buffer_type
{
public:
	_message_impl(void) :m_size(0), m_good(true)
	{
		static_assert(MAX_MSG_LEN > 0, "MAX_MSG_LEN out of range!");
		static_assert(MAX_MSG_LEN <= buffer_type::MAX_MESSAGE_LEN, "MAX_MSG_LEN out of bound!");
	}
	virtual ~_message_impl(void) = default;

	void commit(void)
	{
		this->commit_read(m_size);
		m_size = 0;
		this->reset();
		this->set_read_limit(0);
	}

	void clear(void)
	{
		m_size = 0;
		this->reset();
		this->set_read_limit(0);
		buffer_type::clear();
		m_good = true;
	}

	bool go_bad(void)
	{
		std::lock_guard<std::mutex> lock(this->m_mutex);
		m_good = false;
		return this->buffer_type::_commit_write(0);
	}

	bool is_good(void)
	{
		std::lock_guard<std::mutex> lock(this->m_mutex);
		return m_good;
	}

	bool commit_recv(net_size_t size)
	{
		std::lock_guard<std::mutex> lock(this->m_mutex);
		if (!m_good) return false;
		return this->buffer_type::_commit_write(size);
	}
protected:
	bool readable_state(net_size_t& exp)
	{
		std::lock_guard<std::mutex> lock(this->m_mutex);
		if (!m_good) return false;
		exp = this->buffer_type::_readable_size(exp);
		return true;
	}
protected:
	std::uint32_t m_size;
	bool m_good;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_MESSAGE_HPP__