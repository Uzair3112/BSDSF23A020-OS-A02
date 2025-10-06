/*
* Programming Assignment 02: ls v1.5.0
* Feature: Colorized Output + Alphabetical Sort + Horizontal display (-x)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      // for getopt(), STDOUT_FILENO
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>   // for ioctl() and struct winsize
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/types.h>

extern int errno;

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_RED     "\033[0;31m"
#define COLOR_PINK    "\033[0;35m"
#define COLOR_REVERSE "\033[7m"

void do_ls(const char *dir);
void do_ls_long(const char *dir); // for -l option
void do_ls_horizontal(const char *dir); // for -x option
void print_colored(const char *filename, mode_t mode);
void print_permissions(mode_t mode);

// Comparison function for qsort()
int compare_filenames(const void *a, const void *b)
{
    const char *fa = *(const char **)a;
    const char *fb = *(const char **)b;
    return strcmp(fa, fb); // alphabetical order
}

/* ===========================
   MAIN FUNCTION
   =========================== */
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
   COLOR PRINT FUNCTION
   =========================== */
void print_colored(const char *filename, mode_t mode)
{
    const char *color = COLOR_RESET;

    // Check file type
    if (S_ISLNK(mode))
        color = COLOR_PINK;
    else if (S_ISDIR(mode))
        color = COLOR_BLUE;
    else if (S_ISREG(mode) && (mode & S_IXUSR))
        color = COLOR_GREEN;
    else if (strstr(filename, ".tar") || strstr(filename, ".gz") || strstr(filename, ".zip"))
        color = COLOR_RED;
    else if (S_ISCHR(mode) || S_ISBLK(mode) || S_ISSOCK(mode))
        color = COLOR_REVERSE;

    printf("%s%s%s", color, filename, COLOR_RESET);
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

    // ✅ Sort filenames alphabetically
    qsort(files, count, sizeof(char *), compare_filenames);

    struct winsize w;
    int term_width = 80;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
        term_width = w.ws_col;

    int spacing = 2;
    int num_cols = term_width / (max_len + spacing);
    if (num_cols < 1) num_cols = 1;

    int num_files = count;
    int num_rows = (num_files + num_cols - 1) / num_cols;

    // Print in "down then across" order with colors
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            int index = row + col * num_rows;
            if (index < num_files)
            {
                struct stat fileStat;
                char path[1024];
                snprintf(path, sizeof(path), "%s/%s", dir, files[index]);
                if (lstat(path, &fileStat) == -1)
                    continue;

                printf("%-*s", max_len + spacing, "");
                printf("\033[%dD", max_len + spacing); // move cursor back
                print_colored(files[index], fileStat.st_mode);
                int pad = (max_len + spacing) - strlen(files[index]);
                for (int p = 0; p < pad; p++) printf(" ");
            }
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

    // ✅ Sort filenames alphabetically
    qsort(files, count, sizeof(char *), compare_filenames);

    struct winsize w;
    int term_width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
        term_width = w.ws_col;

    int spacing = 2;
    int num_cols = term_width / (max_len + spacing);
    if (num_cols < 1) num_cols = 1;

    int num_files = count;
    int num_rows = (num_files + num_cols - 1) / num_cols;

    // Print in "across then down" order with colors
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            int index = row * num_cols + col;
            if (index < num_files)
            {
                struct stat fileStat;
                char path[1024];
                snprintf(path, sizeof(path), "%s/%s", dir, files[index]);
                if (lstat(path, &fileStat) == -1)
                    continue;

                print_colored(files[index], fileStat.st_mode);
                int pad = (max_len + spacing) - strlen(files[index]);
                for (int p = 0; p < pad; p++) printf(" ");
            }
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

    // ✅ Step 1: Read all filenames into an array for sorting
    int capacity = 10;
    int count = 0;
    char **files = malloc(capacity * sizeof(char *));
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (count >= capacity)
        {
            capacity *= 2;
            files = realloc(files, capacity * sizeof(char *));
        }
        files[count++] = strdup(entry->d_name);
    }
    closedir(dp);

    // ✅ Step 2: Sort filenames alphabetically
    qsort(files, count, sizeof(char *), compare_filenames);

    // ✅ Step 3: Now print each file’s detailed info with color
    for (int i = 0; i < count; i++)
    {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, files[i]);

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
        printf(" %s ", timeStr);

        print_colored(files[i], fileStat.st_mode);
        printf("\n");
    }

    if (errno != 0)
        perror("readdir failed");

    // ✅ Free memory
    for (int i = 0; i < count; i++)
        free(files[i]);
    free(files);
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
