#include "sensors/version.h"
#include <string.h>

char *sensor_version() {
    char *buffer = strdup(VERSION " - " GIT);
    return buffer;
}