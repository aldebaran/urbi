#include <urbi/uobject.hh>

using namespace urbi;

class test : public UObject
{
	public:

	test(std::string);
};

test::test(std::string s) : UObject(s) {}

UStart(test);

