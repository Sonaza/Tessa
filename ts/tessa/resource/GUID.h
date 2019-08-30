#pragma once

TS_PACKAGE0()

class GUID
{
public:
	GUID(uint32 guid);
	GUID(const std::string &str);
	GUID(const GUID &guid);

	uint32 getUint32() const;
	const std::string getString() const;

	void offset(uint32 value);

	static const GUID none;

private:
	uint32 guid = 0xDEADBEEF;
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
