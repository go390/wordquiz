#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <windows.h> // Required for GetModuleFileName
#include <unistd.h>  // Required for chdir

// Function to set the working directory
void set_working_directory(const char* path) {
    if (chdir(path) != 0) {
        perror("Failed to set working directory");
        exit(EXIT_FAILURE);
    }
}

// Function to duplicate a string with a specified length
char* strndup(const char* s, size_t n) {
    char* new_str = malloc(n + 1);
    if (new_str) {
        strncpy(new_str, s, n);
        new_str[n] = '\0';
    }
    return new_str;
}

// Enum to represent command types
typedef enum {
    C_ZERO,
    C_LIST = 1,
    C_SHOW,
    C_TEST,
    C_ADD,
    C_ADD_COMPLEX,
    C_EXIT,
} command_t;

// Function to read a line from a file
char* read_a_line(FILE* fp) {
    static char buf[BUFSIZ];
    static int buf_n = 0;
    static int curr = 0;

    if (feof(fp) && curr == buf_n - 1) {
        return NULL;
    }
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

// Function to print the menu
void print_menu() {
    printf("1. List all wordbooks\n");
    printf("2. Show the words in a wordbook\n");
    printf("3. Test with a wordbook\n");
    printf("4. Add more vocabulary\n");
    printf("5. Add complex words and phrases\n");
    printf("6. Exit\n");
}

// Function to get user command
int get_command() {
    int cmd;
    printf(">");
    scanf("%d", &cmd);
    return cmd;
}

// Function to list all wordbooks
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

// Function to show all words in a wordbook
void show_words() {
    char wordbook[128];
    char filepath[BUFSIZ];

    list_wordbooks();

    printf("Type in the name of the wordbook?\n>");
    scanf("%s", wordbook);

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

// Function to test the user with words in a wordbook
void run_test() {
    char wordbook[128];
    char filepath[BUFSIZ];

    printf("Type in the name of the wordbook?\n>");
    scanf("%s", wordbook);

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

        printf("Q. %s\n?  ", meaning);

        char answer[128];
        scanf("%s", answer);

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

// Function to add new vocabulary to a wordbook
void add_voca() {
    char wordbook[128];
    char filepath[BUFSIZ];

    list_wordbooks();

    printf("Type in the name of the wordbook?\n>");
    scanf("%s", wordbook);

    snprintf(filepath, sizeof(filepath), "wordbooks/%s", wordbook);

    FILE* fp = fopen(filepath, "a");
    if (fp == NULL) {
        perror("Failed to open file");
        return;
    }

    char word[128];
    char meaning[256];

    printf("Type the new word:\n>");
    scanf("%s", word);
    printf("Type the meaning of the word:\n>");
    getchar(); // Consume newline left by previous scanf
    fgets(meaning, sizeof(meaning), stdin);
    meaning[strcspn(meaning, "\n")] = 0; // Remove newline character

    fprintf(fp, "\"%s\" : \"%s\"\n", word, meaning);

    fclose(fp);

    printf("New vocabulary added to %s\n", wordbook);
}

// Function to add complex words and phrases to a wordbook
void add_complex_voca() {
    char wordbook[128];
    char filepath[BUFSIZ];

    list_wordbooks();

    printf("Type in the name of the wordbook?\n>");
    scanf("%s", wordbook);

    snprintf(filepath, sizeof(filepath), "wordbooks/%s", wordbook);

    FILE* fp = fopen(filepath, "a");
    if (fp == NULL) {
        perror("Failed to open file");
        return;
    }

    char word[128];
    char meaning[256];
    char additional[256];

    printf("Type the new word or phrase:\n>");
    scanf("%s", word);
    printf("Type the primary meaning of the word or phrase:\n>");
    getchar(); // Consume newline left by previous scanf
    fgets(meaning, sizeof(meaning), stdin);
    meaning[strcspn(meaning, "\n")] = 0; // Remove newline character

    printf("Type the additional meanings or similar words (comma separated):\n>");
    fgets(additional, sizeof(additional), stdin);
    additional[strcspn(additional, "\n")] = 0; // Remove newline character

    fprintf(fp, "\"%s\" : \"%s\" | Additional: %s\n", word, meaning, additional);

    fclose(fp);

    printf("New complex vocabulary added to %s\n", wordbook);
}

int main() {
    printf(" *** Word Quiz *** \n\n");

    char exe_path[BUFSIZ];
    if (GetModuleFileName(NULL, exe_path, BUFSIZ) == 0) {
        perror("Failed to get executable path");
        return 1;
    }

    char* last_slash = strrchr(exe_path, '\\');
    if (last_slash != NULL) {
        *last_slash = '\0';
        set_working_directory(exe_path);
    }

    int cmd;
    do {
        print_menu();
        cmd = get_command();
        switch (cmd) {
            case C_LIST: {
                list_wordbooks();
                break;
            }
            case C_SHOW: {
                show_words();
                break;
            }
            case
