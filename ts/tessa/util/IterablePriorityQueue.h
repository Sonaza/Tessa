#pragma once

#include <queue>

TS_PACKAGE1(util)

template<class Type, class Container = std::vector<Type>>
class IterablePriorityQueue : public std::priority_queue<Type, Container>
{
public:
	typedef typename Container::iterator iterator;
	typedef typename Container::const_iterator const_iterator;

	iterator begin() { return this->c.begin(); }
	iterator end() { return this->c.end(); }
	const_iterator begin() const { return this->c.begin(); }
	const_iterator end() const { return this->c.end(); }

	iterator find_if(std::function<bool(const Type &)> predicate)
	{
		for (iterator it = begin(); it != end(); ++it)
		{
			if (predicate(*it))
				return it;
		}
		return end();
	}

	iterator erase(iterator it) { return this->c.erase(it); }
	bool erase_if(std::function<bool(const Type &)> predicate)
	{
		iterator it = find_if(predicate);
		if (it != end())
		{
			erase(it);
			return true;
		}
		return false;
	}

	void clear()
	{
		IterablePriorityQueue<Type, Container> empty;
		std::swap(*this, empty);
	}
};

TS_END_PACKAGE1()
