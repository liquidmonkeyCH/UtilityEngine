#include "UtilityTest.hpp"
#include "mem_data_factory.hpp"

using namespace Utility;
void UtilityTest::_DataFactory()
{
	{
		std::cout << std::endl
			<< "/////////////////////////////////////////////////////////////////////////"
			<< std::endl << "// mem::data_factory" << std::endl
			<< "/////////////////////////////////////////////////////////////////////////"
			<< std::endl;

		mem::data_factory<int, 100> _data_factory1;
		mem::data_factory<int> _data_factory2;
		_data_factory2.init(10);

		int* a = _data_factory1.malloc();
		int* b = _data_factory2.malloc();
		bool ret = false;
		//ret = _data_factory1.free(b);
		ret = _data_factory1.free(a);
		//ret = _data_factory1.free(a);
		ret = _data_factory2.free(b);

	}
}