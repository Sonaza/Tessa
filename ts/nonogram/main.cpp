#include "Precompiled.h"
#include "ts/nonogram/GameApplication.h"

int main()
{
	ts::log::initialize();

	std::shared_ptr<ts::game::GameApplication> app = std::make_shared<ts::game::GameApplication>();
	if (app != nullptr)
		app->launch();

	ts::log::finalize();

	return 0;
}