#pragma once
#include <chrono>
#include <random>
#include <limits.h>

namespace Utility
{
	namespace com
	{
		template <typename T>
		class auto_free
		{
		public:
			auto_free(T* t) : m_t(t) {}
			~auto_free()
			{
				if (m_t != nullptr)
				{
					delete m_t;
					m_t = nullptr;
				}
			}
		private:
			T* m_t;
		};

		time_t time_seconds();
		time_t time_ms();
		time_t time_mms();

		//random
		class Random
		{
		public:
			Random()
			{
				m_gen = std::mt19937(m_rd());
			}
			~Random() {}

			static Random& Instance()
			{
				static Random instance;
				return instance;
			}

			int32_t rand()
			{
				std::uniform_int_distribution<> dis(0, INT_MAX);
				return dis(m_gen);
			}

			int32_t rand(int32_t min, int32_t max)
			{
				if (max <= min)
				{
					max = min;
				}
				std::uniform_int_distribution<> dis(min, max);
				return dis(m_gen);
			}

			template<typename T>
			T rand(T min, T max)
			{
				if (max <= min)
				{
					max = min;
				}
				std::uniform_real_distribution<T> dis(min, max);
				return dis(m_gen);
			}
		private:
			std::mt19937 m_gen;  // 利用梅森旋转伪随机数生成器
			std::random_device m_rd;  //get seed  
		};

		//	int32_t n1 = Random::Instance().rand(1, 10);
		//	cout << n1 << endl;
		//	float n2 = Random::Instance().rand<float>(1, 10);
		//	cout << n2 << endl;
		//	double n3 = Random::Instance().rand<double>(1, 10);
		//	cout << n3 << endl;
		//	return 0;
	}
}

