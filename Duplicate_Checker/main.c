//
// Created by SungJae on 2024-09-27.
//

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

    // 총 숫자의 개수 받기
    int num_count = 0;
    fscanf(file, "%d", &num_count);

    bool isValid = true;
    int total_num = 0;
    for (int numIdx = 0; numIdx < num_count; numIdx++) {

        int input_num = 0;
        fscanf(file, "%d", &input_num);

        // 쉬프트 연산을 이용해 bit자리별 on/off 확인 (ex. 1 << 1 -> 0010, 1 << 2 -> 0100, 1 << 3 -> 1000)
        if (total_num & (1 << input_num)) {
            isValid = false;
            break;
        }

        total_num |= (1 << input_num);
    }

    fclose(file);

    printf("Duplicate Check Result: %s\n", isValid ? "VALID" : "INVALID");

    return 0;
}


