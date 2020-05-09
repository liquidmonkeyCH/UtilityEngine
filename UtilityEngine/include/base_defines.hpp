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

#ifdef _WIN32
using net_size_t = unsigned long;
#else
using net_size_t = std::size_t;
#endif
#endif //__BASE_DEFINES_HPP__