#include "../conn.h"

int task_reboot() {

}

int register_reboot(MQTTClient client) {
    return conn_register_task(client, "reboot", task_reboot);
}