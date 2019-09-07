#pragma once

#include <list>
#include <algorithm>

TS_PACKAGE1(util)

template<class Type, class SortingPredicate = std::less<Type>>
class PriorityQueue
{
	typedef std::list<Type> Container;

public:
	typedef typename Container::iterator iterator;
	typedef typename Container::const_iterator const_iterator;

	PriorityQueue() = default;

	PriorityQueue(const PriorityQueue &) = delete;
	PriorityQueue &operator=(const PriorityQueue &) = delete;

	PriorityQueue(PriorityQueue &&) = delete;
	PriorityQueue &operator=(PriorityQueue &&) = delete;

	iterator begin() { return container.begin(); }
	iterator end() { return container.end(); }
	const_iterator begin() const { return container.begin(); }
	const_iterator end() const { return container.end(); }

	bool empty() const
	{
		return container.empty();
	}

	BigSizeType size() const
	{
		return container.size();
	}

	void push(const Type &value)
	{
		container.push_back(value);
		container.sort(sorting);
	}

	void push(Type &&value)
	{
		container.push_back(std::move(value));
		container.sort(sorting);
	}

	Type &top()
	{
		TS_ASSERT(!empty());
		return container.front();
	}

	const Type &top() const
	{
		TS_ASSERT(!empty());
		return container.front();
	}

	void pop()
	{
		TS_ASSERT(!empty());
		container.pop_front();
	}

	iterator find_if(std::function<bool(const Type &)> predicate)
	{
		for (iterator it = begin(); it != end(); ++it)
		{
			if (predicate(*it))
				return it;
		}
		return end();
	}

	iterator erase(iterator it)
	{
		return container.erase(it);
	}

	const_iterator erase(const_iterator it)
	{
		return container.erase(it);
	}

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
		container.clear();
	}

private:
	std::list<Type> container;
	SortingPredicate sorting;
};

TS_END_PACKAGE1()
