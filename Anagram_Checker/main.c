#include "main.h"

int main(int argc, char *argv[]) {

    // 파일 경로를 입력 받지 못했을 경우
    if (argc != 2) {
        printf("[FATAL] Please checked Argument Count. (required : 2)\n");
        return -1;
    }

    char *filename = argv[1];

    FILE *file = fopen(filename, "r");
    // 입력 받은 파일 경로에 파일이 없을 경우
    if (file == NULL) {
        printf("[FATAL] File Not Found. (path : %s)\n", filename);
        return -2;
    }

    char *source_content = NULL;
    char *target_content = NULL;
    int line_count = 1;
    bool isFatal = false;

    // 파일 내용에 대한 유효성 체크 + anagram 체크할 내용 복사
    while (!feof(file)) {
        char file_buff[FILE_BUFFER_SIZE] = {0,};
        fgets(file_buff, FILE_BUFFER_SIZE, file);

        if (strlen(file_buff) > FILE_CHAR_MAX_SIZE) {
            printf("[FATAL] Content Length is too longer. (current : %llu byte, limited : 150 byte)\n", strlen(file_buff));
            isFatal = true;
            break;
        }

        if (line_count == 1) {
            source_content = calloc(strlen(file_buff) + 1, sizeof(char));
            strncpy(source_content, file_buff, strlen(file_buff));

        } else if (line_count == 2) {
            target_content = calloc(strlen(file_buff) + 1, sizeof(char));
            strncpy(target_content, file_buff, strlen(file_buff));

        } else {
            printf("[FATAL] Content Line is over. (limited : 2 Line)\n");
            free(source_content);
            free(target_content);
            isFatal = true;
        }

        line_count++;
    }

    fclose(file);

    // 파일 내용 길이가 150byte 넘을 경우 + 파일 내용이 2줄 넘어갈 경우
    if (isFatal) {
        return -3;
    }

    bool isValid = AnagramCheck(source_content, target_content);
    printf("Anagram Check Result: %s\n", isValid ? "VALID" : "INVALID");

    free(source_content);
    free(target_content);

    return 0;
}

bool AnagramCheck(char *source_content, char *target_content) {

    int source_char_arr[ALPHABET_SIZE + 1] = {0,};
    int target_char_arr[ALPHABET_SIZE + 1] = {0,};
    bool isValid = true;

    for (int charIdx = 0; charIdx < strlen(source_content); charIdx++) {
        // 대소문자 구분 없이 알파벳 idx에 해당되는 공간에 count (ex. 'a', 'A' -> 0번째 공간)
        char source_char = source_content[charIdx];
        char target_char = target_content[charIdx];

        if (source_char >= 'a' && source_char <= 'z') {
            source_char_arr[source_char - 'a'] += 1;
        } else if (source_char >= 'A' && source_char <= 'Z') {
            source_char_arr[source_char - 'A'] += 1;
        }

        if (target_char >= 'a' && target_char <= 'z') {
            target_char_arr[target_char - 'a'] += 1;
        } else if (target_char >= 'A' && target_char <= 'Z') {
            target_char_arr[target_char - 'A'] += 1;
        }
    }

    // 알파벳 공간 내 해당 알파벳이 나온 횟수 체크
    for (int alpha_idx = 0; alpha_idx < ALPHABET_SIZE; alpha_idx++) {
        if (source_char_arr[alpha_idx] != target_char_arr[alpha_idx]) {
            isValid = false;
            break;
        }
    }

    return isValid;
}