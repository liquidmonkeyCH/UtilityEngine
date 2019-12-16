#include "com_util.hpp"

namespace Utility
{
	namespace com
	{
		time_t time_seconds()
		{
			auto now = std::chrono::system_clock::now();
			auto now_epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
			return (time_t)now_epoch.count();
		}
		time_t time_ms()
		{
			auto now = std::chrono::system_clock::now();
			auto now_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
			return (time_t)now_epoch.count();
		}
		time_t time_mms()
		{
			auto now = std::chrono::system_clock::now();
			auto now_epoch = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
			return (time_t)now_epoch.count();
		}
	}
}