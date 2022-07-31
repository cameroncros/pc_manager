#include "update.h"
#include "utils.h"
#include <curl/curl.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>

int install_update_arch(char update_url[MAX_URL_LENGTH + 1]) {
    int ret = SUCCESS;
    curl_buffer buffer = {0};
    char tmpdir[PATH_MAX] = "/tmp/pc_manager_XXXXXX";
    ASSERT_TRUE_CLEANUP(mkdtemp(tmpdir) != 0, "Failed to get temporary folder name");
    logInfo(SUCCESS, tmpdir);
    ASSERT_TRUE_CLEANUP(chdir(tmpdir) == 0, "Failed to switch to temporary folder");

    ASSERT_SUCCESS_CLEANUP(download_mem(update_url, &buffer), "Failed to download update");
    FILE *f = fopen("PKGBUILD", "wb");
    fwrite(buffer.data, buffer.size, 1, f);
    fclose(f);

    ASSERT_TRUE_CLEANUP(system("makepkg") == 0, "Failed to build");
    ASSERT_TRUE_CLEANUP(system("pacman -U *.tar.xz") == 0, "Failed to install");
    ASSERT_TRUE_CLEANUP(system("systemctl restart pc_manager") == 0, "Failed to install");

cleanup:
    chdir("/tmp");
    rmdir(tmpdir);
    return ret;
}

int task_update(void) {
    char update_url[MAX_URL_LENGTH + 1] = {0};
    if (!check_for_update(update_url)) {
        return SUCCESS;
    }
#if __linux__
    return install_update_arch(update_url);
#elif _WIN32
    return GENERIC_ERROR;
#endif
}
