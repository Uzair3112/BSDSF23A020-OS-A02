/*
* Programming Assignment 02: lsv1.1.0
* Feature: -l (long listing format)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // for getopt()
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

extern int errno;

void do_ls(const char *dir);
void do_ls_long(const char *dir); // new function for -l (to be implemented later)

int main(int argc, char *argv[])
{
    int opt;
    int long_format = 0; // flag to check if -l is passed

    // Parse command-line options
    while ((opt = getopt(argc, argv, "l")) != -1)
    {
        switch (opt)
        {
            case 'l':
                long_format = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [directories...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // If no directory specified, use current directory
    if (optind == argc)
    {
        if (long_format)
            do_ls_long(".");
        else
            do_ls(".");
    }
    else
    {
        // Process all directory arguments
        for (int i = optind; i < argc; i++)
        {
            printf("Directory listing of %s:\n", argv[i]);
            if (long_format)
                do_ls_long(argv[i]);
            else
                do_ls(argv[i]);
            puts("");
        }
    }

    return 0;
}

void do_ls(const char *dir)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        printf("%s\n", entry->d_name);
    }

    if (errno != 0)
    {
        perror("readdir failed");
    }

    closedir(dp);
}

#include <pwd.h>     // for getpwuid()
#include <grp.h>     // for getgrgid()
#include <time.h>    // for ctime()
#include <sys/types.h>
#include <sys/stat.h>

void print_permissions(mode_t mode);
void do_ls_long(const char *dir);

void do_ls_long(const char *dir)
{
    struct dirent *entry;
    struct stat fileStat;
    DIR *dp = opendir(dir);

    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue; // skip hidden files

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        if (lstat(path, &fileStat) == -1)
        {
            perror("lstat");
            continue;
        }

        // Print file permissions
        print_permissions(fileStat.st_mode);

        // Print number of hard links
        printf(" %2ld", fileStat.st_nlink);

        // Print owner and group
        struct passwd *pw = getpwuid(fileStat.st_uid);
        struct group  *gr = getgrgid(fileStat.st_gid);

        printf(" %-8s %-8s", 
               pw ? pw->pw_name : "UNKNOWN",
               gr ? gr->gr_name : "UNKNOWN");

        // Print file size
        printf(" %8ld", fileStat.st_size);

        // Print modification time (trim newline from ctime)
        char *timeStr = ctime(&fileStat.st_mtime);
        timeStr[strlen(timeStr) - 1] = '\0';
        printf(" %s", timeStr);

        // Print filename
        printf(" %s\n", entry->d_name);
    }

    if (errno != 0)
        perror("readdir failed");

    closedir(dp);
}

void print_permissions(mode_t mode)
{
    char perms[11] = "----------";

    if (S_ISDIR(mode)) perms[0] = 'd';
    else if (S_ISLNK(mode)) perms[0] = 'l';
    else if (S_ISCHR(mode)) perms[0] = 'c';
    else if (S_ISBLK(mode)) perms[0] = 'b';
    else if (S_ISFIFO(mode)) perms[0] = 'p';
    else if (S_ISSOCK(mode)) perms[0] = 's';

    if (mode & S_IRUSR) perms[1] = 'r';
    if (mode & S_IWUSR) perms[2] = 'w';
    if (mode & S_IXUSR) perms[3] = 'x';
    if (mode & S_IRGRP) perms[4] = 'r';
    if (mode & S_IWGRP) perms[5] = 'w';
    if (mode & S_IXGRP) perms[6] = 'x';
    if (mode & S_IROTH) perms[7] = 'r';
    if (mode & S_IWOTH) perms[8] = 'w';
    if (mode & S_IXOTH) perms[9] = 'x';

    // Handle special bits (setuid, setgid, sticky)
    if (mode & S_ISUID) perms[3] = (perms[3] == 'x') ? 's' : 'S';
    if (mode & S_ISGID) perms[6] = (perms[6] == 'x') ? 's' : 'S';
    if (mode & S_ISVTX) perms[9] = (perms[9] == 'x') ? 't' : 'T';

    printf("%s", perms);
}
