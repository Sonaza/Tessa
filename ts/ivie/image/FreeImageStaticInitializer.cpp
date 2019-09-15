#include "Precompiled.h"
#include "FreeImageStaticInitializer.h"

#include "FreeImage.h"

TS_PACKAGE2(app, image)

namespace
{

void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message)
{
	TS_PRINTF("\n*** ");
	if (fif != FIF_UNKNOWN)
	{
		TS_PRINTF("%s Format: ", FreeImage_GetFormatFromFIF(fif));
	}
	TS_PRINTF("%s ***\n", message);
}

}

FreeImageStaticInitializer::FreeImageStaticInitializer()
{
	FreeImage_Initialise();
	TS_PRINTF("FreeImage initialized (running version %s)\n", FreeImage_GetVersion());

	FreeImage_SetOutputMessage(&FreeImageErrorHandler);
}

FreeImageStaticInitializer::~FreeImageStaticInitializer()
{
	FreeImage_DeInitialise();
}

FreeImageStaticInitializer &FreeImageStaticInitializer::staticInitialize()
{
	static FreeImageStaticInitializer initializer;
	return initializer;
}



TS_END_PACKAGE2()
