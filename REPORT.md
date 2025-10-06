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

