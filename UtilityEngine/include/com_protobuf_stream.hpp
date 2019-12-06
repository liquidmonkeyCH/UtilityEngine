/**
* @file com_protobuf_stream.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_PROTOBUF_STREAM_HPP__
#define __COM_PROTOBUF_STREAM_HPP__

#include "msg_message.hpp"
#include "google/protobuf/io/zero_copy_stream.h"

namespace Utility
{
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
///////////////////////////////////////////////////////////////////////////////////////////////////
class protocol_stream : public google::protobuf::io::ZeroCopyInputStream
{
public:
	protocol_stream(mem::message* msg) :m_msg(msg), last_returned_size(0) {}

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
}// namespace com 
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_PROTOBUF_STREAM_HPP__

