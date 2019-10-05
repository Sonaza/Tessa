#pragma once

#include "ts/string/String.h"

TS_PACKAGE1(string)

class StringView
{
public:
	StringView(const char32_t *utf32ptr, BigSizeType size);
	StringView(const String &str);

	bool isEmpty() const;
	BigSizeType getSize() const;

	explicit operator String() const;

	const char32_t &operator[](BigSizeType index) const;

private:
	const char32_t *m_ptr = nullptr;
	BigSizeType m_size = 0;
};

TS_END_PACKAGE1()
