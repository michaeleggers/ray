#include "lang.h"

void _Assert(char const * str_file, unsigned u_line, char const * exp)
{
    fflush(stdout);
    fprintf(stderr, "\nAssertion failed: %s\nfile: %s, line %u\n",
	    exp, str_file, u_line);
    fflush(stderr);
    abort();
}


