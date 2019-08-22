#pragma once

#include <vector>

TS_PACKAGE2(app, util)

template<class Type, BigSizeType capacity>
class RingBuffer : public lang::Noncopyable
{
public:
	RingBuffer();
	~RingBuffer();

	RingBuffer(RingBuffer &&other);
	RingBuffer &operator=(RingBuffer &&other);

	void reserveFullCapacity();
	void clear();

	bool isEmpty() const;
	bool isFull() const;

	BigSizeType getBufferedAmount() const;
	BigSizeType getFreeAmount() const;

	BigSizeType getSize() const;
	BigSizeType getMaxSize() const;

	// If buffer is not full, appends a new entry to the current
	// write position and advances the write pointer by one.
	// Overwrites oldest data or allocates memory if buffer isn't at full capacity.
	// Returns false if not write was not possible (check isFull before to avoid)
	bool pushWrite(Type &&value);
	bool pushWrite(const Type &value);
	// Returns reference to current element at write position. Allocates memory if write position is not yet allocated.
	Type &getWritePtr();
	// Returns if write pointer can be incremented (only possible when buffer is not full)
	bool canIncrementWrite() const;
	// Increments write pointer if possible (when buffer is not full)
	bool incrementWrite();

	// Returns reference to current element at read position. Galaxies explode if buffer is empty.
	Type &getReadPtr();
	const Type &getReadPtr() const;
	// Returns if read pointer can be incremented (only possible when buffer has at least 2 elements buffered)
	bool canIncrementRead() const;
	// Increments write pointer if possible (when buffer has at least 2 elements buffered)
	bool incrementRead();
	// Removes read constraint, the requirement that write pointer is always ahead of read.
	// Useful when no more data will be written and existing data will be continuously looped through.
	void removeReadConstraint();

	const BigSizeType getWritePositionIndex() const { return writePosition; }
	const BigSizeType getReadPositionIndex() const { return readPosition; }


	Type &operator[](BigSizeType index) { return buffer[index]; }
	const Type &operator[](BigSizeType index) const { return buffer[index]; }

private:
	BigSizeType writePosition = 0;
	BigSizeType readPosition = 0;
	BigSizeType numBuffered = 0;
	bool constrained = true;
	std::vector<Type> buffer;

	enum { ReadReservedAmount = 1 };
};

template<class Type, BigSizeType capacity>
void RingBuffer<Type, capacity>::reserveFullCapacity()
{
	if (buffer.size() < capacity)
		buffer.resize(capacity);
}

template<class Type, BigSizeType capacity>
void ts::app::util::RingBuffer<Type, capacity>::clear()
{
	writePosition = 0;
	readPosition = 0;
	numBuffered = 0;
	constrained = true;
	buffer.clear();
}

template<class Type, BigSizeType capacity>
RingBuffer<Type, capacity>::RingBuffer()
{

}

template<class Type, BigSizeType capacity>
RingBuffer<Type, capacity>::~RingBuffer()
{

}

template<class Type, BigSizeType capacity>
RingBuffer<Type, capacity>::RingBuffer(RingBuffer &&other)
{
	*this = other;
}

template<class Type, BigSizeType capacity>
RingBuffer<Type, capacity> &RingBuffer<Type, capacity>::operator=(RingBuffer &&other)
{
	if (this != &other)
	{
		writePosition = other.writePosition;
		other.writePosition = 0;

		readPosition = other.readPosition;
		other.readPosition = 0;

		numBuffered = other.numBuffered;
		other.numBuffered = 0;

		constrained = other.constrained;
		other.constrained = true;

		buffer = std::move(other.buffer);
	}
	return *this;
}

template<class Type, BigSizeType capacity>
TS_FORCEINLINE BigSizeType RingBuffer<Type, capacity>::getBufferedAmount() const
{
	return constrained ? numBuffered : getSize();
}

