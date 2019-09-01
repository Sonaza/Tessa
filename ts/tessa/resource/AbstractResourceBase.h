#pragma once

TS_PACKAGE1(resource)

class AbstractResourceBase
{
public:
	AbstractResourceBase(const String &filepath);
	virtual ~AbstractResourceBase();

	virtual bool loadResource() = 0;
	virtual void unloadResource() = 0;

protected:
	String getAbsolutePath();
	String filepath;

private:
	// Make resource non-copyable
	AbstractResourceBase(const AbstractResourceBase &) = delete;
};

TS_END_PACKAGE1()
