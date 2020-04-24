/**
* @file task_dispatcher.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_DISPATCHER_HPP__
#define __MSG_DISPATCHER_HPP__

#include "com_thread_pool.hpp"
#include "msg_object.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class controler_iface;
////////////////////////////////////////////////////////////////////////////////////////////////////
class dispatcher
{
public:
	struct task_info
	{
		controler_iface* m_controler;
		object_iface* m_obj;
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
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_DISPATCHER_HPP__