template<class Type, BigSizeType capacity>
TS_FORCEINLINE BigSizeType RingBuffer<Type, capacity>::getFreeAmount() const
{
	TS_ASSERT(capacity >= numBuffered);
	return constrained ? capacity - numBuffered : 0;
}

template<class Type, BigSizeType capacity>
TS_FORCEINLINE bool RingBuffer<Type, capacity>::isEmpty() const
{
	return numBuffered == 0;
}

template<class Type, BigSizeType capacity>
TS_FORCEINLINE bool RingBuffer<Type, capacity>::isFull() const
{
	return numBuffered == capacity;
}

template<class Type, BigSizeType capacity>
TS_FORCEINLINE BigSizeType RingBuffer<Type, capacity>::getSize() const
{
	return buffer.size();
}

template<class Type, BigSizeType capacity>
TS_FORCEINLINE BigSizeType RingBuffer<Type, capacity>::getMaxSize() const
{
	return capacity;
}

template<class Type, BigSizeType capacity>
bool RingBuffer<Type, capacity>::pushWrite(Type &&value)
{
	TS_ASSERT(constrained == true && "Read constraint has been removed and no more writes are expected.");

	if (numBuffered < capacity)
	{
		if (buffer.size() < capacity)
			buffer.push_back(std::move(value));
		else
			buffer[writePosition] = std::move(value);

		numBuffered++;
		TS_ASSERT(numBuffered <= capacity);
		writePosition = (writePosition + 1) % getSize();
		return true;
	}
	return false;
}

template<class Type, BigSizeType capacity>
bool RingBuffer<Type, capacity>::pushWrite(const Type &value)
{
	TS_ASSERT(constrained == true && "Read constraint has been removed and no more writes are expected.");

	if (numBuffered < capacity)
	{
		if (buffer.size() < capacity)
			buffer.push_back(value);
		else
			buffer[writePosition] = value;

		numBuffered++;
		TS_ASSERT(numBuffered <= capacity);
		writePosition = (writePosition + 1) % getSize();
		return true;
	}
	return false;
}

template<class Type, BigSizeType capacity>
Type &RingBuffer<Type, capacity>::getWritePtr()
{
	TS_ASSERT(constrained == true && "Read constraint has been removed and no more writes are expected.");

	if (writePosition == buffer.size())
		buffer.push_back(Type());
	
	return buffer[writePosition];
}

template<class Type, BigSizeType capacity>
bool RingBuffer<Type, capacity>::canIncrementWrite() const
{
	return !isFull() && constrained;
}

template<class Type, BigSizeType capacity>
bool RingBuffer<Type, capacity>::incrementWrite()
{
	if (canIncrementWrite())
	{
		numBuffered++;
		TS_ASSERT(numBuffered <= capacity);
		writePosition = (writePosition + 1) % math::min(capacity, getSize() + 1);
	}
	return false;
}

template<class Type, BigSizeType capacity>
TS_FORCEINLINE Type &RingBuffer<Type, capacity>::getReadPtr()
{
	TS_ASSERT(numBuffered > 0 && "Reading buffer when it is empty.");
	return buffer[readPosition];
}

template<class Type, BigSizeType capacity>
TS_FORCEINLINE const Type &RingBuffer<Type, capacity>::getReadPtr() const
{
	TS_ASSERT(numBuffered > 0 && "Reading buffer when it is empty.");
	return buffer[readPosition];
}

template<class Type, BigSizeType capacity>
TS_FORCEINLINE bool RingBuffer<Type, capacity>::canIncrementRead() const
{
	return constrained ? numBuffered > ReadReservedAmount : true;
}

template<class Type, BigSizeType capacity>
bool RingBuffer<Type, capacity>::incrementRead()
{
	if (canIncrementRead())
	{
		if (constrained)
			numBuffered--;

		readPosition = (readPosition + 1) % getSize();
		return true;
	}
	return false;
}

template<class Type, BigSizeType capacity>
void RingBuffer<Type, capacity>::removeReadConstraint()
{
	constrained = false;
}

TS_END_PACKAGE2()
