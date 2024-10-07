//
// Created by SungJae on 2024-09-27.
//

#include "simple_chatting.h"

char *client_name = NULL;

int main(int argc, char *argv[]) {

    // signal 세팅
    SetServerSignal();

    // 공유메모리 생성
    int shm_id = CreateSharedMemory();
    if (shm_id < 0) {
        return -1;
    }

    // 메시지 큐 생성
    int msq_id = CreateMessageQueue();
    if (msq_id < 0) {
        ResetAllocMemory(msq_id, shm_id);
        return -2;
    }

    // 공유메모리에 server 정보 저장
    pid_t server_pid = getpid();
    if (SavedServerInfoInSharedMemory(shm_id, msq_id, server_pid) < 0) {
        ResetAllocMemory(msq_id, shm_id);
        return -3;
    }

    while (!(server_signal_value & SERVER_STOP)) {

        // Client 시작 signal이 올 경우
        if (server_signal_value & CLIENT_START) {
            StartClient(client_name);

            client_name = NULL;
            server_signal_value ^= CLIENT_START;
        }

        // Client 종료 signal이 올 경우
        if (server_signal_value & CLIENT_STOP) {
            StopClient(client_name);

            client_name = NULL;
            server_signal_value ^= CLIENT_STOP;
        }

        // 1초 대기
        sleep(1);
    }

    printf("[INFO] Server [%lld] Shutdown.\n", server_pid);

    // 공유메모리 + 메시지 큐 메모리 해제
    ResetAllocMemory(msq_id, shm_id);

    return 0;
}

void ServerSignalHandler(int sig_no, siginfo_t *info, void *context) {

    switch (info->si_value.sival_int) {
        case SERVER_STOP :
            printf("[DEBUG] Server Stop Signal [%d]\n", sig_no);
            server_signal_value |= SERVER_STOP;
            break;

        case CLIENT_START :
            printf("[DEBUG] Client Start Signal [%d]\n", sig_no);
            server_signal_value |= CLIENT_START;
            client_name = (char *) info->si_value.sival_ptr;
            break;

        case CLIENT_STOP :
            printf("[DEBUG] Client Stop Signal [%d]\n", sig_no);
            server_signal_value |= CLIENT_STOP;
            client_name = (char *) info->si_value.sival_ptr;
            break;
    }
}

void SetServerSignal() {

    struct sigaction sa;
    memset(&sa, 0x00, sizeof(sa));

    // SIGUSR1에 대한 signal handler 설정
    sa.sa_sigaction = ServerSignalHandler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGUSR1, &sa, NULL);
}

int CreateSharedMemory() {

    // 공유메모리 key 생성
    shm_key = ftok("keyfile", "S");

    // 공유메모리 생성
    int shm_id = 0;
    if ((shm_id = shmget(shm_key, sizeof(SharedData), IPC_CREAT | IPC_EXCL | 0666)) < 0) {
        perror("[FATAL] Failed to Create Shared Memory");
        return -1;
    }

    return shm_id;
}

int CreateMessageQueue() {

    // 메시지 큐 key 생성
    key_t msq_key = ftok("keyfile", 'M');

    // 메시지 큐 생성
    int msq_id = 0;
    if ((msq_id = msgget(msq_key, IPC_CREAT | IPC_EXCL | 0666)) < 0) {
        perror("[FATAL] Failed to Create Message Queue");
        return -1;
    }

    return msq_id;
}

int SavedServerInfoInSharedMemory(int shm_id, int msq_id, pid_t server_pid) {

    SharedData *shared_data = NULL;

    // 공유메모리 연결
    shared_data = (SharedData *) shmat(shm_id, NULL, 0);
    if (shared_data == (SharedData *) -1) {
        perror("[FATAL] Failed to Set Shared Memory Data");
        return -1;
    }

    // server 정보 저장
    shared_data->server_info.msq_id = msq_id;
    shared_data->server_info.server_pid = server_pid;

    // 공유 메모리 분리
    if (shmdt(shared_data) < 0) {
        perror("[FATAL] Failed to detach Shared Memory Data");
        return -2;
    }

    return 0;
}

pid_t CheckClientExist(char *client_name, bool is_start) {

    if (shm_key <= 0) {
        return -1;
    }

    // 공유메모리 id 정보 가져오기
    int shm_id = shmget(shm_key, 0);
    if (shm_id < 0) {
        return -2;
    }

    // 공유메모리 정보 가져오기
    SharedData *shared_data = NULL;
    shared_data = (SharedData *) shmat(shm_id, NULL, 0);
    if (shared_data == (SharedData *) -1) {
        return -3;
    }

    // 동일한 client name이 존재할 경우, 해당 client name의 pid 가져오기
    pid_t client_pid = 0;
    for (int client_idx = 0; client_idx < shared_data->total_client; client_idx++) {
        ClientInfo *client_info = shared_data->client_info + client_idx;

        if (strcmp(client_info->client_name, client_name) == 0) {
            // 해당 pid가 살아있는지 체크
            if (kill(pid, 0) < 0) {
                client_pid = -4;

            } else {
                client_pid = client_info->client_pid;
            }

            break;
        }
    }

    // client 시작 command일 경우, 총 client 수 체크
    if (is_start) {
        if (shared_data->total_client >= 2) {
            client_pid = 1;
        }
    }

    // 공유 메모리 분리
    if (shmdt(shared_data) < 0) {
        return -5;
    }

    return client_pid;
}


int StartClient(char *client_name) {

    // 총 client가 2명 이상이거나 client 이름이 이미 존재할 경우, fail
    pid_t client_pid = CheckClientExist(client_name, true);
    if (client_pid == 1) {
        printf("[WARN] Total client count is over 2.\n");
        return -1;

    } else if (client_pid > 1) {
        printf("[WARN] Client is already exist. [%s : %lld]\n", client_name, client_pid);
        return -2;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Client Process 시작
        char *new_argv[] = {"client", client_name, NULL};
        execvp("client", new_argv);

    } else if (pid < 0) {
        // Fork 실패했을 경우
        printf("[FATAL] Failed Fork.");
        return -3;
    }

    printf("[INFO] Client %s [%lld] is joined.\n", client_name, pid);

    return 0;
}

int StopClient(char *client_name) {

    // client 이름에 해당하는 client가 존재하지 않을 경우, fail
    pid_t client_pid = CheckClientExist(client_name, false);
    if (client_pid <= 0) {
        printf("[FATAL] Client [%s] is not alive.\n", client_name);
        return -1;
    }

    printf("[INFO] Client %s [%lld] is stopping..", client_name, client_pid);

    // 해당 pid에 종료 signal 보내기
    kill(client_pid, SIGTERM);

    // 5초 간격으로 해당 client가 정상 종료하고 있는지 체크
    while (kill(client_pid, 0) == 0) {
        printf(".");
        sleep(5);
    }

    printf("OK.\n");

    return 0;
}

void ResetAllocMemory(int msq_id, int shm_id) {

    // 메시지 큐 삭제
    if (msq_id > 0) {
        msgctl(msq_id, IPC_RMID, NULL);
    }

    // 공유메모리 삭제
    if (msq_id > 0) {
        shmctl(shm_id, IPC_RMID, NULL);
        shm_key = 0;
    }
}



