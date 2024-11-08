#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <getopt.h>

#define BUF_SIZE 4096

struct file_metadata
{
    char filename[256];
    off_t size;
    mode_t mode;
    uid_t uid;
    gid_t gid;
};

void add_to_archive(const char *archive_name, char **files, int file_count);
void delete_from_archive(const char *archive_name, char **files, int file_count);
void extract_all_from_archive(const char *archive_name);
void extract_from_archive(const char *archive_name, char **files, int file_count);
void display_archive_stats(const char *archive_name);
void print_help();

void add_to_archive(const char *archive_name, char **files, int file_count)
{
    int archive_fd, file_fd;
    struct stat file_stat;
    struct file_metadata metadata;
    char buffer[BUF_SIZE];
    ssize_t bytes_read, bytes_written;

    archive_fd = open(archive_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (archive_fd == -1)
    {
        perror("Error opening archive");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < file_count; i++)
    {
        const char *file_name = files[i];
        file_fd = open(file_name, O_RDONLY);
        if (file_fd == -1)
        {
            perror("Error opening input file");
            close(archive_fd);
            exit(EXIT_FAILURE);
        }

        if (fstat(file_fd, &file_stat) == -1)
        {
            perror("Error getting file stats");
            close(file_fd);
            close(archive_fd);
            exit(EXIT_FAILURE);
        }

        strncpy(metadata.filename, file_name, sizeof(metadata.filename) - 1);
        metadata.size = file_stat.st_size;
        metadata.mode = file_stat.st_mode;
        metadata.uid = file_stat.st_uid;
        metadata.gid = file_stat.st_gid;

        bytes_written = write(archive_fd, &metadata, sizeof(metadata));
        if (bytes_written != sizeof(metadata))
        {
            perror("Error writing metadata to archive");
            close(file_fd);
            close(archive_fd);
            exit(EXIT_FAILURE);
        }

        while ((bytes_read = read(file_fd, buffer, BUF_SIZE)) > 0)
        {
            bytes_written = write(archive_fd, buffer, bytes_read);
            if (bytes_written != bytes_read)
            {
                perror("Error writing file data to archive");
                close(file_fd);
                close(archive_fd);
                exit(EXIT_FAILURE);
            }
        }

        if (bytes_read == -1)
        {
            perror("Error reading file");
        }

        close(file_fd);
    }

    close(archive_fd);
}

void delete_from_archive(const char *archive_name, char **files, int file_count)
{
    int archive_fd, temp_fd;
    struct file_metadata metadata;
    char buffer[BUF_SIZE];
    ssize_t bytes_read, bytes_written;
    off_t file_size_remaining;
    int file_found = 0;

    archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd == -1)
    {
        perror("Error opening archive");
        exit(EXIT_FAILURE);
    }

    temp_fd = open("temp.arch", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd == -1)
    {
        perror("Error opening temporary archive");
        close(archive_fd);
        exit(EXIT_FAILURE);
    }

    while (read(archive_fd, &metadata, sizeof(metadata)) == sizeof(metadata))
    {
        int skip_file = 0;

        for (int i = 0; i < file_count; i++)
        {
            if (strcmp(metadata.filename, files[i]) == 0)
            {
                skip_file = 1;
                file_found = 1;

                if (lseek(archive_fd, metadata.size, SEEK_CUR) == (off_t)-1)
                {
                    perror("Error seeking in archive");
                    close(archive_fd);
                    close(temp_fd);
                    exit(EXIT_FAILURE);
                }
                break;
            }
        }

        if (skip_file)
            continue;

        bytes_written = write(temp_fd, &metadata, sizeof(metadata));
        if (bytes_written != sizeof(metadata))
        {
            perror("Error writing metadata to temp archive");
            close(archive_fd);
            close(temp_fd);
            exit(EXIT_FAILURE);
        }

        file_size_remaining = metadata.size;
        while (file_size_remaining > 0)
        {
            bytes_read = read(archive_fd, buffer, (file_size_remaining > BUF_SIZE) ? BUF_SIZE : file_size_remaining);
            if (bytes_read == -1)
            {
                perror("Error reading from archive");
                close(archive_fd);
                close(temp_fd);
                exit(EXIT_FAILURE);
            }
            bytes_written = write(temp_fd, buffer, bytes_read);
            if (bytes_written != bytes_read)
            {
                perror("Error writing to temp archive");
                close(archive_fd);
                close(temp_fd);
                exit(EXIT_FAILURE);
            }
            file_size_remaining -= bytes_written;
        }
    }

    close(archive_fd);
    close(temp_fd);

    if (!file_found)
    {
        printf("File(s) not found in the archive.\n");
        remove("temp.arch");
    }
    else
    {

        remove(archive_name);
        rename("temp.arch", archive_name);
    }
}

