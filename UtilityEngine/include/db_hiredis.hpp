/**
* @file db_mysql.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __DB_MYSQL_HPP__
#define __DB_MYSQL_HPP__

#include "logger.hpp"
#include "com_singleton.hpp"
#include <cstddef>
#include <mutex>
#include <assert.h>
#include <deque>
#include <hiredis/hiredis.h>
#include "net_framework.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace db
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum class redis_dsn;
enum class redis_errors {
	unknown_dsn,
	duplicate_dsn,
	bad_params,
	connect_fail,
};
class redis;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace _redis_impl
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<redis_dsn dsn>
class connection_pool : public com::iface::Singleton<connection_pool<dsn>>
{
public:
	connection_pool() :m_port(0),m_bInit(false) {}
	~connection_pool() { clear(); }
	friend class db::redis;
	template<redis_dsn> friend struct auto_connect;
protected:
	void init(const char* server, unsigned int port, std::size_t size)
	{
		assert(size > 0);
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_bInit)
			Clog::throw_error(redis_errors::duplicate_dsn, "hiredis: init dsn(%d) duplicate!", dsn);
		
		m_bInit = true;
		m_server = server;
		m_port = port;
		redisContext* redis_ctx = nullptr;
		timeval tv = { 1,1500 };
		for (std::size_t i = 0; i < size; ++i)
		{
			
			redis_ctx = redisConnectWithTimeout(m_server.c_str(), m_port, tv);
			if (redis_ctx == nullptr)
				Clog::error_throw(redis_errors::connect_fail, "hiredis: connect redis fail!");
			
			m_pool.emplace_back(redis_ctx);
			redisEnableKeepAlive(redis_ctx);
		}
	}

	redisContext* malloc(void)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if(!m_bInit)
			Clog::error_throw(redis_errors::unknown_dsn, "hiredis: unknown_dsn(%d) or not init!", dsn);

		redisContext* redis_ctx = nullptr;
		if (m_pool.empty())
		{
			timeval tv = { 1,1500 };
			redis_ctx = redisConnectWithTimeout(m_server.c_str(), m_port, tv);
			if (redis_ctx == nullptr)
				Clog::error_throw(redis_errors::connect_fail, "hiredis: connect redis fail!");

			redisEnableKeepAlive(redis_ctx);
			return redis_ctx;
		}

		redis_ctx = m_pool.front();
		m_pool.pop_front();

		redisReply* preply = reinterpret_cast<redisReply*>(redisCommand(redis_ctx, "PING"));
		if (preply == nullptr || preply->type == REDIS_REPLY_ERROR || strcmp(preply->str, "PONG") != 0)
		{
			freeReplyObject(preply);
			if (redisReconnect(redis_ctx) == REDIS_OK)
			{
				redisEnableKeepAlive(redis_ctx);
				return redis_ctx;
			}
				
			Clog::error_throw(redis_errors::connect_fail, "hiredis: reconnect redis fail!");
		}

		freeReplyObject(preply);
		return redis_ctx;
	}

	void free(redisContext* redis_ctx)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_pool.emplace_back(redis_ctx);
	}

	void clear(void)
	{
		for (auto it = m_pool.begin(); it != m_pool.end(); ++it)
			redisFree(*it);

		m_pool.clear();
	}
private:
	std::mutex m_mutex;
	std::string m_server;
	unsigned int m_port;
	bool m_bInit;
	std::deque<redisContext*> m_pool;
};

template<redis_dsn dsn>
struct auto_connect
{
public:
	auto_connect(redisContext* connect) :m_connect(connect) {}
	~auto_connect(void) { if (m_connect) connection_pool<dsn>::GetInstance()->free(m_connect); }
	redisContext* get() { return m_connect; }
private:
	redisContext*  m_connect;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace _redis_impl
////////////////////////////////////////////////////////////////////////////////////////////////////
class redis
{
private:
	redis(void) = default;
	~redis(void) = default;

	redis(const redis&) = delete;
	redis& operator=(const redis&) = delete;
	redis(const redis&&) = delete;
	redis& operator=(const redis&&) = delete;
public:
	template<redis_dsn dsn>
	static void init(const char* server,unsigned int port = 6379,std::size_t size = 5)
	{
		if (!server)
			Clog::throw_error(redis_errors::bad_params, "hiredis: init dsn(%d) params error!", dsn);

		_redis_impl::connection_pool<dsn>::GetInstance()->init(server, port, size);
	}

	template<redis_dsn dsn>
	static _redis_impl::auto_connect<dsn> get_connection(void)
	{
		return { _redis_impl::connection_pool<dsn>::GetInstance()->malloc() };
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace db
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace Utility
////////////////////////////////////////////////////////////////////////////////////////////////////
#define DATABASE_REDIS_BEGIN(DSN)																	\
{																									\
	try																								\
	{																								\
		auto utility_db_redis_connection = Utility::db::redis::get_connection<DSN>();				\

#define DATABASE_REDIS_END()																		\
	}																								\
	catch(std::exception& e)																		\
	{																								\
		Utility::Clog::error(e.what());																\
	}																								\
}

#define DATABASE_REDIS_QUERY(res,sql)																\
		res = reinterpret_cast<redisReply*>(redisCommand(utility_db_redis_connection.get(),sql));

#define DATABASE_REDIS_FREE_REPLY(res)																\
		freeReplyObject(res);
///////////////////////////////////////////////////////////////////////////////////////////////////
#endif // __DB_MYSQL_HPP__