//
// Created by SungJae on 2024-09-27.
//

#ifndef SIMPLE_CHATTING_H
#define SIMPLE_CHATTING_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/** define **/
/* control.c */
#define START_COMMAND "start"
#define STOP_COMMAND "stop"
#define CLIENT_COMMAND "client"
#define SERVER_COMMAND "server"
#define CLIENT_INDEX 0x01
#define SERVER_INDEX 0x02

/* server.c */
#define SERVER_STOP 0x01
#define CLIENT_START 0x02
#define CLIENT_STOP 0x04

/* client.c */
#define INPUT_BUFFER_SIZE 1024
#define MAX_MSG_SIZE 256
#define MAX_MSG_DATE_SIZE 20
#define NAME_SIZE 16
#define NORMAL_MSG_TYPE 1
#define MSG_DATE_FORMAT "%Y/%m/%d %H:%M:%S"

/** struct **/
/* Shared Memory */
static key_t shm_key = 0;

typedef struct ServerInfo {
    int msq_id;
    pid_t server_pid;
} ServerInfo;

typedef struct ClientInfo {
    pid_t client_pid;
    char client_name[NAME_SIZE];
} ClientInfo;

typedef struct SharedData {
    ServerInfo server_info;
    int total_client;
    ClientInfo *client_info;
} SharedData;

/* Message Queue */
typedef struct MsgInfo {
    pid_t sender_pid;
    char sender_name[NAME_SIZE];
    char msg_content[MAX_MSG_SIZE + 1];
    char msg_date[MAX_MSG_DATE_SIZE + 1];
} MsgInfo;

typedef struct MsgBuff {
    long msg_type;
    MsgInfo msg_info;
} MsgBuff;

/* signal */
volatile sig_atomic_t server_signal_value = 0;
volatile sig_atomic_t client_signal_value = 0;

/** function **/
/* control */
int CheckArgumentValid(char *argv[], bool *is_start, uint8_t *target, char *client_name);
SharedData GetServerInfoInSharedMemory();
int ServerControl(bool is_start, SharedData *shared_data);
int ClientControl(bool is_start, char *client_name, SharedData *shared_data);

/* server */
void ServerSignalHandler(int sig_no, siginfo_t *info, void *context);
void SetServerSignal();
int CreateSharedMemory();
int CreateMessageQueue();
int SavedServerInfoInSharedMemory(int shm_id, int msq_id, pid_t server_pid);
pid_t CheckClientExist(char *client_name, bool is_start);
int StartClient(char *client_name);
int StopClient(char *client_name);
void ResetAllocMemory(int msq_id, int shm_id);

/* client */
void ClientSignalHandler(int sig_no);
void SetClientSignal();
int GetMessageQueueIdInSharedMemory();
int SavedClientInfoInSharedMemory(pid_t client_pid, char *client_name);
char* GetCurrentTime();
int SendMessage(int msq_id, char *client_name, pid_t client_pid, char *message_content);
int RecvMessage(int msq_id);
int DeleteClientInfoInSharedMemory(pid_t client_pid);

#endif //SIMPLE_CHATTING_H
