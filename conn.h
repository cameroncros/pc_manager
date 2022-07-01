#ifndef PC_MANAGER_CONN_H
#define PC_MANAGER_CONN_H

typedef enum {
    QOS0 = 0,
    QOS1 = 1,
    QOS2 = 2,
} QOS;

int conn_init(MQTTClient *client, const char *address);

int conn_subscribe(MQTTClient client, const char *topic, QOS qos);

int conn_publish(MQTTClient client, const char *topic, const void *value, size_t value_len, QOS qos, bool retained);

int conn_cleanup(MQTTClient *client);

#endif //PC_MANAGER_CONN_H