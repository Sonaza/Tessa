#pragma once

#include <vector>
#include <algorithm>

TS_PACKAGE1(util)

template<class DataType>
bool findIfContains(const std::vector<DataType> &container, const DataType &value)
{
	return std::find(container.begin(), container.end(), value) != container.end();
}

TS_END_PACKAGE1()
