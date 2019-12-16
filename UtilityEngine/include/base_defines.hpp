/**
* @file base_defines.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __BASE_DEFINES_HPP__
#define __BASE_DEFINES_HPP__

//#define NDEBUG

#include <assert.h>
#include <string.h>
#include <cstddef>
#define MAX_PACKET_LEN 4096

#define REDIS_CHECK_INTERVAL 500 // redis 连接检查间隔500ms

#ifdef _WIN32
using net_size_t = unsigned long;
#else
using net_size_t = std::size_t;
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum class errors{
	none,
	system,
	logic
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif //__BASE_DEFINES_HPP__