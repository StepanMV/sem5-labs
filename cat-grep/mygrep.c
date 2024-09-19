#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

int contains_pattern(const char *line, const char *pattern) {
    return strstr(line, pattern) != NULL;
}

void grep_stream(FILE *stream, const char *pattern) {
    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), stream)) {
        if (contains_pattern(line, pattern)) {
            printf("%s", line);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pattern> [file ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *pattern = argv[1];

    if (argc == 2) {
        grep_stream(stdin, pattern);
    } else {
        for (int i = 2; i < argc; ++i) {
            FILE *file = fopen(argv[i], "r");
            if (file == NULL) {
                perror(argv[i]);
                continue;
            }

            grep_stream(file, pattern);
            fclose(file);
        }
    }

    return EXIT_SUCCESS;
}
