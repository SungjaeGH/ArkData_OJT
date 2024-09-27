//
// Created by SungJae on 2024-09-25.
//

#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_PATH_SIZE 1024
#define FILE_BUFFER_SIZE 1024
#define FILE_CHAR_MAX_SIZE 150
#define ALPHABET_SIZE 26

bool AnagramCheck(char *source_content, char *target_content);

#endif //MAIN_H
