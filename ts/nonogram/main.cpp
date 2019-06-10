#include "Precompiled.h"
#include "GameApplication.h"

#include "ts/tessa/system/ThreadPool.h"

#include <thread>

int test2()
{
	ts::SharedPointer<int> ptr(new int(100));

	std::vector<std::thread> threads;
	for (int i = 0; i < 10; ++i)
	{
		threads.push_back(std::thread([i](ts::SharedPointer<int> ptr)
		{
			TS_PRINTF("[#%d] Thread online\n", i);
			while (true)
			{
				if (*ptr > 0)
				{
					(*ptr)--;
					TS_PRINTF("[#%d] Value is now %d\n", i, *ptr);
				}
				else
				{
					TS_PRINTF("[#%d] Oh time to leave\n", i);
					break;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}, ptr));
	}

	for (std::thread &t : threads)
	{
		t.join();
	}

	TS_PRINTF("Finished! Value is now %d\n", *ptr);

	ptr.reset();

	return 0;
}

class Base
{
public:
	virtual void doThing() = 0;
};

class Derived : public Base
{
public:
	virtual void doThing()
	{
		TS_PRINTF("Doing a thing! Number %d\n", number);
	}
	ts::Int32 number = 1337;
};

class Poopoo
{
public:
	virtual void doSomtinElse()
	{
		TS_PRINTF("Doing a somtin else! Number %d\n", number);
	}
	ts::Int32 number;
};

class CustomDeleter
{
public:
	void operator()(void *ptr)
	{
		TS_PRINTF("Using custom deleter!\n");
		free(ptr);
	}
};

int test()
{
	int *customRawPtr = (int*)malloc(sizeof(int));
	*customRawPtr = 1234512345;
	ts::UniquePointer<int, CustomDeleter> customAD(customRawPtr);

	customAD.reset();

// 	make_shared

	// std::shared_ptr<int> potato(new int(1234));

	ts::ScopedPointer<std::string> strptr(new std::string("Potato text"));

	TS_PRINTF("String! %s\n", *strptr);

	ts::SharedPointer<Base> basePtr(new Derived, CustomDeleter());
	ts::SharedPointer<Derived> derivedPtr = ts::staticPointerCast<Derived>(basePtr);

	std::unique_ptr<int> stdunique = std::make_unique<int>(1241325);
	TS_VERIFY_POINTERS_WITH_RETURN_VALUE(-1, stdunique);

	if (basePtr == derivedPtr)
	{
		TS_PRINTF("They are pointing to the same thing!\n");
	}
	else
	{
		TS_PRINTF("They are not pointing to the same thing!\n");
	}

	basePtr.reset();

	ts::UniquePointer<int> scp1 = ts::makeUnique<int>(1337);

	TS_VERIFY_POINTERS_WITH_RETURN_VALUE(-1, scp1);

	int *rawPtr = scp1.dismiss();

	// TODO: potato
	if (scp1)
	{
		TS_PRINTF("POINTER 1 IS A THING!\n");
	}
	else
	{
		TS_PRINTF("POINTER 1 IS NOT A THING!\n");
	}

	TS_PRINTF("rawPtr on the other hand has value %d\n", *rawPtr);
	delete rawPtr;
	rawPtr = nullptr;

	ts::UniquePointer<int> scp2 = std::move(scp1);

	TS_VERIFY_POINTERS_WITH_RETURN_VALUE(-2, scp1, scp2);

// 	*scp1 = 1753474;

	TS_PRINTF("shared pointer 1 value %d\n", *scp1);
	TS_PRINTF("shared pointer 2 value %d\n", *scp2);

	scp1.reset();
	TS_PRINTF("Reset shared pointer 1\n");

	if (scp1)
	{
		TS_PRINTF("POINTER 1 IS A THING!\n");
	}
	else
	{
		TS_PRINTF("POINTER 1 IS NOT A THING!\n");
	}

	TS_PRINTF("shared pointer 2 value %d\n", *scp2);

	return 1337;
}

int main(int argc, const char **argv)
{
// 	int v = test();
// 	TS_PRINTF("Test result %d\n", v);

	ts::game::GameApplication app(argc, argv);
	int returnCode = app.start();
	return returnCode;
}