void extract_all_from_archive(const char *archive_name)
{
    int archive_fd, file_fd;
    struct file_metadata metadata;
    char buffer[BUF_SIZE];
    ssize_t bytes_read, bytes_written;
    off_t file_size_remaining;

    archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd == -1)
    {
        perror("Error opening archive");
        exit(EXIT_FAILURE);
    }

    while (read(archive_fd, &metadata, sizeof(metadata)) == sizeof(metadata))
    {
        printf("Extracting file: %s\n", metadata.filename);

        file_fd = open(metadata.filename, O_WRONLY | O_CREAT | O_TRUNC, metadata.mode);
        if (file_fd == -1)
        {
            perror("Error creating output file");
            close(archive_fd);
            exit(EXIT_FAILURE);
        }

        if (fchown(file_fd, metadata.uid, metadata.gid) == -1)
        {
            perror("Error setting file ownership");
        }

        file_size_remaining = metadata.size;
        while (file_size_remaining > 0)
        {

            bytes_read = read(archive_fd, buffer, (file_size_remaining > BUF_SIZE) ? BUF_SIZE : file_size_remaining);
            if (bytes_read == -1)
            {
                perror("Error reading from archive");
                close(file_fd);
                close(archive_fd);
                exit(EXIT_FAILURE);
            }

            bytes_written = write(file_fd, buffer, bytes_read);
            if (bytes_written != bytes_read)
            {
                perror("Error writing to output file");
                close(file_fd);
                close(archive_fd);
                exit(EXIT_FAILURE);
            }

            file_size_remaining -= bytes_written;
        }

        close(file_fd);
    }

    close(archive_fd);
}

void extract_from_archive(const char *archive_name, char **files, int file_count)
{
    int archive_fd, file_fd;
    struct file_metadata metadata;
    char buffer[BUF_SIZE];
    ssize_t bytes_read, bytes_written;
    off_t file_size_remaining;
    int file_found;

    archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd == -1)
    {
        perror("Error opening archive");
        exit(EXIT_FAILURE);
    }

    while (read(archive_fd, &metadata, sizeof(metadata)) == sizeof(metadata))
    {
        file_found = 0;
        for (int i = 0; i < file_count; i++)
        {
            if (strcmp(metadata.filename, files[i]) == 0)
            {
                file_found = 1;
                printf("Extracting file: %s\n", metadata.filename);

                file_fd = open(metadata.filename, O_WRONLY | O_CREAT | O_TRUNC, metadata.mode);
                if (file_fd == -1)
                {
                    perror("Error creating output file");
                    close(archive_fd);
                    exit(EXIT_FAILURE);
                }

                if (fchown(file_fd, metadata.uid, metadata.gid) == -1)
                {
                    perror("Error setting file ownership");
                }

                file_size_remaining = metadata.size;
                while (file_size_remaining > 0)
                {
                    bytes_read = read(archive_fd, buffer, (file_size_remaining > BUF_SIZE) ? BUF_SIZE : file_size_remaining);
                    if (bytes_read == -1)
                    {
                        perror("Error reading from archive");
                        close(file_fd);
                        close(archive_fd);
                        exit(EXIT_FAILURE);
                    }

                    bytes_written = write(file_fd, buffer, bytes_read);
                    if (bytes_written != bytes_read)
                    {
                        perror("Error writing to output file");
                        close(file_fd);
                        close(archive_fd);
                        exit(EXIT_FAILURE);
                    }

                    file_size_remaining -= bytes_written;
                }

                close(file_fd);
                break;
            }
        }

        if (!file_found)
        {
            if (lseek(archive_fd, metadata.size, SEEK_CUR) == (off_t)-1)
            {
                perror("Error seeking in archive");
                close(archive_fd);
                exit(EXIT_FAILURE);
            }
        }
    }

    close(archive_fd);
}

