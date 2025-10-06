----Q1. Difference between stat() and lstat()?

stat() follows symbolic links and returns info about the target file, while lstat() returns info about the link itself.
In ls, lstat() is used to correctly display symbolic links (e.g., link -> target) instead of showing details of the target file.


----Q2. Using st_mode with Bitwise Operators?

st_mode holds both file type and permission bits.

To check file type:
if ((st_mode & S_IFMT) == S_IFDIR) // directory

To check permissions:
if (st_mode & S_IRUSR) // user read
if (st_mode & S_IWUSR) // user write
if (st_mode & S_IXUSR) // user execute

Bitwise AND (&) with macros like S_IFDIR, S_IRUSR, etc., helps extract file type and permission info.


----Q3. Explain the general logic for printing items in a "down then across" columnar format?

The program prints filenames row by row instead of one long list. It uses two loops: the outer for rows and the inner for columns. The index is calculated as index = row + col * num_rows. A single loop can’t be used because it would print all filenames in one line instead of aligned columns.



----Q4. What is the purpose of the ioctl system call in this context?

ioctl gets the terminal’s current width so the program can adjust columns dynamically. If only a fixed width (e.g., 80) is used, the layout won’t adjust to terminal size changes and may look uneven or cut off filenames.



----Q5. Compare the implementation complexity of the "down then across" (vertical) printing logic
versus the "across" (horizontal) printing logic?

The horizontal ("across") display logic is more complex because it requires pre-calculating the number of rows and columns to fit filenames evenly across the terminal width. The vertical ("down then across") layout is simpler—it prints one column at a time without much alignment computation.


----Q6. Describe the strategy you used in your code to manage the different display modes (-l, -x,
and default)?

I used flag checking after parsing command-line options. If -l was set, the program called the long-listing function; if -x was set, it called the horizontal display function; otherwise, it used the default vertical display function.



----Q7.Why is it necessary to read all directory entries into memory before sorting?

All directory entries must be read into memory first because sorting requires access to the complete dataset. The sorting algorithm (like qsort) compares and reorders elements, so it needs all filenames stored in an array.
Drawbacks:
For directories with millions of files, this approach can cause:
High memory usage — storing all names at once consumes large RAM.
Slow performance — sorting and allocating memory take longer.
Possible crashes — system may run out of memory if the directory is too large.


----Q8.Purpose and signature of the comparison function in qsort()?

The comparison function tells qsort() how to compare two elements — in this case, two filenames — so it knows their order (alphabetical, numeric, etc.).
int compare(const void *a, const void *b);
The void * pointers are generic, so qsort can work with any data type.
Inside the function, they are cast to the correct type, e.g.
return strcmp(*(const char **)a, *(const char **)b);
It returns:
< 0 if a < b
0 if equal
> 0 if a > b


----Q9: How do ANSI escape codes work to produce color in a standard Linux terminal? Show the specific code sequence for printing text in green?

ANSI escape codes are special character sequences that control text formatting, color, and cursor movement in a terminal. They start with \033[ (escape character + [), followed by color or style codes, and end with m.
For example, to print text in green, use:
printf("\033[0;32mHello\033[0m");
\033[0;32m → sets the text color to green
\033[0m → resets color back to default



----Q10: To color an executable file, you need to check its permission bits. Explain which bits in the st_mode field you need to check to determine if a file is executable by the owner, group, or others.

The st_mode field contains permission bits that describe file access rights.
To check if a file is executable, use these macros:
S_IXUSR → executable by owner
S_IXGRP → executable by group
S_IXOTH → executable by others
If any of these bits are set, the file is executable:
if (mode & (S_IXUSR | S_IXGRP | S_IXOTH))

----Q11. What is a "base case" in recursion? In the context of recursive ls, what is the base case?

In recursion, a base case is the condition that stops the recursive function from calling itself indefinitely.
In the context of the recursive ls, the base case occurs when the program encounters a directory that is not supposed to be explored further — specifically:
When the entry is "." (current directory) or ".." (parent directory).
By skipping these two entries, the recursion eventually stops once there are no more subdirectories to explore, preventing infinite loops.


----Q12.Why is it essential to construct a full path before making a recursive call?

It is essential to construct the full path (e.g., "parent_dir/subdir") before calling do_ls() so that the recursive function knows the exact location of the next directory in the filesystem.
If you only passed "subdir" without its parent path:
The program would look for "subdir" in the current working directory instead of inside "parent_dir".
This would cause incorrect listings, "No such file or directory" errors, or skipped directories.
By constructing the full path, each recursive call correctly operates within the context of its parent directory.
