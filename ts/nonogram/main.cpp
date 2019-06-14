#include "Precompiled.h"
#include "GameApplication.h"

#include "ts/tessa/system/ConfigReader.h"

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

#pragma warning(disable : 4189)
#define ALIGNED(alignment, type, variable) __declspec(align(alignment)) type variable
#include <cstdlib>
#include <emmintrin.h>

float randomfloat(float min, float max)
{
	return (rand() / (float)RAND_MAX) * (max - min) + min;
}

int sse()
{
	ALIGNED(16, float, div[4]) = { 2.f, 3.f, 4.f, 5.f };
	__m128 *divsse = (__m128*)div;

	{
		std::vector<float> values;
		values.resize(4 * 500000);
		for (float &v : values)
		{
			v = randomfloat(50.f, 1337.f);
		}

		auto start = std::chrono::system_clock::now();

		for (size_t i = 0; i < values.size(); i += 4)
		{
			__m128 *m = (__m128 *)&values[i];
			_mm_store_ps((float*)&values[i], _mm_div_ps(*m, *divsse));
		}

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		TS_PRINTF("SSE elapsed: %uus (%ums)\n", elapsed.count(), elapsed.count() / 1000);
	}
	
	{
		std::vector<float> values;
		values.resize(4 * 500000);
		for (float &v : values)
		{
			v = randomfloat(50.f, 1337.f);
		}

		auto start = std::chrono::system_clock::now();

		for (size_t i = 0; i < values.size(); ++i)
		{
			values[i] = values[i] / div[i % 4];
		}

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		TS_PRINTF("Normal elapsed: %uus (%ums)\n", elapsed.count(), elapsed.count() / 1000);
	}

	return 0;
}

int main(int argc, const char **argv)
{
// 	int v = sse();
// 	TS_PRINTF("Test result %d\n", v);

	ts::game::GameApplication app(argc, argv);
	int returnCode = app.start();
	return returnCode;
}