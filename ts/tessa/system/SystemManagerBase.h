#pragma once

#define TS_DECLARE_SYSTEM_MANAGER_TYPE(__class_name) \
	public: \
		static const char * name;

#define TS_DEFINE_SYSTEM_MANAGER_TYPE(__class_name) \
	namespace ts { const char * __class_name::name = #__class_name; }

TS_DECLARE1(system, Application);

TS_PACKAGE1(system)

class AbstractSystemManagerBase
{
public:
	AbstractSystemManagerBase() {}
	virtual ~AbstractSystemManagerBase() {}

	virtual bool initialize() = 0;
	virtual void deinitialize() = 0;
};

template<SizeType ManagerTypeIndex>
class SystemManagerBase : public AbstractSystemManagerBase
{
public:
	enum { TypeIndex = ManagerTypeIndex };

	SystemManagerBase(system::Application *application)
		: application(application)
	{
	}
	virtual ~SystemManagerBase() {}

protected:
	system::Application *application = nullptr;
};

TS_END_PACKAGE1()
