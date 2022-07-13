#ifndef PC_MANAGER_TASKS_H
#define PC_MANAGER_TASKS_H

int task_reboot(void);

int task_shutdown(void);

#define REGISTER_TASK(task) \
    ASSERT_SUCCESS(conn_register_task(client, #task, task_##task), \
                   "Failed to register " #task)

#define REGISTER_ALL_TASKS \
    REGISTER_TASK(reboot);  \
    REGISTER_TASK(shutdown);

#endif //PC_MANAGER_TASKS_H
