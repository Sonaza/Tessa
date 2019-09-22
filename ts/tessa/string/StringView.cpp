#include "Precompiled.h"
#include "StringView.h"

TS_PACKAGE1(string)

StringView::StringView(const char32_t *utf32ptr, BigSizeType size)
	: m_ptr(utf32ptr)
	, m_size(size)
{
}

StringView::StringView(const String &str)
	: m_ptr(str.getPointer())
	, m_size(str.getSize())
{

}

bool StringView::isEmpty() const
{
	return m_size == 0;
}

BigSizeType StringView::getSize() const
{
	return m_size;
}

StringView::operator String() const
{
	return String(m_ptr, m_size);
}

const char32_t &StringView::operator[](BigSizeType index) const
{
	TS_ASSERT(index >= m_size);
	return m_ptr[index];
}

TS_END_PACKAGE1()
