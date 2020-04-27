/**
* @file net_session.inl
*
* @author Hourui (liquidmonkey)
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
session_wrap<st, pares_message_wrap>::session_wrap(void)
{
	m_socket = &m_socket_impl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
session_wrap<st, pares_message_wrap>::~session_wrap(void)
{
	m_socket_impl.close();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::init_buffer(std::size_t recv_buffer_size, std::size_t send_buffer_size)
{
	m_recv_buffer.init(recv_buffer_size);
	m_send_buffer.init(send_buffer_size);

	m_recv_data.m_buffer.len = MAX_MSG_PACKET_LEN;
	m_recv_data.m_buffer.buf = m_recv_buffer.write(m_recv_data.m_buffer.len);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::clear(void)
{
	this->m_close_reason = reason::cs_none;

	m_recv_buffer.clear();
	m_send_buffer.clear();

	m_recv_data.m_buffer.len = MAX_MSG_PACKET_LEN;
	m_recv_data.m_buffer.buf = m_recv_buffer.write(m_recv_data.m_buffer.len);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::do_close(void)
{
	on_close(this->m_close_reason);
	m_parent->on_close_session(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
bool session_wrap<st, pares_message_wrap>::process_send(net_size_t size)
{
	if (m_state != static_cast<int>(state::connected))
		return false;

	m_send_buffer.commit_read(size);
	size = MAX_MSG_PACKET_LEN;
	const char* p = m_send_buffer.read(size);

	if (size == 0 || m_state != static_cast<int>(state::connected))
		return false;

	m_send_data.m_buffer.buf = const_cast<char*>(p);
	m_send_data.m_buffer.len = size;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::process_close(void)
{
	if(m_recv_buffer.go_bad())
		m_parent->post_request(this, &m_recv_buffer);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
bool session_wrap<st, pares_message_wrap>::process_recv(net_size_t size)
{
	if (m_state != static_cast<int>(state::connected))
		return false;

	if (m_recv_buffer.commit_recv(size))
		m_parent->post_request(this, &m_recv_buffer);

	m_recv_data.m_buffer.len = MAX_MSG_PACKET_LEN;
	m_recv_data.m_buffer.buf = m_recv_buffer.write(m_recv_data.m_buffer.len);

	if (m_recv_data.m_buffer.len == 0)
	{
		close(reason::cs_recv_buffer_overflow);
		return false;
	}

	return (m_state == static_cast<int>(state::connected));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
bool session_wrap<st, pares_message_wrap>::send_check(net_size_t size)
{
	if (m_state != static_cast<int>(state::connected))
		return false;

	m_send_mutex.lock();

	if (m_send_buffer.writable_size() < size)
	{
		// buffer overflow 
		close(reason::cs_send_buffer_overflow);
		m_send_mutex.unlock();
		return false;
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::post_send(bool flag)
{
	if (flag)
	{
		m_send_data.m_buffer.len = MAX_MSG_PACKET_LEN;
		m_send_data.m_buffer.buf = const_cast<char*>(m_send_buffer.read(m_send_data.m_buffer.len));

		if (m_state == static_cast<int>(state::connected))
			m_io_service->post_send_event(&m_send_data);
	}
	m_send_mutex.unlock();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
bool session_wrap<st, pares_message_wrap>::send(const char* packet, net_size_t size)
{
	if (!send_check(size)) 
		return false;

	char* p = nullptr;
	net_size_t left = size;
	bool b_send = false;
	do {
		size = left;
		p = m_send_buffer.write(size);
		memcpy(p, packet, size);
		b_send = m_send_buffer.commit_write(size);
		packet += size;
		left -= size;
	} while (left != 0);

	post_send(b_send);
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////