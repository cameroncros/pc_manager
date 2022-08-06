#ifndef PC_MANAGER_UPDATE_H
#define PC_MANAGER_UPDATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#define MAX_URL_LENGTH 2048
#define RELEASE_URL "https://api.github.com/repos/cameroncros/pc_manager/releases"

#if __linux__
int install_update_arch(char update_url[MAX_URL_LENGTH + 1]);
#elif _WIN32
int install_update_win32(char update_url[MAX_URL_LENGTH + 1]);
#endif
bool check_for_update(char url[MAX_URL_LENGTH + 1]);

#ifdef __cplusplus
}
#endif

#endif // PC_MANAGER_UPDATE_H