//
// Created by 정지민 on 2023/02/13.
//

#ifndef TODOER_BACKLEVEL_PQ_DATABASE_CONN_H
#define TODOER_BACKLEVEL_PQ_DATABASE_CONN_H
#include "libpq-fe.h"
#include "../log.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define NULL_ASSERT(x) assert((x) != NULL)

typedef enum _CONNECTION_STATUS {
    CONNECTED = 0,
    NOT_CONNECTED = -1,
    CONNECTION_ERROR = -2,
    ERROR = -3
} CONNECTION_STATUS;

typedef struct PG_CONNECTION {
    PGconn* main_connection;
    CONNECTION_STATUS conn_status;


    char* err_msg;
} PG_CONN;

typedef struct PG_CONNECTION_INFO {
    bool need_use;
    const char* address;
    const char* port;
    const char* db_name;
} PG_CONNECTION_INFO;

PG_CONN* pq_new_connection(PG_CONNECTION_INFO info);
char* pq_get_error_message(PG_CONN* self);
void pq_free_connection_resource(PG_CONN* self);

#endif //TODOER_BACKLEVEL_PQ_DATABASE_CONN_H
