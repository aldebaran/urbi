// test.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"
#include <urbi/uobject.hh>

using namespace urbi;


class test : public UObject
{
	public:
	test(std::string s) : UObject(s){};
};


UStart(test);

