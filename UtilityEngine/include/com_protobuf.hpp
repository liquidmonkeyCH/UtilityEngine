/**
* @file com_protobuf.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_PROTOBUF_HPP__
#define __COM_PROTOBUF_HPP__

#include "google/protobuf/message.h"
#include "msg_message.hpp"
#include "net_session.hpp"

namespace Utility
{
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace _impl
{
///////////////////////////////////////////////////////////////////////////////////////////////////
class protobuf_ostream : public google::protobuf::io::ZeroCopyOutputStream
{
public:
	protobuf_ostream(mem::message* msg)
		:m_msg(dynamic_cast<mem::buffer_iface*>(msg)),
		m_total_size(0),
		last_returned_size(0),
		m_need_send(false) {}

	bool Next(void** data, int* size) override
	{
		net_size_t len = 0;
		if (last_returned_size > 0) m_need_send = m_msg->commit_write(last_returned_size);
		*data = m_msg->write(len);
		if (*data == nullptr) return false;
		last_returned_size = static_cast<int>(len);
		*size = last_returned_size;
		m_total_size += len;
		return true;
	}
	void BackUp(int count) override
	{
		assert(last_returned_size > 0);
		assert(count <= last_returned_size && count > 0);
		m_total_size -= count;
		m_need_send = m_msg->commit_write(last_returned_size - count);
		last_returned_size = 0;
	}

	google::protobuf::int64 ByteCount() const override { return m_total_size; }
	bool need_send(void) 
	{ 
		if (last_returned_size > 0) {
			m_need_send = m_msg->commit_write(last_returned_size);
			last_returned_size = 0;
		}
		return m_need_send; 
	}
private:
	mem::buffer_iface* m_msg;
	std::size_t m_total_size;
	int last_returned_size;
	bool m_need_send;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace _impl
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
///////////////////////////////////////////////////////////////////////////////////////////////////
class protobuf_istream : public google::protobuf::io::ZeroCopyInputStream
{
public:
	protobuf_istream(mem::message* msg) :m_msg(msg), last_returned_size(0) {}

	bool Next(const void** data, int* size) override
	{
		net_size_t len = 0;
		*data = m_msg->next(len);
		if (*data == nullptr) return false;
		last_returned_size = static_cast<int>(len);
		*size = last_returned_size;
		return true;
	}
	void BackUp(int count) override
	{
		assert(last_returned_size > 0);
		assert(count <= last_returned_size && count > 0);
		m_msg->back_up(count);
		last_returned_size = 0;
	}
	bool Skip(int count) override { last_returned_size = 0; return m_msg->skip(count); }
	google::protobuf::int64 ByteCount() const override { return m_msg->get_read_bytes(); }
private:
	mem::message* m_msg;
	int last_returned_size;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template<net::socket_type st, class pares_message_wrap>
class protobuf_session : public net::session_wrap<st, pares_message_wrap>
{
public:
	template<class T>
	bool send(const T& data)
	{
		static_assert(std::is_base_of<google::protobuf::Message, T>::value,"data mast be google::protobuf::Message");
		if (!this->send_check(data.ByteSizeLong()))
			return false;

		_impl::protobuf_ostream kStream(&this->m_send_buffer);
		data.SerializePartialToZeroCopyStream(&kStream);

		this->post_send(kStream.need_send());
		return true;
	}
	inline bool send(const char* package, size_t size)
	{
		return net::session_wrap<st, pares_message_wrap>::send(package, size);
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace com 
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_PROTOBUF_STREAM_HPP__

