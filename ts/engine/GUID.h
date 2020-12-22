#pragma once

TS_PACKAGE0()

class GUID
{
public:
	GUID(uint32_t guid);
	GUID(const String &str);
	GUID(const GUID &guid);

	uint32_t getUint32() const;
	const String getString() const;

	void offset(uint32_t value);

	static const GUID none;

private:
	uint32_t guid = 0xDEADBEEF;
};

struct GuidHash
{
	size_t operator()(const GUID &guidParam) const
	{
		return static_cast<size_t>(guidParam.getUint32());
	}
};

bool operator==(const GUID &a, const GUID &b);
bool operator!=(const GUID &a, const GUID &b);
bool operator<(const GUID &a, const GUID &b);

TS_END_PACKAGE0()
