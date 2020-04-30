/**
* @file msg_pares_len.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_PARES_LEN_HPP__
#define __MSG_PARES_LEN_HPP__

#include "msg_message.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace pares_len
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class buffer_type, net_size_t max_message_len>
class comfirmer : public _message_impl<buffer_type, max_message_len>
{
public:
	using buffer_t = buffer_type;

	comfirmer(void)
	{
		static_assert(max_message_len > sizeof(std::uint32_t), "max_message_len out of range!");
	}
	virtual ~comfirmer(void) = default;
	template<class message_wrap, class handler_manager> friend class controler_wrap;
private:
	state comfirm(void)
	{
		net_size_t size = this->m_size;
		if (size == 0)							// 未获取消息长度
		{
			size = sizeof(std::uint32_t);
			if (!this->readable_state(size))	// 消息已无效
				return state::bad;

			if (size == 0)						// 消息长度不足
				return state::pending;

			size = sizeof(std::uint32_t);
			net_size_t pos = 0;
			const char* p;
			do {
				p = this->next(size);
				memcpy((char*)(&this->m_size) + pos, p, size);
				pos += size;
				if (pos >= sizeof(std::uint32_t)) break;
				size = sizeof(std::uint32_t) - pos;
			} while (true);

			if (this->m_size < sizeof(std::uint32_t) || this->m_size > max_message_len)
				return state::error;

			size = this->m_size;
		}

		if (!this->readable_state(size))	// 消息已无效
			return state::bad;

		//  last_read == 0
		if (size == 0)						// 消息长度不足
			return state::pending;

		this->set_read_limit(this->m_size);	// 消息完整 
		return state::ok;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace pares_len
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_MESSAGE_HPP__
