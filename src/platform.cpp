#include "platform.h"


#ifdef _WIN32
#include <cstdlib>

int setenv(const char* name, const char* value, int overwrite)
{
    return _putenv_s(name, value);
}

#endif