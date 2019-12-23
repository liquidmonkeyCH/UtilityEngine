#include "UtilityTest.hpp"
#include "db_hiredis.hpp"

using namespace Utility;

enum class db::redis_dsn
{
	GAME_DB,
};

using DSN = db::redis_dsn;

void UtilityTest::_Redis()
{
	net::framework::net_init();

	std::cout << std::endl
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl << "// db_hiredis" << std::endl
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl;
	try
	{
		db::redis::init<DSN::GAME_DB>("192.168.60.178");
	}
	catch (utility_error & e)
	{
		Clog::error(e.what());
	}

	DATABASE_REDIS_BEGIN(DSN::GAME_DB)
		redisReply* res;
		DATABASE_REDIS_QUERY(res, "SET hourui 1")
		Clog::debug(res->str);
		DATABASE_REDIS_FREE_REPLY(res)
	DATABASE_REDIS_END()

	net::framework::net_free();
}