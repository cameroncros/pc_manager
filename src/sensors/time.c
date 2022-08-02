#include <time.h>
#include <string.h>

char *sensor_time(time_t now) {
    if (now % 60 != 0)
    {
        return NULL;
    }
    struct tm *timeinfo = localtime(&now);
    char *buffer = strdup(asctime(timeinfo));
    return buffer;
}