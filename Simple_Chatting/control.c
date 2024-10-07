//
// Created by SungJae on 2024-09-27.
//

#include "simple_chatting.h"

int main(int argc, char *argv[]) {

    // 입력 받은 값이 올바르지 않을 경우
    if (argc < 2 || argc > 4) {
        printf("[FATAL] Please checked Argument Count. (required : 3 ~ 4)\n");
        return -1;
    }

    // 입력 받은 argument 체크
    bool is_start = false;
    uint8_t target = 0;
    char *client_name = NULL;
    if (CheckArgumentValid(argv, &is_start, &target, client_name) < 0) {
        return -2;
    }

    // 공유 메모리 존재 여부 확인을 통해 서버 pid 정보 얻기
    SharedData shared_data = GetServerInfoInSharedMemory();

    int result = 0;
    switch (target) {
        case SERVER_INDEX:
            result = ServerControl(is_start, &shared_data);
            break;

        case CLIENT_INDEX:
            result = ClientControl(is_start, client_name, &shared_data);
            break;
    }

    return result;
}

int CheckArgumentValid(char *argv[], bool *is_start, uint8_t *target, char *client_name) {

    // command 유효성 체크 (start | end)
    char *command = argv[1];
    if (strcasecmp(command, START_COMMAND) == 0) {
        *is_start = true;

    } else if (strcasecmp(command, STOP_COMMAND) == 0) {
        *is_start = false;

    } else {
        printf("[FATAL] Please checked command. (required : start | end)\n");
        return -1;
    }

    // target 유효성 체크 (server | client)
    char *input_target = argv[2];

    if (strcasecmp(input_target, SERVER_COMMAND) == 0) {
        *target |= SERVER_INDEX;

    } else if (strcasecmp(input_target, CLIENT_COMMAND) == 0) {
        *target |= CLIENT_INDEX;
        client_name = argv[3];
        if (client_name == NULL) {
            printf("[FATAL] Please input client name. (ex. start client client1)\n");
            return -2;
        }

    } else {
        printf("[FATAL] Please checked target. (required : server | client)\n");
        return -3;
    }

    return 0;
}

SharedData GetServerInfoInSharedMemory() {

    SharedData shared_data;
    memset(&shared_data, 0x00, sizeof(SharedData));

    if (shm_key <= 0) {
        return shared_data;
    }

    // 공유메모리 id 정보 가져오기
    int shm_id = shmget(shm_key, 0);
    if (shm_id < 0) {
        return shared_data;
    }

    // 공유메모리 정보 가져오기
    SharedData *info = NULL;
    info = (SharedData *) shmat(shm_id, NULL, 0);
    if (info == (SharedData *) -1) {
        return shared_data;
    }

    // 공유메모리 정보 복사
    shared_data.server_info.server_pid = info->server_info.server_pid;
    shared_data.total_client = info->total_client;

    // 공유 메모리 분리
    if (shmdt(info) < 0) {
        return shared_data;
    }

    return shared_data;
}

int ServerControl(bool is_start, SharedData *shared_data) {

    pid_t server_pid = shared_data->server_info.server_pid;

    if (is_start) {
        // 시작 command일 경우
        if (server_pid < 0) {
            printf("[INFO] Server is shutdown.\n");

            pid_t pid = fork();

            if (pid == 0) {
                // Server Process 시작
                execvp("server", NULL);

            } else if (pid < 0) {
                // Fork 실패했을 경우
                printf("[FATAL] Failed Fork.");
                return -1;
            }

        } else {
            // Server가 이미 실행 중일 경우
            printf("[WARN] Server is already Started. If you want to restart, Please command \'stop server\'");
        }

    } else {
        // 종료 command일 경우, stop signal 보내기
        if (server_pid > 0) {
            printf("[INFO] Stop Server.\n");

            union sigval value;
            memset(&value, 0, sizeof(value));

            value.sival_int = SERVER_STOP;
            sigqueue(server_pid, SIGUSR1, value, NULL);

        } else {
            // 서버가 이미 종료되었을 경우
            printf("[WARN] Server is already Stopped.\n");
        }
    }

    return 0;
}

int ClientControl(bool is_start, char *client_name, SharedData *shared_data) {

    pid_t server_pid = shared_data->server_info.server_pid;
    int total_client = shared_data->total_client;

    // 서버가 꺼져있을 경우, stop
    if (server_pid == 0) {
        printf("[WARN] Server is stopped. If you want to start client, Please command \'start server\' first.\n");
        return -1;
    }

    // 시작 command에서 client가 2명 이상일 경우, stop
    if (is_start && total_client >= 2) {
        printf("[WARN] Failed to Start Client. Total Client count is %d. \n", total_client);
        return -2;
    }

    // 서버에게 client 시작/종료 signal 보내기
    union sigval value;
    memset(&value, 0, sizeof(value));

    if (is_start) {
        printf("[INFO] Start Client.\n");
        value.sival_int = CLIENT_START;
        value.sival_ptr = (void *) client_name;

    } else {
        printf("[INFO] Stop Client.\n");
        value.sival_int = CLIENT_STOP;
        value.sival_ptr = (void *) client_name;
    }

    sigqueue(server_pid, SIGUSR1, value, NULL);

    return 0;
}