void display_archive_stats(const char *archive_name)
{
    int archive_fd;
    struct file_metadata metadata;
    int file_count = 0;
    off_t total_size = 0;

    archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd == -1)
    {
        perror("Error opening archive");
        exit(EXIT_FAILURE);
    }

    while (read(archive_fd, &metadata, sizeof(metadata)) == sizeof(metadata))
    {
        file_count++;
        total_size += metadata.size;
        printf("File: %s, Size: %ld bytes, UID: %d, GID: %d, Mode: %o\n",
               metadata.filename, metadata.size, metadata.uid, metadata.gid, metadata.mode);

        if (lseek(archive_fd, metadata.size, SEEK_CUR) == (off_t)-1)
        {
            perror("Error seeking in archive");
            close(archive_fd);
            exit(EXIT_FAILURE);
        }
    }

    printf("\nTotal files: %d\n", file_count);
    printf("Total size: %ld bytes\n", total_size);

    close(archive_fd);
}

void print_help()
{
    printf("Usage:\n");
    printf("archive -i archive_name [filenames...] : Add file(s) to archive\n");
    printf("archive -d archive_name [filenames...] : Delete file(s) from archive\n");
    printf("archive -e archive_name                : Extract all files from archive\n");
    printf("archive -e archive_name [filenames...] : Extract specific file(s) from archive\n");
    printf("archive -s archive_name                : Display archive stats\n");
    printf("archive -h                             : Display this help message\n");
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <archive_name> [-i file1 file2 ... | -d file1 file2 ... | -e [file1 file2 ...] | -s | -h]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *archive_name = argv[1];
    char **files = NULL;
    int file_count = 0;

    if (strcmp(argv[2], "-i") == 0)
    {

        if (argc < 4)
        {
            fprintf(stderr, "Error: No file(s) specified to add to archive.\n");
            exit(EXIT_FAILURE);
        }
        files = argv + 3;
        file_count = argc - 3;
        add_to_archive(archive_name, files, file_count);
    }
    else if (strcmp(argv[2], "-d") == 0)
    {

        if (argc < 4)
        {
            fprintf(stderr, "Error: No file(s) specified to delete from archive.\n");
            exit(EXIT_FAILURE);
        }
        files = argv + 3;
        file_count = argc - 3;
        delete_from_archive(archive_name, files, file_count);
    }
    else if (strcmp(argv[2], "-e") == 0)
    {

        if (argc == 3)
        {
            extract_all_from_archive(archive_name);
        }
        else
        {
            files = argv + 3;
            file_count = argc - 3;
            extract_from_archive(archive_name, files, file_count);
        }
    }
    else if (strcmp(argv[2], "-s") == 0)
    {
        display_archive_stats(archive_name);
    }
    else if (strcmp(argv[2], "-h") == 0)
    {
        print_help();
        exit(EXIT_SUCCESS);
    }
    else
    {
        fprintf(stderr, "Invalid option.\n");
        print_help();
        exit(EXIT_FAILURE);
    }

    return 0;
}
