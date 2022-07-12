#include <time.h>
#include <string.h>

char* sensor_time()
{
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    char *buffer = strdup(asctime(timeinfo));
    return buffer;
}