# cs213-grinnell-parallel-search

pfs is a parallel search function that is used to search for all files and directories, within the directory pfs is called from, who's names contain the search term

When a matching file or directory is found, its name is printed out along with the path that leads to it.

pfs also will print if it encounters a directory it can't search inside, giving the path to the directory.

#_____________________________________________________________________________________________
pfs is only designed to work on unmodded linux binaries

#_____________________________________________________________________________________________
in order to use the function, call pfs from the directory you want to search using the format "pfs <search_str>" where search_str is the substring you want to search for.

For example, running "./pfs pfs" inside the cs213-grinnell-parallel-search file would print:

pfs.dSYM in .
pfs.c in .
pfs.h in .
pfs in .
pfs in ./pfs.dSYM/Contents/Resources/DWARF

though the order of the files might be different. 

#_____________________________________________________________________________________________
all commits made during class time were peer programing including all group members
all commits outside of class time were individual programming

#_____________________________________________________________________________________________
run time tests can be run on pfs by using the command "pfs <search_str> <threads_per_core>" which will result in "pfs <search_str>" being run 100 times with max_threads being calculated by multiplying the number of cores in the machine by <threads_per_core>, then printing the average time it took the function to run once.
