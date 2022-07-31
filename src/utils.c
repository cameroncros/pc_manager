#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>
#include "utils.h"

void logmsg(LogTier tier, char* filename, int lineno, int code, const char* string)
{
    (void)tier;
    fprintf(stderr, "%s:%i :Error[%i]: %s\n", filename, lineno, code, string);
}

/* callback for curl fetch */
size_t curl_save_mem(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;                             /* calculate buffer size */
    curl_buffer *p = (curl_buffer *) userp;   /* cast pointer to fetch struct */

    p->data = (char *) realloc(p->data, p->size + realsize + 1);
    if (p->data == NULL) {
        fprintf(stderr, "ERROR: Failed to expand buffer in curl_save_mem");
        free(p->data);
        return -1;
    }

    /* copy contents to buffer */
    memcpy(&(p->data[p->size]), contents, realsize);
    p->size += realsize;
    p->data[p->size] = 0;
    return realsize;
}

int download_mem(const char *url, curl_buffer *buffer) {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_save_mem);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "pc_manager/1.0");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);

        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    if (res == CURLE_OK) return SUCCESS;
    return ERROR_GENERIC;
}
