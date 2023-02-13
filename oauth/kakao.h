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

#define MEMORY_SAFETY_LENGTH 10

void kakao_code_req_test(char* code);

struct MemoryStruct {
    char *memory;
    size_t size;
};

size_t __Network_write_to_memory_callback(void *buffer, size_t size, size_t nmemb, void *userp);

#endif //TODOER_BACKLEVEL_KAKAO_H
