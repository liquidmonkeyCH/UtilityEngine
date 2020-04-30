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
namespace pares_zero
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
		net_size_t size = 0;
		const char* p = nullptr;
		net_size_t len,last_readable;

		do {
			if (!this->readable_state(size)) // 消息已无效
				return state::bad;

			if (size == 0)					// 消息长度不足
				return state::pending;

			last_readable = size + 1;

			do {
				len = 0;
				p = this->next(len);
				if (!p) break;
				size = static_cast<net_size_t>(strlen(p)) + 1;

				if (size <= len)
				{
					this->m_size += size;
					if (this->m_size > max_message_len)
						return state::error;

					this->set_read_limit(this->m_size);
					this->reset();

					return state::ok;
				}

				this->m_size += len;

				if (this->m_size > max_message_len)
					return state::error;

			} while (true);

			size = last_readable;

		} while (true);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace pares_zero
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_MESSAGE_HPP__
