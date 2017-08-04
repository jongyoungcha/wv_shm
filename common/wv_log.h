#include <stdio.h>

#define LOG_ENTRY fprintf( stderr, "%s(line %d): %s entry\n", \
                                   __FILE__, __LINE__, __FUNCTION__ )

#define LOG_EXIT  fprintf( stderr, "%s(line %d): %s exit\n", \
                                   __FILE__, __LINE__, __FUNCTION__ )
