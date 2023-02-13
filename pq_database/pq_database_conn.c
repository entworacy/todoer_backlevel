//
// Created by 정지민 on 2023/02/13.
//

#include "pq_database_conn.h"

PG_CONN* pq_new_connection(PG_CONNECTION_INFO info) {
    PG_CONN* pq_result = (PG_CONN *)malloc(sizeof(PG_CONN));
    pq_result->conn_status = NOT_CONNECTED;
    if(info.need_use) {
        PGconn * conn = PQsetdb(info.address, info.port, NULL, NULL, info.db_name);
        if (conn == NULL) {
            log_fatal("PG에 연결하는데에 실패했습니다. [NULL_PGConn]");
            pq_result->err_msg = "pgconn init error(n)";
            pq_result->conn_status = CONNECTION_ERROR;
        } else if(PQstatus(conn) == CONNECTION_BAD) {
            log_fatal("PG에 연결하는데에 실패했습니다. 아래의 내용을 확인하시기 바랍니다.");
            log_fatal("PQerrorMessage: [%s]", PQerrorMessage(conn));
            log_fatal("Host: %s, port: %s, DB name: %s", info.address, info.port, info.db_name);
            pq_result->err_msg = PQerrorMessage(conn);
            pq_result->conn_status = CONNECTION_ERROR;
        } else if(PQstatus(conn) == CONNECTION_OK) {
            pq_result->conn_status = CONNECTED;
            log_info("PG에 연결되었습니다. (%d)", pq_result->conn_status);
        }
    }
    return pq_result;
}

char* pq_get_error_message(PG_CONN* self) {
    NULL_ASSERT(self);
    if (self->conn_status == CONNECTED)
        return NULL;
    else if (self->err_msg == NULL)
        self->err_msg = "Unknown Error";

    return self->err_msg;
}

void pq_free_connection_resource(PG_CONN* self) {
    NULL_ASSERT(self);
    if(self->main_connection != NULL)
        PQfinish(self->main_connection);
    self->err_msg = NULL;
    self->conn_status = NOT_CONNECTED;
    free(self);
}