#include <stdio.h>
#include <stdlib.h>
#include "log.h"

#include "libpq-fe.h"
#include "pq_database/pq_database_conn.h"
#include "oauth/kakao.h"


const static char* SERVICE_VERSION = "2.3.1";
int main() {
    printf("todoer backlevel service %s\n", SERVICE_VERSION);
    char* database_url = getenv("DB_URL");
    char* env = getenv("SV_ENV");
    char* port = "";
    if(database_url == NULL || env == NULL || ( {char* port = getenv("PORT"); port; } ) == NULL) {
        log_fatal("Required variable(s) is not set.");
        if(env == NULL) {
            env = "dev";
            log_info("Env가 설정되어 있지 않아 'dev'로 설정되었습니다. 환경변수는 개발환경으로 자동 변경됩니다.");
            database_url = "127.0.0.1";
            port = "5432";
        } else return 1;
    }

    PG_CONNECTION_INFO conn_info;
    conn_info.address = database_url;
    conn_info.port = port;
    conn_info.db_name = "postgres";
    conn_info.need_use = true;

    PG_CONN* conn = pq_new_connection(conn_info);
    if(pq_get_error_message(conn) == NULL) {
        // Successfully connected
        log_info("연결되었습니다.");
        kakao_code_req_test("un7Z0CTNeF9IG-MmJqw1cV6wncEZLI-QyIrivspO9Ee4qx7sUiilIpcUhRLC6I79HRR9Ugo9dNsAAAGGS7lFcg");
    } else {
        // Something went wrong while connecting to pg database.
        return 1;
    }
    return 0;
}
