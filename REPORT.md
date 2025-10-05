Q1. Difference between stat() and lstat()

stat() follows symbolic links and returns info about the target file, while lstat() returns info about the link itself.
In ls, lstat() is used to correctly display symbolic links (e.g., link -> target) instead of showing details of the target file.


Q2. Using st_mode with Bitwise Operators

st_mode holds both file type and permission bits.

To check file type:
if ((st_mode & S_IFMT) == S_IFDIR) // directory

To check permissions:
if (st_mode & S_IRUSR) // user read
if (st_mode & S_IWUSR) // user write
if (st_mode & S_IXUSR) // user execute

Bitwise AND (&) with macros like S_IFDIR, S_IRUSR, etc., helps extract file type and permission info.
