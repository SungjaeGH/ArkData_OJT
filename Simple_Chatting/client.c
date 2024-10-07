//
// Created by SungJae on 2024-09-27.
//

#include "simple_chatting.h"

int main(int argc, char *argv[]) {

    // signal 세팅
    SetClientSignal();

    // 공유메모리에서 메시지 큐 id 가져오기
    int msq_id = GetMessageQueueIdInSharedMemory();
    if (msq_id < 0) {
        printf("[FATAL] Failed to get Message Queue Id.\n");
        return -1;
    }

    // client 정보 저장
    pid_t client_pid = getpid();
    char *client_name = argv[1];
    if (SavedClientInfoInSharedMemory(client_pid, client_name) < 0) {
        printf("[FATAL] Failed to saved Client [%s] info in Shared Memory.\n", client_name);
        return -2;
    }

    while (client_signal_value == 0) {

        // 메시지 내용 입력 받기
        char message_content[INPUT_BUFFER_SIZE] = {0,};
        scanf("%s", message_content);

        // 버퍼 비우기
        getchar();

        // 메시지 내용이 256 byte 이상일 경우, 에러 메시지 보낸 후 다시 받기
        if (strlen(message_content) >= MAX_MSG_SIZE) {
            printf("[WARN] Message Content is too long. (curr : %llu byte, limit : 256 byte\n", strlen(message_content));
            continue;
        }

        // 입력 메시지 전송
        SendMessage(msq_id, client_name, client_pid, message_content);

        // 받은 메시지 출력
        RecvMessage(msq_id);
    }

    // 공유메모리에 client 정보 삭제
    DeleteClientInfoInSharedMemory(client_pid);

    return 0;
}

void ClientSignalHandler(int sig_no) {
    printf("[DEBUG] Client Signal [%d]\n", sig_no);
    client_signal_value = sig_no;
}

void SetClientSignal() {

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    // SIGTERM에 대한 signal handler 설정
    sa.sa_handler = ClientSignalHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGTERM, &sa, NULL);
}

int GetMessageQueueIdInSharedMemory() {

    if (shm_key <= 0) {
        printf("[FATAL] Failed to get the shared memory key.\n");
        return -1;
    }

    // 공유메모리 id 정보 가져오기
    int shm_id = shmget(shm_key, 0);
    if (shm_id < 0) {
        perror("[FATAL] Failed to get Shared Memory");
        return -2;
    }

    // 공유메모리 연결
    SharedData *shared_data = NULL;
    shared_data = (SharedData *) shmat(shm_id, NULL, 0);
    if (shared_data == (SharedData *) -1) {
        perror("[FATAL] Failed to Set Shared Memory Data");
        return -3;
    }

    int msq_id = shared_data->server_info.msq_id;

    // 공유 메모리 분리
    if (shmdt(shared_data) < 0) {
        perror("[FATAL] Failed to detach Shared Memory Data");
        return -4;
    }

    return msq_id;
}

int SavedClientInfoInSharedMemory(pid_t client_pid, const char *client_name) {

    if (shm_key <= 0) {
        printf("[FATAL] Failed to get the shared memory key.\n");
        return -1;
    }

    // 공유메모리 id 정보 가져오기
    int shm_id = shmget(shm_key, 0);
    if (shm_id < 0) {
        perror("[FATAL] Failed to get Shared Memory");
        return -2;
    }

    // 공유메모리 연결
    SharedData *shared_data = NULL;
    shared_data = (SharedData *) shmat(shm_id, NULL, 0);
    if (shared_data == (SharedData *) -1) {
        perror("[FATAL] Failed to Set Shared Memory Data");
        return -3;
    }

    // 새로운 client 정보 저장
    shared_data->total_client += 1;
    shared_data->client_info = realloc(shared_data->client_info, sizeof(ClientInfo) * shared_data->total_client);

    ClientInfo *client_info = shared_data->client_info + (shared_data->total_client - 1);
    client_info->client_pid = client_pid;
    strncpy(client_info->client_name, client_name, strlen(client_name));

    // 공유 메모리 분리
    if (shmdt(shared_data) < 0) {
        perror("[FATAL] Failed to detach Shared Memory Data");
        return -4;
    }

    return 0;
}

