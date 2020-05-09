
#include "logger.hpp"

#include <iostream>
#include <sstream>
#include <random>
#include <memory>

#include "net_io_service_iocp.hpp"
#include "net_requester.hpp"

#include "mem_rotative_buffer.hpp"
#include "mem_stream_buffer.hpp"
#include "msg_pares_zero.hpp"

#include "msg_handler_manager_map.hpp"
#include "msg_handler_manager_deque.hpp"
#include "msg_dispatcher.hpp"

#include "com_service_manager.hpp"
#include "com_protobuf.hpp"

#define MAX_PACKET_LEN 4096
using namespace Utility;

class GameSession : public net::session_wrap < net::socket_type::tcp, msg::pares_zero::comfirmer<mem::stream_buffer<MAX_PACKET_LEN>, MAX_PACKET_LEN> >
{
public:
	void on_connect(void)
	{
		m_socket->set_recv_buffer(8);
	}
};

int handler(msg::object_iface* obj, mem::message* _msg)
{
	char buffer[MAX_PACKET_LEN + 1];
	GameSession* session = dynamic_cast<GameSession*>(obj);

	net_size_t len = 0, size = 0;
	while (true) {
		const char* p = _msg->next(len);
		if (!p) break;

		memcpy(buffer + size, p, len);
		size += len;
		len = 0;
	}

	Clog::info("recv msg: %s", buffer);
	session->send(buffer, size);

	return 0;
}

class NetClient : public net::requester <GameSession, msg::handler_manager_deque>
{
public:
	void on_start(void)
	{
		m_recv_buffer_size = 10 * MAX_PACKET_LEN;
		m_send_buffer_size = 10 * MAX_PACKET_LEN;
		m_controler.attach(handler);
	}

	msg::channel m_channel;
};

int main(void)
{
	logger klogger(logger::log_level::debug);
	Clog::active_logger(&klogger);

	net::framework::net_init();

	net::io_service_iocp io_service;
	io_service.start();

	NetClient::dispatcher_t dispatcher;
	dispatcher.start(10);

	std::string host;
	std::cout << "Enter host ip:";
	std::cin >> host;
	std::cout << "Entet Client num:";
	int nCount;
	std::cin >> nCount;

	std::list<std::shared_ptr<NetClient>> m_list;

	for (int i = 0; i < nCount; ++i)
	{
		std::shared_ptr<NetClient> client(new NetClient);
		client->init(&io_service, &dispatcher);
		if (client->start(host.c_str(), 55552) == net::requester_iface::state::timeout)
		{
			Clog::debug("connect timeout!");
			continue;
		}

		m_list.push_back(client);
	}
	
	std::string ip;
	std::stringstream str;
	do
	{
	
		std::cin >> ip;
		if (ip.compare("quit") == 0) break;
		int n = 0;

		for (std::list<std::shared_ptr<NetClient>>::iterator it = m_list.begin(); it != m_list.end();)
		{
			if (!(*it)->is_connected())
			{
				m_list.erase(it++);
				continue;
			}
				
			str << ip << " client:" << n++;
			(*it)->send(str.str().c_str(), str.str().length()+1);
			
			str.str("");
			++it;
		}
		Clog::debug("left client: %u", m_list.size());
	} while (true);


	for (std::shared_ptr<NetClient> client : m_list)
	{
		client->stop();
	}

	for (std::shared_ptr<NetClient> client : m_list)
	{
		client->join();
	}

	m_list.clear();

	io_service.stop();
	dispatcher.stop();

	net::framework::net_free();

	system("pause");

	return 0;
}