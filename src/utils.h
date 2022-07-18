#ifndef PC_MANAGER_UTILS_H
#define PC_MANAGER_UTILS_H


#if __linux__
#  include <stdbool.h>
#elif _WIN32
#  include <Windows.h>
#  define sleep(a) Sleep(a*1000)
#  define bool BOOL
#  define true TRUE
#  define false FALSE
#endif

#define SUCCESS 0
#define ERROR_GENERIC -1
#define UNUSED(a) (void)(a)

typedef enum {
    INFO,
    WARNING,
    ERROR
} LogTier;

void logmsg(LogTier tier, char* filename, int lineno, int code, const char* string);
#define logInfo(code, string) logmsg(INFO, __FILE__, __LINE__, code, string)
#define logWarning(code, string) logmsg(WARNING, __FILE__, __LINE__, code, string)
#define logError(code, string) logmsg(ERROR, __FILE__, __LINE__, code, string)

#define ASSERT_SUCCESS(a, b) {                \
    int rc = (a);                             \
    if (rc != SUCCESS) {                      \
        logError(rc, b);                      \
        return rc;                            \
    }                                         \
}

#endif //PC_MANAGER_UTILS_H
