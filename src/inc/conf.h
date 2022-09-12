#ifndef PC_MANAGER_CONFIG_H
#define PC_MANAGER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux
#  include <unistd.h>
#  include <bits/local_lim.h>
#elif WIN32
#  define HOST_NAME_MAX 1000
#endif

struct json_object *get_device(const char *hostname, const char *location);

int get_server_addr(char addr[2048]);

int getdevicename(char devicename[HOST_NAME_MAX + 1]);

#ifdef __cplusplus
}
#endif

#endif  // PC_MANAGER_CONFIG_H