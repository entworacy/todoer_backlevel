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

static int get_real_payload_key_list_arr_size(const char *payload_key_list[]) {
    int res = 0, current_size = 0;
    int arr_size = sizeof(payload_key_list);
    if(payload_key_list == NULL) res = -1;
    else {
        for(; current_size < arr_size; current_size++) {
            if(payload_key_list[current_size] == NULL) continue;
            res++;
        }
    }
    return res + 1;
}


KakaoIDTokenPayload *oauth_kakao_check_token(char* code, AuthStatus2* auth_status) {
    if(auth_status == NULL) {
        log_fatal("AuthStatus is NULL");
        return NULL;
    }
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
            auth_status->error_code = OAUTH_PARAM_MISSING;
            auth_status->status = EXCEPTION;
            return NULL;
        }

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            log_error("curl_easy_perform() failed");
            auth_status->error_code = OAUTH_PROVIDER_ERROR;
            auth_status->status = EXCEPTION;
            return NULL;
        }

        json_object *obj = json_tokener_parse((const char*) chunk.memory);
        if (obj == NULL) {
            auth_status->status = EXCEPTION;
            auth_status->error_code = OAUTH_SERVER_ERROR;
            log_error("json_tokener_parse() failed");
            return NULL;
        }
        else {
            long response_code = 0;
            curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &response_code);
            if(response_code != 200) {
                // Error
                if(response_code == 401 || response_code == 400) {
                    auth_status->status = UNAUTHORIZED;
                    auth_status->error_code = OAUTH_SESSION_EXPIRED;
                }
                else {
                    auth_status->status = OTHER_HTTP_ERROR;
                    auth_status->error_code = OAUTH_SERVER_ERROR;
                }
                const char* error_code = json_object_get_string(json_object_object_get(obj, "error_code"));
                const char* error = json_object_get_string(json_object_object_get(obj, "error"));
                const char* error_description = json_object_get_string(json_object_object_get(obj, "error_description"));
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
                free(pTimeInfo);

                return NULL;
            } else {
                // Success
                json_object *access_token_object = json_object_object_get(obj, "access_token");
                json_object *token_type_object = json_object_object_get(obj, "token_type");
                json_object *refresh_token_object = json_object_object_get(obj, "refresh_token");
                json_object *id_token_object = json_object_object_get(obj, "id_token");
                json_object *expires_in_object = json_object_object_get(obj, "expires_in");
                json_object *scope_object = json_object_object_get(obj, "scope");
                json_object *refresh_token_expires_in_object = json_object_object_get(obj, "refresh_token_expires_in");
                if(access_token_object&&token_type_object&&refresh_token_object&&id_token_object&&expires_in_object&&scope_object&&refresh_token_expires_in_object) {
                    const char* access_token = json_object_get_string(access_token_object);
                    const char* id_token = json_object_get_string(id_token_object);
                    strtok(id_token, ".");
                    char* id_v = strtok(NULL, ".");
                    int def_len_size = strlen(id_v) * 3 / 4;
                    char* __buffer = malloc(def_len_size);
                    memset(__buffer, '\0', def_len_size);
                    base64_decode(id_v, __buffer, def_len_size);
                    // printf("%s\ndecode_len:%d, expected len:%d", __buffer, strlen(__buffer), def_len_size);
                    log_info("real_decode_length: %d, decode_expected_length: %d", strlen(__buffer), def_len_size);
                    json_object *id_payload_object = json_tokener_parse(__buffer);
                    if (id_payload_object == NULL) {
                        log_fatal("Failed to parse ID Token payload.");
                        auth_status->error_code = OAUTH_SERVER_ERROR;
                        auth_status->status = EXCEPTION;
                        return NULL;
                    }

                    json_object *id_payload_object_list[KAKAO_ID_TOKEN_PAYLOAD_KEY_VALUE_LENGTH] = {NULL}; //추후 늘어날 것 예방
                    const char* payload_key_list[KAKAO_ID_TOKEN_PAYLOAD_KEY_VALUE_LENGTH] = {"aud", "sub", "auth_time", "iss", "nickname", "exp", "iat", "picture", "email", NULL,};
                    void* payload_result_list[KAKAO_ID_TOKEN_PAYLOAD_KEY_VALUE_LENGTH] = {NULL};
                    enum json_type* payload_result_type_list = malloc(sizeof(enum json_type) * KAKAO_ID_TOKEN_PAYLOAD_KEY_VALUE_LENGTH);
                    memset(payload_result_type_list, NULL, sizeof(enum json_type) * KAKAO_ID_TOKEN_PAYLOAD_KEY_VALUE_LENGTH); // DEBUG
                    /**
                     * KAKAO_ID_TOKEN_PAYLOAD_KEY_VALUE_LENGTH는 메모리 보호 및 추후 수정의 용이성을 위해
                     * 기존 Key-Value 개수보다 더 크게 정의되어 있습니다.
                     *
                     * 메모리 부적절 접근을 막기위해 for문에서는 KAKAO_ID_TOKEN_PAYLOAD_KEY_VALUE_LENGTH을 제한조건으로 설정하면 안됩니다.
                     * payload_key_list의 실제 크기로 접근해야 overflow 문제가 생기지 않습니다.
                     */
                    unsigned int for_loop_limit_size = get_real_payload_key_list_arr_size(payload_key_list);
                    log_info("for_loop_limit_size: %d", for_loop_limit_size);
                    for (int i = 0; i < for_loop_limit_size; i++) {
                        id_payload_object_list[i] = json_object_object_get(id_payload_object, payload_key_list[i]);
                        if (id_payload_object_list[i] == NULL) {
                            log_fatal("id_payload에 대한 키를 정렬하던 중에 문제가 발생했습니다.");
                            log_fatal("KAKAO_ID_TOKEN_PAYLOAD_KEY_VALUE_LENGTH: %d", KAKAO_ID_TOKEN_PAYLOAD_KEY_VALUE_LENGTH);
                            log_fatal("Current Index: %d", i);
                            log_fatal("Current Key: %s", payload_key_list[i]);
                            log_info("오류가 발생하여 %d번째의 키가 무효 처리되었습니다.", i + 1);
                            continue;
                        }
                        enum json_type type = json_object_get_type(id_payload_object_list[i]);
                        payload_result_type_list[i] = type;
                        switch (type) {
                            case json_type_null:
                                payload_result_list[i] = malloc(sizeof(NULL));
                                *(char*)payload_result_list[i] = NULL;
                                break;
                            case json_type_string: {
                                char* _res_memory = json_object_get_string(id_payload_object_list[i]);

                                payload_result_list[i] = malloc( sizeof(char) * strlen(_res_memory) );
                                strcpy(payload_result_list[i], _res_memory);
                                // json_object_get_string()은 동적 할당인데 여러 메모리를 배열에 넣고 하나만 free하면
                                // 나머지도 모두 freed 되는 문제가 있어서 별도의 메모리(malloc)에 넣고 기존거는 삭제해야 한다.
                                break;
                            }
                            case json_type_int:
                                payload_result_list[i] = malloc(sizeof(int32_t));
                                *(int32_t *) payload_result_list[i] = json_object_get_int(id_payload_object_list[i]);
                                break;
                            default:
                                break;
                        }
                    }

                    char* email = (char*) get_value_by_key_indexed(payload_key_list, payload_result_list, "email");
                    char* sub = get_value_by_key_indexed(payload_key_list, payload_result_list, "sub"); // OAuth 고유 ID
                    char* profile_url = get_value_by_key_indexed(payload_key_list, payload_result_list, "picture");
                    char* user_name = get_value_by_key_indexed(payload_key_list, payload_result_list, "nickname");

                    if(!email || !sub || !profile_url || !user_name) {
                        // 모든 항목에 동의하지 않은 경우 일부 값이 NULL이다.
                        printf("필수 항목에 동의되어 있지 않습니다. [%s]\n", OAUTH_REQUIRED_DATA_MISSING);
                        auth_status->status = EXCEPTION;
                        auth_status->error_code = OAUTH_REQUIRED_DATA_MISSING;
                        return NULL;
                    }

                    // email, sub 등의 값이 저장된 메모리 배열인 payload_result_list은 함수 마지막에 freed 되므로,
                    // strcpy를 통해 값을 복사해야 합니다.
                    KakaoIDTokenPayload *payload_result = malloc(sizeof(KakaoIDTokenPayload));
                    payload_result->email = malloc(sizeof(const char) * strlen(email));
                    payload_result->sub = malloc(sizeof(const char) * strlen(sub));
                    payload_result->picture = malloc(sizeof(const char) * strlen(profile_url));
                    payload_result->nickname = malloc(sizeof(const char) * strlen(user_name));
                    if (payload_result->email && payload_result->sub && payload_result->picture && payload_result->nickname) {
                        strcpy(payload_result->email, email);
                        strcpy(payload_result->sub, sub);
                        strcpy(payload_result->picture, profile_url);
                        strcpy(payload_result->nickname, user_name);

                        free(__buffer);
                        oauth_kakao_free_resource(payload_result_type_list);
                        oauth_kakao_free_list(payload_result_list, sizeof(payload_result_list) / sizeof(void*), false);
                        oauth_kakao_free_list(id_payload_object_list, sizeof(id_payload_object_list) / sizeof(json_object*), false);
                        curl_easy_cleanup(curl);
                        oauth_kakao_free_resource(buffer);
                        free(chunk.memory);

                        auth_status->status = SUCCESS;
                        auth_status->error_code = OAUTH_SUCCESS;
                        return payload_result;
                    } else {
                        // malloc 실패
                        log_error("payload_result malloc failed");
                        auth_status->status = EXCEPTION;
                        auth_status->error_code = OAUTH_SERVER_ERROR;
                        return NULL;
                    }
                } else {
                    log_fatal("Response JSON object null error");
                    auth_status->status = EXCEPTION;
                    auth_status->error_code = OAUTH_SERVER_ERROR;
                    return NULL;
                }
            }
        }
    } else {
        log_error("curl is NULL");
        auth_status->status = EXCEPTION;
        auth_status->error_code = OAUTH_SERVER_ERROR;
        return NULL;
    }
}

static void* get_value_by_key_indexed(const char* payload_key_list[], void* payload_result_list[], const char* req_key) {
    if(payload_key_list == NULL || payload_result_list == NULL)
        return NULL;

    int idx = 0;
    if(!__inner_get_key_index(payload_key_list, req_key, &idx))
        return NULL;
    return payload_result_list[idx];
}
void oauth_kakao_free_resource(void* res) {
    if(res == NULL) return;
    free(res);
}
void oauth_kakao_free_list(void** res, int32_t size, bool is_parent_ptr) {
    for(int i = 0; i < size; i++) {
        if(res[i] != NULL) free(res[i]);
    }
    if(is_parent_ptr) free(res);
}
static int __inner_get_key_index(const char* payload_key_list[], const char* req_key, int* target) {
    int arr_size = get_real_payload_key_list_arr_size(payload_key_list);
    if (arr_size < 0) return 0;
    for(int i = 0; i < arr_size; i++) {
        if(strcmp(payload_key_list[i], req_key) == 0) *target = i;
    }
    return 1;
}