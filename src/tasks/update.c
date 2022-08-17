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
#include <sys/wait.h>
#include <pwd.h>
#include <sys/stat.h>
#endif
#include <stdlib.h>

#if __linux__
int build_package(char update_url[MAX_URL_LENGTH + 1])
{
    int ret = SUCCESS;
    struct passwd* pw = getpwnam("nobody");
    ASSERT_TRUE_CLEANUP(pw != NULL, "Failed to get 'nobody' user");
    ASSERT_TRUE_CLEANUP(setuid(pw->pw_uid) == 0, "Failed to set uid");
    ASSERT_TRUE_CLEANUP(seteuid(pw->pw_uid) == 0, "Failed to set euid");

    ASSERT_SUCCESS_CLEANUP(download_file(update_url, "PKGBUILD"), "Failed to download update");
    ASSERT_TRUE_CLEANUP(system("makepkg --nocheck") == 0, "Failed to build");
cleanup:
    return ret;
}

int install_update_arch(char update_url[MAX_URL_LENGTH + 1]) {
    int ret = SUCCESS;
    char tmpdir[PATH_MAX] = "/tmp/pc_manager_XXXXXX";
    ASSERT_TRUE_CLEANUP(mkdtemp(tmpdir) != 0, "Failed to get temporary folder name");
    ASSERT_TRUE_CLEANUP(chmod(tmpdir, 0777) == 0, "Failed to set permissions");
    ASSERT_TRUE_CLEANUP(chdir(tmpdir) == 0, "Failed to switch to temporary folder");

    int pid = fork();
    if (pid == 0)
    {
        exit(build_package(update_url));
    }
    else
    {
        int status = 0;
        waitpid(pid, &status, 0);
        ASSERT_SUCCESS_CLEANUP(WEXITSTATUS(status), "Failed to build package");
    }

    ASSERT_TRUE_CLEANUP(system("pacman -U --noconfirm *.tar*") == 0, "Failed to install");
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
