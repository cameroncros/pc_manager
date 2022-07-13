#ifndef PC_MANAGER_CONFIG_H
#define PC_MANAGER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

struct json_object *get_device(const char *hostname, const char *location);

#ifdef __cplusplus
}
#endif

#endif  // PC_MANAGER_CONFIG_H