char* GetCurrentTime() {

    static char current_time[MAX_MSG_DATE_SIZE + 1] = {0,};

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    strftime(current_time, sizeof(current_time), MSG_DATE_FORMAT, tm_info);

    return current_time;
}

int SendMessage(int msq_id, char *client_name, pid_t client_pid, char *message_content) {

    MsgBuff msg_buff;
    memset(&msg_buff, 0x00, sizeof(msg_buff));

    // 전송할 메시지 정보 세팅
    msg_buff.msg_type = NORMAL_MSG_TYPE;
    msg_buff.msg_info.sender_pid = client_pid;
    strncpy(msg_buff.msg_info.sender_name, client_name, strlen(client_name));
    strncpy(msg_buff.msg_info.msg_date, GetCurrentTime(), sizeof(msg_buff.msg_info.msg_date));
    strncpy(msg_buff.msg_info.msg_content, message_content, strlen(message_content));

    // 메시지 큐에 메시지 전송
    if (msgsnd(msq_id, &msg_buff, sizeof(msg_buff), 0) == -1) {
        perror("[FATAL] Failed to send message");
        return -1;
    }

    return 0;
}

int RecvMessage(int msq_id) {

    MsgBuff msg_buff;
    memset(&msg_buff, 0x00, sizeof(msg_buff));

    if (msgrcv(msq_id, &msg_buff, sizeof(msg_buff), NORMAL_MSG_TYPE, 0) == -1) {
        perror("[FATAL] Failed to recv message");
        return -1;
    }

    printf("%s[%lld] : %s (%s)\n", msg_buff.msg_info.sender_name, msg_buff.msg_info.sender_pid,
                                        msg_buff.msg_info.msg_content, msg_buff.msg_info.msg_date);

    return 0;
}

int DeleteClientInfoInSharedMemory(pid_t client_pid) {

    if (shm_key <= 0) {
        printf("[FATAL] Failed to get the shared memory key.\n");
        return -1;
    }

    // 공유메모리 id 정보 가져오기
    int shm_id = shmget(shm_key, 0);
    if (shm_id < 0) {
        perror("[FATAL] Failed to get Shared Memory");
        return -2;
    }

    // 공유메모리 연결
    SharedData *shared_data = NULL;
    shared_data = (SharedData *) shmat(shm_id, NULL, 0);
    if (shared_data == (SharedData *) -1) {
        perror("[FATAL] Failed to Set Shared Memory Data");
        return -3;
    }

    // client pid와 일치하는 client 정보의 위치 체크하기
    int client_loc = 0;
    for (int client_idx = 0; client_idx < shared_data->total_client; client_idx++) {
        ClientInfo *client_info = shared_data->client_info + client_idx;
        if (client_info->client_pid == client_pid) {
            client_loc = client_idx;
            break;
        }
    }

    // 해당하는 client 메모리 해제
    if (shared_data->total_client == 2) {
        ClientInfo copy_client;
        memset(&copy_client, 0x00, sizeof(copy_client));

        if (client_loc == 0) {
            memcpy(&copy_client, shared_data->client_info + client_loc + 1, sizeof(ClientInfo));

        } else {
            memcpy(&copy_client, shared_data->client_info + client_loc - 1, sizeof(ClientInfo));
        }

        free(shared_data->client_info);
        shared_data->client_info = calloc(1, sizeof(ClientInfo));
        memcpy(shared_data->client_info, &copy_client, sizeof(ClientInfo));

    } else {
        free(shared_data->client_info);
    }

    shared_data->total_client -= 1;

    // 공유 메모리 분리
    if (shmdt(shared_data) < 0) {
        perror("[FATAL] Failed to detach Shared Memory Data");
        return -4;
    }

    return 0;
}