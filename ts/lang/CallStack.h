#pragma once

#include <string>

TS_PACKAGE1(lang)

class CallStack
{
	CallStack() = delete;
public:
	static void printCallstack();

	static std::vector<std::string> getCallstack();
	static std::string getCallstackAsString();

};

TS_END_PACKAGE1()
