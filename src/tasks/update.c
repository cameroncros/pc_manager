#include "update.h"
#include "utils.h"
#if _WIN32
#include <direct.h>
#define PATH_MAX MAX_PATH
#endif
#include <curl/curl.h>
#include <malloc.h>
#if __linux__
#include <unistd.h>
#endif
#include <stdlib.h>

#if __linux__
int install_update_arch(char update_url[MAX_URL_LENGTH + 1]) {
    int ret = SUCCESS;
    curl_buffer buffer = {0};
    char tmpdir[PATH_MAX] = "/tmp/pc_manager_XXXXXX";
    ASSERT_TRUE_CLEANUP(mkdtemp(tmpdir) != 0, "Failed to get temporary folder name");
    ASSERT_TRUE_CLEANUP(chdir(tmpdir) == 0, "Failed to switch to temporary folder");

    ASSERT_SUCCESS_CLEANUP(download_mem(update_url, &buffer), "Failed to download update");
    FILE *f = fopen("PKGBUILD", "wb");
    ASSERT_TRUE_CLEANUP(f != NULL, "Failed to open PKGBUILD for writing");
    ASSERT_TRUE_CLEANUP(fwrite(buffer.data, buffer.size, 1, f) == buffer.size, "Failed to write PKGBUILD");
    ASSERT_TRUE_CLEANUP(fclose(f) == 0, "Failed to close PKGBUILD");

    ASSERT_TRUE_CLEANUP(system("makepkg") == 0, "Failed to build");
    ASSERT_TRUE_CLEANUP(system("pacman -U *.tar.xz") == 0, "Failed to install");
    ASSERT_TRUE_CLEANUP(system("systemctl restart pc_manager") == 0, "Failed to install");

cleanup:
    if (!chdir("/tmp")) {
        rmdir(tmpdir);
    }
    return ret;
}
#elif _WIN32
int install_update_win32(char update_url[MAX_URL_LENGTH + 1]) {
    int ret = SUCCESS;
    curl_buffer buffer = {0};
    char tmpdir[PATH_MAX] = { 0 };
    DWORD numchars = GetTempPathA(sizeof(tmpdir), tmpdir);
    ASSERT_TRUE_CLEANUP(numchars != 0, "Temppath too short");
    ASSERT_TRUE_CLEANUP(numchars < sizeof(tmpdir), "Temppath too short");
    ASSERT_TRUE_CLEANUP(chdir(tmpdir) == 0, "Failed to switch to temporary folder");

    ASSERT_SUCCESS_CLEANUP(download_file(update_url, "installer.msi"), "Failed to download update");

    ASSERT_TRUE_CLEANUP(system("msiexec /i installer.msi /passive") == 0, "Failed to run installer");

cleanup:
    rmdir(tmpdir);
    return ret;
}
#endif

int task_update(void) {
    char update_url[MAX_URL_LENGTH + 1] = {0};
    if (!check_for_update(update_url)) {
        return SUCCESS;
    }
#if __linux__
    return install_update_arch(update_url);
#elif _WIN32
    return install_update_win32(update_url);;
#endif
}
