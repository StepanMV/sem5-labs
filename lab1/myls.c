#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>

#define BLUE "\x1b[34m"
#define GREEN "\x1b[32m"
#define CYAN "\x1b[36m"
#define RED "\x1b[31m"
#define RESET "\x1b[0m"

struct max_lengths
{
    int links_len;
    int owner_len;
    int group_len;
    int size_len;
};

void calculate_max_lengths(const char *path, int show_hidden, struct max_lengths *max_len, blkcnt_t *total_blocks)
{
    struct dirent **namelist;
    int n;

    if ((n = scandir(path, &namelist, NULL, alphasort)) == -1)
    {
        perror("scandir");
        exit(EXIT_FAILURE);
    }

    max_len->links_len = 0;
    max_len->owner_len = 0;
    max_len->group_len = 0;
    max_len->size_len = 0;
    *total_blocks = 0;

    for (int i = 0; i < n; i++)
    {
        struct dirent *entry = namelist[i];

        if (!show_hidden && entry->d_name[0] == '.')
        {
            free(entry);
            continue;
        }

        struct stat fileStat;
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (lstat(fullpath, &fileStat) == -1)
        {
            free(entry);
            continue;
        }

        *total_blocks += fileStat.st_blocks;

        struct passwd *pwd = getpwuid(fileStat.st_uid);
        struct group *grp = getgrgid(fileStat.st_gid);

        int links_len = snprintf(NULL, 0, "%ld", (long)fileStat.st_nlink);
        int owner_len = pwd ? strlen(pwd->pw_name) : snprintf(NULL, 0, "%ld", (long)fileStat.st_uid);
        int group_len = grp ? strlen(grp->gr_name) : snprintf(NULL, 0, "%ld", (long)fileStat.st_gid);
        int size_len = snprintf(NULL, 0, "%ld", (long)fileStat.st_size);

        if (links_len > max_len->links_len)
            max_len->links_len = links_len;
        if (owner_len > max_len->owner_len)
            max_len->owner_len = owner_len;
        if (group_len > max_len->group_len)
            max_len->group_len = group_len;
        if (size_len > max_len->size_len)
            max_len->size_len = size_len;

        free(entry);
    }

    free(namelist);
}

void print_permissions(struct stat *fileStat)
{
    printf(S_ISDIR(fileStat->st_mode) ? "d" : (S_ISLNK(fileStat->st_mode) ? "l" : "-"));
    printf((fileStat->st_mode & S_IRUSR) ? "r" : "-");
    printf((fileStat->st_mode & S_IWUSR) ? "w" : "-");
    printf((fileStat->st_mode & S_IXUSR) ? "x" : "-");
    printf((fileStat->st_mode & S_IRGRP) ? "r" : "-");
    printf((fileStat->st_mode & S_IWGRP) ? "w" : "-");
    printf((fileStat->st_mode & S_IXGRP) ? "x" : "-");
    printf((fileStat->st_mode & S_IROTH) ? "r" : "-");
    printf((fileStat->st_mode & S_IWOTH) ? "w" : "-");
    printf((fileStat->st_mode & S_IXOTH) ? "x" : "-");
}

void print_file_info(const char *path, const char *name, struct stat *fileStat, int detailed, struct max_lengths *max_len)
{
    if (detailed)
    {
        print_permissions(fileStat);

        printf(" %*ld", max_len->links_len, (long)fileStat->st_nlink);

        struct passwd *pwd = getpwuid(fileStat->st_uid);
        struct group *grp = getgrgid(fileStat->st_gid);
        if (pwd && grp)
        {
            printf(" %-*s %-*s", max_len->owner_len, pwd->pw_name, max_len->group_len, grp->gr_name);
        }
        else
        {
            printf(" %-*ld %-*ld", max_len->owner_len, (long)fileStat->st_uid, max_len->group_len, (long)fileStat->st_gid);
        }

        printf(" %*ld", max_len->size_len, (long)fileStat->st_size);

        char timebuf[80];
        struct tm lt;
        localtime_r(&fileStat->st_mtime, &lt);
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", &lt);
        printf(" %s ", timebuf);
    }

    const char *color = RESET;
    int is_bad_link = 0;

    if (S_ISDIR(fileStat->st_mode))
    {
        color = BLUE;
    }
    else if (S_ISLNK(fileStat->st_mode))
    {
        char link_target[1024];
        ssize_t len = readlink(path, link_target, sizeof(link_target) - 1);
        if (len != -1)
        {
            link_target[len] = '\0';

            struct stat target_stat;
            if (stat(path, &target_stat) == -1)
            {
                color = RED;
                is_bad_link = 1;
            }
            else
            {
                color = CYAN;
            }
        }
    }
    else if (fileStat->st_mode & S_IXUSR)
    {
        color = GREEN;
    }

    if (detailed)
    {
        printf("%s%s%s", color, name, RESET);

        if (S_ISLNK(fileStat->st_mode))
        {
            char link_target[1024];
            ssize_t len = readlink(path, link_target, sizeof(link_target) - 1);
            if (len != -1)
            {
                link_target[len] = '\0';
                printf(" -> %s%s%s", is_bad_link ? RED : RESET, link_target, RESET);
            }
        }

        printf("\n");
    }
    else
    {
        printf("%s%s%s  ", color, name, RESET);
    }
}

int compare_names(const struct dirent **a, const struct dirent **b)
{
    return strcasecmp((*a)->d_name, (*b)->d_name);
}

void list_directory(const char *path, int show_hidden, int detailed)
{
    struct dirent **namelist;
    int n;

    struct max_lengths max_len;
    blkcnt_t total_blocks;
    calculate_max_lengths(path, show_hidden, &max_len, &total_blocks);

    printf("total %ld\n", (long)total_blocks / 2);

    if ((n = scandir(path, &namelist, NULL, compare_names)) == -1)
    {
        exit(1);
    }

    for (int i = 0; i < n; i++)
    {
        struct dirent *entry = namelist[i];

        if (!show_hidden && entry->d_name[0] == '.')
        {
            free(entry);
            continue;
        }

        struct stat fileStat;
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (lstat(fullpath, &fileStat) == -1)
        {
            free(entry);
            continue;
        }

        print_file_info(fullpath, entry->d_name, &fileStat, detailed, &max_len);
        free(entry);
    }

    if (!detailed)
    {
        printf("\n");
    }

    free(namelist);
}

int main(int argc, char **argv)
{
    int opt;
    int show_hidden = 0;
    int detailed = 0;

    while ((opt = getopt(argc, argv, "la")) != -1)
    {
        switch (opt)
        {
        case 'l':
            detailed = 1;
            break;
        case 'a':
            show_hidden = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s [-l] [-a] <directory> ...\n", argv[0]);
            return 1;
        }
    }

    if (optind >= argc)
    {
        list_directory(".", show_hidden, detailed);
    }
    else
    {
        for (int i = optind; i < argc; i++)
        {
            printf("%s:\n", argv[i]);
            list_directory(argv[i], show_hidden, detailed);
            if (i < argc - 1)
            {
                printf("\n");
            }
        }
    }

    return 0;
}
