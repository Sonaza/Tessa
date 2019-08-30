#include "Precompiled.h"
#include "Application.h"

#include <locale>
#include <codecvt>

using namespace ts;

int asdfasd()
{
// 	std::locale locale;
// // 	locale.
// 
// 	wchar_t input = L'ä';
// 
// 	// Get the facet of the locale which deals with character conversion
// 	const std::ctype<wchar_t>& facet = std::use_facet< std::ctype<wchar_t> >(locale);

	// Use the facet to convert each character of the input string
// 	uint32 widened = static_cast<uint32>(facet.widen(input));
// 	(void)widened;

	return 1;
}

int wmain(int argc, const wchar_t **argv)
{
// 	if (asdfasd())
// 		return 0;
	
	app::Application app(argc, argv);
	int returnCode = app.launch();
	return returnCode;
}