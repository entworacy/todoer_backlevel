//
// Created by 정지민 on 2023/02/14.
//

#include "kakao.h"

size_t __Network_write_to_memory_callback(void *buffer, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *) userp;
    char *ptr = (char *) realloc(mem->memory, mem->size + realsize + 1);

    if(!ptr) {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), buffer, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}


void kakao_code_req_test(char* code) {
    CURL* curl;
    CURLcode res;

    struct curl_slist *list = NULL;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://kauth.kakao.com/oauth/token");
        list = curl_slist_append(list, "Content-Type: application/x-www-form-urlencoded");
        list = curl_slist_append(list, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:109.0) Gecko/20100101 Firefox/109.0");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);

        struct MemoryStruct chunk;
        chunk.memory = (char *) malloc(1);
        chunk.size = 0;

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, __Network_write_to_memory_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);

        char* client_id = "2f2ac911a29f8a195144b2fe24ee0789"; // 9 + 1 = 10
        char* redirect_uri = "http://localhost:3000/oauth/kakao"; // 12 + 1 + 1(&) = 14
        char* grant_type = "authorization_code"; // 10 + 1 + 1(&) = 12
        // header_title and '&', '=' length: 10 + 14 + 12 = 36

        unsigned int buffer_size = (strlen(client_id) + strlen(redirect_uri) + strlen(grant_type))
                + 36 /* header_title and '&', '=' length */ + MEMORY_SAFETY_LENGTH;
        char* buffer = (char*) malloc(sizeof(char) /* 1 */ * buffer_size);

        int len = sprintf(buffer, "grant_type=%s&client_id=%s&redirect_uri=%s&code=%s",
                          curl_easy_escape(curl, grant_type, strlen(grant_type)),
                          curl_easy_escape(curl, client_id, strlen(client_id)),
                          curl_easy_escape(curl, redirect_uri, strlen(redirect_uri)),
                          curl_easy_escape(curl, code, strlen(code)));
        if (len < 36) {
            fprintf(stderr, "sprintf buffer length less than 36.\n");
            return;
        }

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        printf("res: %s\n", chunk.memory);

        json_object *obj = json_tokener_parse((const char*) chunk.memory);
        if (obj == NULL)
            fprintf(stderr, "json_tokener_parse() failed\n");
        else {
            long response_code = 0;
            curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &response_code);
            if(response_code != 200) {
                // Error
                char* error_code = json_object_get_string(json_object_object_get(obj, "error_code"));
                char* error = json_object_get_string(json_object_object_get(obj, "error"));
                char* error_description = json_object_get_string(json_object_object_get(obj, "error_description"));
                char* template = "time: %s\nerror_code: %s\nerror: %s\nerror_description: %s\nhttp_status: %ld\npost_buffer: %s\nurl: %s";
                char times[35];
                time_t rawTime = time(NULL);
                struct tm* pTimeInfo = localtime(&rawTime);
                int year = pTimeInfo->tm_year + 1900;    //연도에는 1900 더해줌
                int month = pTimeInfo->tm_mon + 1;    // 월에는 1 더해줌
                int day = pTimeInfo->tm_mday;
                int hour = pTimeInfo->tm_hour;
                int min = pTimeInfo->tm_min;
                int sec = pTimeInfo->tm_sec;

                sprintf(times, "%d-%d-%d %d:%d:%d", year, month, day, hour, min, sec);
                char* _buffer = (char*) malloc(sizeof(char) * ( (strlen(error_code) + strlen(error) + strlen(error_description) +
                        strlen(buffer)) +
                        strlen(template) + strlen("https://kauth.kakao.com/oauth/token") + strlen(times) ));
                sprintf(_buffer, template, times, error_code, error, error_description, response_code, buffer, "https://kauth.kakao.com/oauth/token");
                fprintf(stderr, "%s\n", _buffer);
            } else {
                // Success
            }
        }

        curl_easy_cleanup(curl);
        free(buffer);
        free(chunk.memory);
    }
}