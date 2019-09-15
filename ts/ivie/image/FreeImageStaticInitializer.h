
TS_PACKAGE2(app, image)

class FreeImageStaticInitializer
{
public:
	static FreeImageStaticInitializer &staticInitialize();

private:
	FreeImageStaticInitializer();
	~FreeImageStaticInitializer();
};

TS_END_PACKAGE2()
