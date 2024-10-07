# ArkData_OJT
- ArkData OJT 기간 동안 주어진 과제에 대한 코드 정리
- 과제 목록
    - Anagram Checker
    - Duplicate Checker
    - Simple Chatting

***

## 1. Anagram Checker
### ✔ Anagram?
- 한 단어의 철자를 분해해 다른 단어, 혹은 다른 문장으로 바꾸는 놀이

### ✔ 과제
- Anagram의 정합성을 체크하기 위한 프로그램을 작성
- Output은 Input이 올바른지 올바르지 않은 지 검증 결과를 알려준다.
- O(N)으로 풀 것

### ✔ 조건
- 문자의 제한은 150글자로 제한
- 2개의 문자를 입력 받음
- 유니코드, 특수문자는 검증에서 제외 (알파벳과 숫자만 허용)
- 알파벳 대소문자는 구분하지 않는다. ('A' == 'a')
- 두 개의의 값을 서로 비교하지 않고 처리할 것

***

## 2. Duplicate Checker
### ✔ 과제
- Input file의 오류를 체크하기 위한 프로그램을 작성
- Output은 Input이 올바른지 올바르지 않은 지 검증 결과를 알려준다.
- O(N)으로 풀것

### ✔ 조건
- 숫자의 개수는 1000개까지 입력 받을 수 있다. 
- 1~n까지의 숫자가 랜덤한 위치로 저장되어 있다. 
- 가끔 하나의 숫자가 중복으로 잘못 입력되어 있는 경우가 있다. 
- 프로그램의 전체 변수 크기는 200byte를 넘지 않는다.

***

## 3. Simple Chatting
### ✔ 과제
- IPC 통신을 이용한 Simple 채팅 기능을 구현한다.
- Server, Control, Client 프로세스로 구성되어 있다.
    - Server : client 접속/해제 모니터링
    - Control : Server 시작/종료, client 접속/해제
    - Client : 메시지 내용 확인

### ✔ 조건
- 메시지 내용은 최대 255byte까지 입력할 수 있다.
- client는 최대 2명까지 들어올 수 있다.
- server가 기동되지 않으면 client들이 접속할 수 없다.

### ✔ 프로그램 명령어 설명
- Server 시작 : control.c start server
- Server 종료 : control.c stop server
- Client 시작 : control.c start client (client 이름)
- Client 종료 : control.c stop client (client 이름)