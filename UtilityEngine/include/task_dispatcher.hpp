/**
* @file task_dispatcher.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __TASK_DISPATCHER_HPP__
#define __TASK_DISPATCHER_HPP__

#include "msg_controler.hpp"
#include "com_thread_pool.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class dispatcher
{
public:
	struct task_info
	{
		msg::controler_iface* m_controler;
		task::object_iface* m_obj;
		std::uint32_t				m_compkey;
		mem::message* m_message;
		void* m_userdata;

		void run(void)
		{
			m_controler->handle_wrap(m_obj, m_compkey, m_message, m_userdata);
		}
	};
public:
	dispatcher(void);
	~dispatcher(void);

	dispatcher(const dispatcher&) = delete;
	dispatcher& operator=(const dispatcher&) = delete;

	void start(std::uint32_t nworker);
	void stop(void);
	void dispatch(task_info&& _task);
private:
	com::task_threadpool<task_info> m_workers;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_DISPATCHER_HPP__