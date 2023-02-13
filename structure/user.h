//
// Created by 정지민 on 2023/02/13.
//

#ifndef TODOER_BACKLEVEL_USER_H
#define TODOER_BACKLEVEL_USER_H

typedef struct _user {
    const char* id;
    const char* name;
    const char* OAuthID; // ID Token sub
    const char* profileUrlPath;
    const char* email;
} User;

#endif //TODOER_BACKLEVEL_USER_H
