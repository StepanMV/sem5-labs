#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

int apply_symbolic_mode(const char *mode_str, mode_t *file_mode)
{
    mode_t add = 0, remove = 0;
    char who = mode_str[0];
    int offset = 1;

    if (who != 'u' && who != 'g' && who != 'o' && who != 'a')
    {
        who = 'a';
        offset = 0;
    }

    char op = mode_str[offset];
    const char *permissions = &mode_str[offset + 1];

    mode_t user_bit = 0, group_bit = 0, other_bit = 0;

    if (who == 'u' || who == 'a')
        user_bit = S_IRWXU;
    if (who == 'g' || who == 'a')
        group_bit = S_IRWXG;
    if (who == 'o' || who == 'a')
        other_bit = S_IRWXO;

    for (const char *p = permissions; *p; p++)
    {
        switch (*p)
        {
        case 'r':
            add |= (S_IRUSR * !!(user_bit) | S_IRGRP * !!(group_bit) | S_IROTH * !!(other_bit));
            break;
        case 'w':
            add |= (S_IWUSR * !!(user_bit) | S_IWGRP * !!(group_bit) | S_IWOTH * !!(other_bit));
            break;
        case 'x':
            add |= (S_IXUSR * !!(user_bit) | S_IXGRP * !!(group_bit) | S_IXOTH * !!(other_bit));
            break;
        case '-':
            remove |= (S_IRWXU * !!(user_bit) | S_IRWXG * !!(group_bit) | S_IRWXO * !!(other_bit));
            break;
        default:
            fprintf(stderr, "Unknown permission: %c\n", *p);
            return -1;
        }
    }

    if (op == '+')
    {
        *file_mode |= add;
    }
    else if (op == '-')
    {
        *file_mode &= ~add;
    }
    else if (op == '=')
    {
        *file_mode &= ~(user_bit | group_bit | other_bit);
        *file_mode |= add;
    }
    else
    {
        fprintf(stderr, "Invalid operation: %c\n", op);
        return -1;
    }

    return 0;
}

int process_chmod(const char *mode_str, const char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        perror("stat");
        return -1;
    }

    mode_t new_mode = file_stat.st_mode;

    if (isdigit(mode_str[0]))
    {

        mode_t mode = strtol(mode_str, NULL, 8);
        if (chmod(file_name, mode) < 0)
        {
            perror("chmod");
            return -1;
        }
    }
    else
    {

        if (apply_symbolic_mode(mode_str, &new_mode) < 0)
        {
            return -1;
        }

        if (chmod(file_name, new_mode) < 0)
        {
            perror("chmod");
            return -1;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <mode> <file>\n", argv[0]);
        return 1;
    }

    const char *mode_str = argv[1];
    const char *file_name = argv[2];

    if (process_chmod(mode_str, file_name) < 0)
    {
        return 1;
    }

    return 0;
}
