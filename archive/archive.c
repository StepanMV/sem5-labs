#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define BUF_SIZE 4096

struct file_metadata
{
    char filename[256];
    off_t size;
    mode_t mode;
    uid_t uid;
    gid_t gid;
};

void delete_from_archive(const char *archive_name, const char *file_name)
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

        if (strcmp(metadata.filename, file_name) == 0)
        {
            printf("Deleting file: %s\n", metadata.filename);
            file_found = 1;

            if (lseek(archive_fd, metadata.size, SEEK_CUR) == (off_t)-1)
            {
                perror("Error seeking in archive");
                close(archive_fd);
                close(temp_fd);
                exit(EXIT_FAILURE);
            }

            continue;
        }

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
        printf("File '%s' not found in the archive.\n", file_name);
        remove("temp.arch");
    }
    else
    {

        if (remove(archive_name) == -1)
        {
            perror("Error removing original archive");
            exit(EXIT_FAILURE);
        }
        if (rename("temp.arch", archive_name) == -1)
        {
            perror("Error renaming temp archive");
            exit(EXIT_FAILURE);
        }
        printf("File '%s' deleted successfully from the archive.\n", file_name);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s -i|-e|-s|-d archive_name [file_name]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *archive_name = argv[2];

    if (strcmp(argv[1], "-i") == 0)
    {
        if (argc != 4)
        {
            fprintf(stderr, "Usage: %s -i archive_name file_name\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        add_to_archive(archive_name, argv[3]);
    }
    else if (strcmp(argv[1], "-e") == 0)
    {
        extract_from_archive(archive_name);
    }
    else if (strcmp(argv[1], "-s") == 0)
    {
        display_archive_stats(archive_name);
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        if (argc != 4)
        {
            fprintf(stderr, "Usage: %s -d archive_name file_name\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        delete_from_archive(archive_name, argv[3]);
    }
    else
    {
        fprintf(stderr, "Invalid option: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    return 0;
}
