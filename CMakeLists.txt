cmake_minimum_required(VERSION 3.21)
project(todoer_backlevel C)

set(CMAKE_C_STANDARD 17)
add_subdirectory(lib/l8w8jwt lib/b64-c)
find_package(PostgreSQL REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(json-c CONFIG REQUIRED)
add_executable(todoer_backlevel main.c log.c pq_database/pq_database_conn.c pq_database/pq_database_conn.h pth_action/pth_timer.c pth_action/pth_timer.h structure/user.h oauth/kakao.c oauth/kakao.h lib/base64/base64.h lib/base64/base64.c)


target_link_libraries(todoer_backlevel PRIVATE PostgreSQL::PostgreSQL CURL::libcurl json-c::json-c l8w8jwt)