#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define RED_COLOR "\033[1;31m"
#define RESET_COLOR "\033[0m"

void print_line_with_highlight(const char *line, regex_t *regex) {
    const char *p = line;
    regmatch_t match;
    int no_matches = 1;

    while (regexec(regex, p, 1, &match, 0) == 0) {
	no_matches = 0;
	if (match.rm_eo - match.rm_so == 0)
	{
		break;
	}
        fwrite(p, 1, match.rm_so, stdout);
        fwrite(RED_COLOR, 1, strlen(RED_COLOR), stdout);
        fwrite(p + match.rm_so, 1, match.rm_eo - match.rm_so, stdout);
        fwrite(RESET_COLOR, 1, strlen(RESET_COLOR), stdout);
        p += match.rm_eo;
    }
    if (!no_matches)
    {
	fputs(p, stdout);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s pattern [file...]\n", argv[0]);
        return 1;
    }

    regex_t regex;
    if (regcomp(&regex, argv[1], REG_EXTENDED) != 0) {
        return 1;
    }

    if (argc == 2) {
        char *buffer = NULL;
        size_t len = 0;
        while (getline(&buffer, &len, stdin) != -1) {
            print_line_with_highlight(buffer, &regex);
        }
    } else {
        for (int i = 2; i < argc; i++) {
            FILE *file = fopen(argv[i], "r");
            if (file == NULL) {
                perror(argv[i]);
                continue;
            }
            char *buffer = NULL;
            size_t len = 0;
            while (getline(&buffer, &len, file) != -1) {
                print_line_with_highlight(buffer, &regex);
            }
            fclose(file);
        }
    }

    regfree(&regex);

    return 0;
}

