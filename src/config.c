#include "json.h"
#include "version.h"

struct json_object *get_device(const char *hostname, const char *location) {
    struct json_object *dev_object = json_object_new_object();
    json_object_object_add(dev_object, "name", json_object_new_string(hostname));
#if __linux
    json_object_object_add(dev_object, "model", json_object_new_string("Linux"));
#elif WIN32
    json_object_object_add(dev_object, "model", json_object_new_string("Windows"));
#endif
    json_object_object_add(dev_object, "manufacturer", json_object_new_string("me"));
    json_object_object_add(dev_object, "sw_version", json_object_new_string(VERSION));
    json_object_object_add(dev_object, "suggested_area", json_object_new_string(location));
    {
        struct json_object *ident_array = json_object_new_array();
        json_object_array_add(ident_array, json_object_new_string("pc_manager"));
        json_object_object_add(dev_object, "identifiers", ident_array);
    }
    return dev_object;
}

