

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

void delete_memory() {
    // 메모리 해제 코드 추가
    // read_a_line 함수에서 동적으로 할당된 메모리를 해제합니다.
	 void* ptr = read_a_line(NULL);
    free(ptr);
}
char* strndup(const char* s, size_t n)
{
    char* new = malloc(n + 1);
    if (new) {
        strncpy(new, s, n);
        new[n] = '\0';
    }
    return new;
}

typedef enum {
    C_LIST = 1,
    C_SHOW,
    C_TEST,
    C_EXIT
} command_t;

char* read_a_line(FILE* fp)
{
    static char buf[BUFSIZ];
    static int buf_n = 0;
    static int curr = 0;

    if (feof(fp) && curr == buf_n - 1)
        return NULL;

    char* s = NULL;
    size_t s_len = 0;
    do {
        int end = curr;
        while (!(end >= buf_n || !iscntrl(buf[end]))) {
            end++;
        }
        if (curr < end && s != NULL) {
            s = realloc(s, s_len + end - curr + 1);
            strncat(s, buf + curr, end - curr);
            s_len += end - curr;
            curr = end;
            break;
        }
        curr = end;
        while (!(end >= buf_n || iscntrl(buf[end]))) {
            end++;
        }
        if (curr < end) {
            if (s == NULL) {
                s = strndup(buf + curr, end - curr);
                s_len = end - curr;
            } else {
                s = realloc(s, s_len + end - curr + 1);
                strncat(s, buf + curr, end - curr);
                s_len += end - curr;
            }
        }
        if (end < buf_n) {
            curr = end + 1;
            break;
        }

        buf_n = fread(buf, 1, sizeof(buf), fp);
        curr = 0;
    } while (buf_n > 0);

    return s;
}

void print_menu() {
    printf("1. List all wordbooks\n");
    printf("2. Show the words in a wordbook\n");
    printf("3. Test with a wordbook\n");
    printf("4. Exit\n");
}

int get_command() {
    int cmd;
    char input[10];

    printf(">");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        cmd = atoi(input);
    } else {
        cmd = 0; // 잘못된 입력
    }
    return cmd;
}

void list_wordbooks() {
    DIR* d = opendir("wordbooks");
    if (d == NULL) {
        perror("Failed to open directory");
        return;
    }

    printf("\n  ----\n");

    struct dirent* wb;
    while ((wb = readdir(d)) != NULL) {
        if (strcmp(wb->d_name, ".") != 0 && strcmp(wb->d_name, "..") != 0) {
            printf("  %s\n", wb->d_name);
        }
    }
    closedir(d);

    printf("  ----\n");
}

void show_words() {
    char wordbook[128];
    char filepath[256];

    list_wordbooks();

    printf("Type in the name of the wordbook?\n");
    printf(">");
    if (fgets(wordbook, sizeof(wordbook), stdin) != NULL) {
        // 개행 문자 제거
        wordbook[strcspn(wordbook, "\n")] = '\0';
    }

    snprintf(filepath, sizeof(filepath), "wordbooks/%s", wordbook);

    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) {
        perror("Failed to open file");
        return;
    }

    printf("\n  -----\n");
    char* line;
    while ((line = read_a_line(fp)) != NULL) {
        char* word = strtok(line, "\"");
        strtok(NULL, "\"");
        char* meaning = strtok(NULL, "\"");

        printf("  %s : %s\n", word, meaning);

        free(line);
    }
    printf("  -----\n\n");

    fclose(fp);
}

void run_test() {
    char wordbook[128];
    char filepath[256];

    printf("Type in the name of the wordbook?\n");
    printf(">");
    if (fgets(wordbook, sizeof(wordbook), stdin) != NULL) {
        // 개행 문자 제거
        wordbook[strcspn(wordbook, "\n")] = '\0';
    }

    snprintf(filepath, sizeof(filepath), "wordbooks/%s", wordbook);

    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) {
        perror("Failed to open file");
        return;
    }

    printf("\n-----\n");

    int n_questions = 0;
    int n_correct = 0;

    char* line;
    while ((line = read_a_line(fp)) != NULL) {
        char* word = strtok(line, "\"");
        strtok(NULL, "\"");
        char* meaning = strtok(NULL, "\"");

        printf("Q. %s\n", meaning);
        printf("?  ");

        char answer[128];
        if (fgets(answer, sizeof(answer), stdin) != NULL) {
            // 개행 문자 제거
            answer[strcspn(answer, "\n")] = '\0';
        }

        if (strcmp(answer, word) == 0) {
            printf("- correct\n");
            n_correct++;
        } else {
            printf("- wrong: %s\n", word);
        }

        n_questions++;
        free(line);
    }

    printf("(%d/%d)\n", n_correct, n_questions);

    printf("-----\n\n");

    fclose(fp);
}

int main() {
    printf(" *** Word Quiz *** \n\n");

   
