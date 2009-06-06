
#include <libgeneral.h>

int main( int argc, const char* argv[] )
{
	Util::initialize();
	Util::debug("Hello World!\n");
	Util::debug("We are now running for: %u ms.\n", Util::getTicks());
	Util::sleep(2000);
	Util::debug("We are now running for: %u ms.\n", Util::getTicks());
	Util::debug("ByeBye\n");
}
