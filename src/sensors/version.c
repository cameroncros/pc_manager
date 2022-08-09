#include "version.h"
#include <string.h>
#include <time.h>
#include <stdbool.h>

time_t last_updated_version = 0;

char *sensor_version(time_t now) {
    if (now - last_updated_version < 5000)
    {
        return NULL;
    }
    char *buffer = strdup(VERSION " - " GIT);
    last_updated_version = now;
    return buffer;
}