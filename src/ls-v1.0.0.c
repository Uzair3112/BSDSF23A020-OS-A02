/*
* Programming Assignment 02: ls v1.3.0
* Feature: Horizontal display (-x)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      // for getopt(), STDOUT_FILENO
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>   // for ioctl() and struct winsize

extern int errno;

void do_ls(const char *dir);
void do_ls_long(const char *dir); // for -l option
void do_ls_horizontal(const char *dir); // for -x option

int main(int argc, char *argv[])
{
    int opt;
    int display_mode = 0; // 0 = default, 1 = long (-l), 2 = horizontal (-x)

    // Parse command-line options
    while ((opt = getopt(argc, argv, "lx")) != -1)
    {
        switch (opt)
        {
        case 'l':
            display_mode = 1; // long listing
            break;
        case 'x':
            display_mode = 2; // horizontal display
            break;
        default:
            fprintf(stderr, "Usage: %s [-l | -x] [directory]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // If no directory specified, use current directory
    if (optind == argc)
    {
        if (display_mode == 1)
            do_ls_long(".");
        else if (display_mode == 2)
            do_ls_horizontal(".");
        else
            do_ls(".");
    }
    else
    {
        // Process all directories given as arguments
        for (int i = optind; i < argc; i++)
        {
            printf("Directory listing of %s:\n", argv[i]);
            if (display_mode == 1)
                do_ls_long(argv[i]);
            else if (display_mode == 2)
                do_ls_horizontal(argv[i]);
            else
                do_ls(argv[i]);
            puts("");
        }
    }

    return 0;
}
 

/* ===========================
   DEFAULT (NON -l) LISTING
   =========================== */
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
    int capacity = 10;
    int count = 0;
    char **files = malloc(capacity * sizeof(char *));
    int max_len = 0;

    // Read directory entries
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;  // skip hidden files

        if (count >= capacity)
        {
            capacity *= 2;
            files = realloc(files, capacity * sizeof(char *));
        }

        files[count] = strdup(entry->d_name);
        if ((int)strlen(entry->d_name) > max_len)
            max_len = strlen(entry->d_name);

        count++;
    }

    if (errno != 0)
        perror("readdir failed");

    closedir(dp);

    struct winsize w;
    int term_width = 80;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
        term_width = w.ws_col;

    int spacing = 2;
    int num_cols = term_width / (max_len + spacing);
    if (num_cols < 1) num_cols = 1;

    int num_files = count;
    int num_rows = (num_files + num_cols - 1) / num_cols;

    // Print in "down then across" order
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            int index = row + col * num_rows;
            if (index < num_files)
                printf("%-*s", max_len + spacing, files[index]);
        }
        printf("\n");
    }

    for (int i = 0; i < count; i++)
        free(files[i]);
    free(files);
}

/* ===========================
   HORIZONTAL DISPLAY (-x)
   =========================== */
void do_ls_horizontal(const char *dir)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    errno = 0;
    int capacity = 10;
    int count = 0;
    char **files = malloc(capacity * sizeof(char *));
    int max_len = 0;

    // Read directory entries
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (count >= capacity)
        {
            capacity *= 2;
            files = realloc(files, capacity * sizeof(char *));
        }

        files[count] = strdup(entry->d_name);
        if ((int)strlen(entry->d_name) > max_len)
            max_len = strlen(entry->d_name);
        count++;
    }

    if (errno != 0)
        perror("readdir failed");

    closedir(dp);

    struct winsize w;
    int term_width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
        term_width = w.ws_col;

    int spacing = 2;
    int num_cols = term_width / (max_len + spacing);
    if (num_cols < 1) num_cols = 1;

    int num_files = count;
    int num_rows = (num_files + num_cols - 1) / num_cols;

    // Print in "across then down" order
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            int index = row * num_cols + col;
            if (index < num_files)
                printf("%-*s", max_len + spacing, files[index]);
        }
        printf("\n");
    }

    for (int i = 0; i < count; i++)
        free(files[i]);
    free(files);
}

/* ===========================
   LONG LISTING (-l) FEATURE
   =========================== */
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/types.h>

void print_permissions(mode_t mode);

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
            continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        if (lstat(path, &fileStat) == -1)
        {
            perror("lstat");
            continue;
        }

        print_permissions(fileStat.st_mode);
        printf(" %2ld", fileStat.st_nlink);

        struct passwd *pw = getpwuid(fileStat.st_uid);
        struct group  *gr = getgrgid(fileStat.st_gid);

        printf(" %-8s %-8s", 
               pw ? pw->pw_name : "UNKNOWN",
               gr ? gr->gr_name : "UNKNOWN");

        printf(" %8ld", fileStat.st_size);

        char *timeStr = ctime(&fileStat.st_mtime);
        timeStr[strlen(timeStr) - 1] = '\0';
        printf(" %s", timeStr);

        printf(" %s\n", entry->d_name);
    }

    if (errno != 0)
        perror("readdir failed");

    closedir(dp);
}

/* ===========================
   PERMISSIONS HELPER FUNCTION
   =========================== */
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

    // Special bits (setuid, setgid, sticky)
    if (mode & S_ISUID) perms[3] = (perms[3] == 'x') ? 's' : 'S';
    if (mode & S_ISGID) perms[6] = (perms[6] == 'x') ? 's' : 'S';
    if (mode & S_ISVTX) perms[9] = (perms[9] == 'x') ? 't' : 'T';

    printf("%s", perms);
}
