#ifndef PC_MANAGER_UTILS_H
#define PC_MANAGER_UTILS_H

#define SUCCESS 0
#define ERROR_GENERIC -1

#define ASSERT_SUCCESS(a, b) {             \
    int rc = (a);                          \
    if (rc != SUCCESS) {                   \
        printf("Error[%i]: %s\n", rc, b);  \
        return rc;                         \
    }                                      \
}

#endif //PC_MANAGER_UTILS_H
