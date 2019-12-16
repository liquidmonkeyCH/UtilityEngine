
#include "db_redis.h"
#include "UtilityTest.hpp"
#include "com_util.hpp"

using namespace Utility;

void UtilityTest::_Redis()
{
	//random
	Clog::info("==============================================================");
	//Clog::info("random test: %d", com::Random::Instance().rand(0,10));
	//Clog::info("random test: %f", com::Random::Instance().rand<float>(0, 10.0));
	//Clog::info("random test: %f", com::Random::Instance().rand<double>(0, 10.00));

	db::db_redis redis;
	redis.init(5, "192.168.40.182", 6379);
	const char* key = "playerid";
	redis.set_key_val(key, "123456789");
	Clog::info("get key is %s", redis.get_val_by_key(key));
	if (redis.is_exist_key(key))
	{
		redis.del_key(key);
		Clog::info("del key ok %s", key);
	}
	
	const char* key2 = "playerids";
	redis.lpush_key_val(key2, "1111111111");
	redis.lpush_key_val(key2, "2222222222");
	redis.lpush_key_val(key2, "3333333333");
	Clog::info("rpop value is %s", redis.rpop_key(key2));
	Clog::info("rpop value is %s", redis.rpop_key(key2));
	Clog::info("rpop value is %s", redis.rpop_key(key2));
	Clog::info("==============================================================");
}
