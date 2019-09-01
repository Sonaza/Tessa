
TS_PACKAGE2(app, viewer)

class FreeImageStaticInitializer
{
public:
	static FreeImageStaticInitializer &staticInitialize();

private:
	FreeImageStaticInitializer();
	~FreeImageStaticInitializer();
};

TS_END_PACKAGE2()
