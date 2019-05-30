#include "Precompiled.h"
#include "GameApplication.h"

#include "ts/tessa/resource/GUID.h"

#include "ts/tessa/math/Hash.h"

#include <cinttypes>

int main(int argc, char **argv)
{
	ts::log::initialize();

	ts::GUID guid("potato");
	TS_PRINTF("Potato = %s\n", guid.getString().c_str());

	std::shared_ptr<ts::game::GameApplication> app = std::make_shared<ts::game::GameApplication>();
	if (app != nullptr)
		app->launch();

	ts::log::finalize();

	return 0;
}