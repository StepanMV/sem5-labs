#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void print_file(FILE *fp, int show_ends, int number_lines, int number_nonempty_lines) {
    char line[1024];
    int line_number = 1;
    
    while (fgets(line, sizeof(line), fp)) {
        int is_empty = (line[0] == '\n');

        if (number_lines && (!number_nonempty_lines || !is_empty)) {
            printf("%6d\t", line_number++);
        } else if (number_nonempty_lines && !is_empty) {
            printf("%6d\t", line_number++);
        }

        
        if (show_ends) {
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';  
                printf("%s$\n", line);  
            } else {
                printf("%s$", line);  
            }
        } else {
            printf("%s", line);  
        }
    }
}

int main(int argc, char *argv[]) {
    int opt;
    int show_ends = 0;
    int number_lines = 0;
    int number_nonempty_lines = 0;

    
    while ((opt = getopt(argc, argv, "nbE")) != -1) {
        switch (opt) {
            case 'n':
                number_lines = 1;
                break;
            case 'b':
                number_nonempty_lines = 1;
                number_lines = 0;  
                break;
            case 'E':
                show_ends = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-n] [-b] [-E] [file...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    
    if (optind == argc) {
        print_file(stdin, show_ends, number_lines, number_nonempty_lines);
    } else {
        
        for (int i = optind; i < argc; i++) {
            FILE *fp = fopen(argv[i], "r");
            if (fp == NULL) {
                perror(argv[i]);
                exit(EXIT_FAILURE);
            }
            print_file(fp, show_ends, number_lines, number_nonempty_lines);
            fclose(fp);
        }
    }

    return 0;
}
