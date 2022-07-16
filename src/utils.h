#ifndef PC_MANAGER_UTILS_H
#define PC_MANAGER_UTILS_H

#if _WIN32
#  include <Windows.h>
#  define sleep(a) Sleep(a*1000)
#endif

#define SUCCESS 0
#define ERROR_GENERIC -1
#define UNUSED(a) (void)(a)

#define ASSERT_SUCCESS(a, b) {             \
    int rc = (a);                          \
    if (rc != SUCCESS) {                   \
        fprintf(stderr, __FILE__ ":%i :Error[%i]: %s\n", __LINE__, rc, b);  \
        return rc;                         \
    }                                      \
}

#endif //PC_MANAGER_UTILS_H
