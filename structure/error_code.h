//
// Created by 정지민 on 2023/02/14.
//

#ifndef TODOER_BACKLEVEL_ERROR_CODE_H
#define TODOER_BACKLEVEL_ERROR_CODE_H

// DFOAT 에러 코드
// DFOAT = DeFault OAuTh 약자로 OAuth 인증과정에서 문제가 발생했을 때 발행되는 오류 코드입니다.
#define OAUTH_REQUIRED_DATA_MISSING "DFOAT000001" // 필수 동의 항목에 동의하지 않음
#define OAUTH_PROVIDER_ERROR "DFOAT000002" // OAuth Provider 서버 에러
#define OAUTH_SESSION_EXPIRED "DFOAT000003" // response_code 만료 또는 이미 사용(401)
#define OAUTH_SERVER_ERROR "DFOAT000004" // 처리 오류



// DFOAT 에러 코드 종료


#endif //TODOER_BACKLEVEL_ERROR_CODE_H
