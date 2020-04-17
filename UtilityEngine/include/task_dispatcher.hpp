/**
* @file task_dispatcher.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __TASK_DISPATCHER_HPP__
#define __TASK_DISPATCHER_HPP__

#include "com_thread_pool.hpp"
#include "task_object.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg { class controler_iface; }
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
		mem::message* m_message;
		void* m_userdata;
		bool m_good;

		void run(void);
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