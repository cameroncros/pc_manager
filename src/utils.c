#include <stdio.h>
#include "utils.h"

void logmsg(LogTier tier, char* filename, int lineno, int code, const char* string)
{
    (void)tier;
    fprintf(stderr, "%s:%i :Error[%i]: %s\n", filename, lineno, code, string);
}
