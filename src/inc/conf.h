#ifndef PC_MANAGER_CONFIG_H
#define PC_MANAGER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

struct json_object *get_device(const char *hostname, const char *location);

int get_server_addr(char addr[2048]);

#ifdef __cplusplus
}
#endif

#endif  // PC_MANAGER_CONFIG_H