//
// Created by 정지민 on 2023/02/14.
//

#ifndef TODOER_BACKLEVEL_KAKAO_H
#define TODOER_BACKLEVEL_KAKAO_H

#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "json-c/json.h"
#include "../base64/base64.h"
#include "../log.h"
#include "../structure/error_code.h"
#include "../structure/user.h"

#define MEMORY_SAFETY_LENGTH 10
#define KAKAO_ID_TOKEN_PAYLOAD_KEY_VALUE_LENGTH 14 // ID Token Payload json 키 최대크기

typedef enum _AuthStatus {
    SUCCESS,
    UNAUTHORIZED,
    OTHER_HTTP_ERROR,
    EXCEPTION
} AuthStatus;

void oauth_kakao_check_token(char* code, AuthStatus* status);
static int get_real_payload_key_list_arr_size(const char *id_payload_object_list[]);
static void* get_value_by_key_indexed(const char* payload_key_list[], void* payload_result_list[], const char* req_key);
static int __inner_get_key_index(const char* payload_key_list[], const char* req_key, int* target);
void oauth_kakao_free_resource(void* res);
void oauth_kakao_free_list(void** res, int32_t size, bool is_parent_ptr);

struct MemoryStruct {
    char *memory;
    size_t size;
};

typedef struct _KakaoOAuthTokenResponse {
    const char* token_type;
    const char* access_token;
    unsigned int expires_in;
    const char* refresh_token;
    unsigned int refresh_token_expires_in;
    const char* scope;
    const char* id_token;
} KakaoOAuthTokenResponse;

size_t __Network_write_to_memory_callback(void *buffer, size_t size, size_t nmemb, void *userp);

#endif //TODOER_BACKLEVEL_KAKAO_H
