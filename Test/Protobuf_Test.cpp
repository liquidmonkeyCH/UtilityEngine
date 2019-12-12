#include "UtilityTest.hpp"
#include "mem_stream_buffer.hpp"
#include "com_protobuf.hpp"
#include "test.pb.h"

using namespace Utility;
void UtilityTest::_Protobuf()
{
	std::cout << std::endl
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl << "// com::protobuf" << std::endl
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl;

	haha::test kTestOut;
	kTestOut.set_id(10);
	kTestOut.add_num(100);
	kTestOut.add_num(200);

	Clog::info("kTestOut:%s",kTestOut.DebugString().c_str());

	mem::stream_buffer<2> kbuffer;
	kbuffer.init(2 * 100);
	// 不要直接使用_impl::protobuf_ostream，除非你知道要做什么
	// 继承com::protobuf_session && send(proto)
	// com::protobuf_session会调用_impl::protobuf_ostream去发送协议
	_impl::protobuf_ostream kOutStream(&kbuffer); 
	kTestOut.SerializeToZeroCopyStream(&kOutStream);
	kOutStream.need_send();	// 这个必须调用，不然可能会丢失数据

	com::protobuf_istream kInStream(&kbuffer);
	haha::test kTestIn;
	kTestIn.ParseFromZeroCopyStream(&kInStream);

	Clog::info("kTestIn:%s", kTestIn.DebugString().c_str());
}