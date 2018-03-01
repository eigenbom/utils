#ifdef DEBUG_MEMORY
#if defined(_WIN32)
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#else
#error DEBUG_MEMORY is not implemented for this platform.
#endif

#endif

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

int main(int argc, char* argv[]) {
#ifdef DEBUG_MEMORY
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	int result = Catch::Session().run(argc, argv);
	return result;
}