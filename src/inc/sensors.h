#ifndef PC_MANAGER_SENSORS_H
#define PC_MANAGER_SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif

char *sensor_time(void);
char *sensor_version(void);
char *sensor_update(void);

#define REGISTER_SENSOR(sensor, unit, type) \
    ASSERT_SUCCESS(conn_register_sensor(client, #sensor, unit, type, sensor_##sensor), \
                   "Failed to register " #sensor)

#define REGISTER_ALL_SENSORS \
    REGISTER_SENSOR(time, NULL, NULL); \
    REGISTER_SENSOR(version, NULL, NULL);\
    REGISTER_SENSOR(update, NULL, NULL);

#ifdef __cplusplus
}
#endif

#endif //PC_MANAGER_SENSORS_H
