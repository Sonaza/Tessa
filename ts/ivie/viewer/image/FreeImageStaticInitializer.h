
TS_PACKAGE2(app, viewer)

class FreeImageStaticInitializer
{
public:
	static FreeImageStaticInitializer &staticInitialize();

	static std::string &getLastErrorMessage();

private:
	FreeImageStaticInitializer();
	~FreeImageStaticInitializer();

	static std::string lastErrorMessage;
};

TS_END_PACKAGE2()
