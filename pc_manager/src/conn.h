#ifndef PC_MANAGER_CONN_H
#define PC_MANAGER_CONN_H

#include <MQTTClient.h>

typedef enum {
    QOS0 = 0,
    QOS1 = 1,
    QOS2 = 2,
} QOS;

int conn_init(MQTTClient *client, const char *address);

int conn_register_task(MQTTClient client, const char *taskname, int (*fn)(void));
int conn_cleanup(MQTTClient *client);

#endif //PC_MANAGER_CONN_H