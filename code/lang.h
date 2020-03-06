#ifndef TR_LANG_H
#define TR_LANG_H

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
#endif
void _Assert(char const *, unsigned, char const *);

#ifdef DEBUG


    #define ASSERT(f) \
        if(f)         \
	    NULL;     \
	else 	      \
	    _Assert(__FILE__, __LINE__, #f)
#else

    #define ASSERT(f) NULL
#endif

#endif
