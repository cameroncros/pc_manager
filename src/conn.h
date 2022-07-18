#ifndef PC_MANAGER_CONN_H
#define PC_MANAGER_CONN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils.h"

#include <MQTTClient.h>
#include "json_object.h"
#define MAX_MQTT_TOPIC 65536


#define ONLINE "online"
#define OFFLINE "offline"

typedef enum {
    QOS0 = 0,
    QOS1 = 1,
    QOS2 = 2,
} QOS;

int conn_init(MQTTClient *client, const char *address);

int conn_register_task(MQTTClient client, const char *task_name, int (*fn)(void));

int conn_register_sensor(MQTTClient client, const char *sensor_name, const char *unit, const char *class,
                         char *(*fn)(void));

int conn_publish(MQTTClient client, const char *topic, const void *value, size_t value_len, QOS qos, bool retained);


int process_sensors(MQTTClient client);

int conn_cleanup(MQTTClient *client);

#ifdef __cplusplus
}
#endif

#endif //PC_MANAGER_CONN_H