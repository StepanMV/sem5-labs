#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <utime.h>

struct file_header
{
    char filename[256]; 
    off_t filesize;     
    mode_t mode;        
    uid_t uid;          
    gid_t gid;          
    time_t mtime;       
};

void add_file_to_archive(int archive_fd, const char *filepath)
{
    int file_fd = open(filepath, O_RDONLY);
    if (file_fd < 0)
    {
        perror("open");
        return;
    }

    struct stat st;
    if (fstat(file_fd, &st) < 0)
    {
        perror("fstat");
        close(file_fd);
        return;
    }

    
    struct file_header header;
    strncpy(header.filename, filepath, sizeof(header.filename));
    header.filesize = st.st_size;
    header.mode = st.st_mode;
    header.uid = st.st_uid;
    header.gid = st.st_gid;
    header.mtime = st.st_mtime;

    
    write(archive_fd, &header, sizeof(header));

    
    char buffer[1024];
    ssize_t bytes;
    while ((bytes = read(file_fd, buffer, sizeof(buffer))) > 0)
    {
        write(archive_fd, buffer, bytes);
    }

    close(file_fd);
}

void extract_file_from_archive(int archive_fd)
{
    struct file_header header;

    
    if (read(archive_fd, &header, sizeof(header)) <= 0)
    {
        return;
    }

    
    int file_fd = open(header.filename, O_WRONLY | O_CREAT | O_TRUNC, header.mode);
    if (file_fd < 0)
    {
        perror("open");
        return;
    }

    
    char buffer[1024];
    ssize_t bytes_left = header.filesize;
    ssize_t bytes;
    while (bytes_left > 0 && (bytes = read(archive_fd, buffer, sizeof(buffer))) > 0)
    {
        write(file_fd, buffer, bytes);
        bytes_left -= bytes;
    }

    
    struct utimbuf new_times;
    new_times.modtime = header.mtime;
    utime(header.filename, &new_times);

    
    chown(header.filename, header.uid, header.gid);

    close(file_fd);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <archive> <operation> <file>\n", argv[0]);
        return 1;
    }

    const char *archive_path = argv[1];
    const char *operation = argv[2];

    int archive_fd = open(archive_path, O_RDWR | O_CREAT, 0644);
    if (archive_fd < 0)
    {
        perror("open");
        return 1;
    }

    if (strcmp(operation, "-a") == 0)
    {
        
        if (argc != 4)
        {
            fprintf(stderr, "Usage: %s <archive> -a <file>\n", argv[0]);
            return 1;
        }
        add_file_to_archive(archive_fd, argv[3]);
    }
    else if (strcmp(operation, "-e") == 0)
    {
        
        extract_file_from_archive(archive_fd);
    }
    else if (strcmp(operation, "-d") == 0)
    {
        
        
    }

    close(archive_fd);
    return 0;
}
