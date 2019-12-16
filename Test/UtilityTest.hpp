#ifndef __UTILITY_TEST_HPP__
#define __UTILITY_TEST_HPP__

#include <iostream>
#include "logger.hpp"

struct UtilityTest
{
	static void _Service();
	static void _Singleton();
	static void _ThreadPool();
	static void _Mysql();
	static void _DataFactory();
	static void _Protobuf();
	static void _Other();
	static void _Redis();
};

#endif//__UTILITY_TEST_HPP__