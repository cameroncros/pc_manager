#include "../conn.h"
#include "../utils.h"
#include "../../external/curl/include/curl/curl.h"

#if __linux__
#  include <unistd.h>
#  include <linux/reboot.h>
#  include <syscall.h>
#endif

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int download_file(const char* url, const char* dest)
{
    CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(dest,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
    }
    return (int)res;
}

int task_update(void) {

    return 0;
}
