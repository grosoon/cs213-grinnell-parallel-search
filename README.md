# cs213-grinnell-parallel-search

By Nolan Schoenle, Greyson Bourgeois, and Benjamin Wong

Parallel File Search (pfs) is a parallel search function that is used to search for all files and directories, within the directory from which pfs is called, whose names contain the search term.

When a matching file or directory is found, its name is printed out along with the path that leads to it.

If pfs cannot search inside of a directory, it exits with an error giving the path to the directory.  
If pfs has a file that it cannot stat, it prints a warning and continues searching.

#__________________________________________________________________________________________

pfs is only designed to work on unmodded linux binaries  

#__________________________________________________________________________________________

In order to use the program, call pfs from the directory you want to search using the format ```pfs <search_str>"``` where <search_str> is the substring you want to find.

For example, running ```./pfs pfs``` inside the cs213-grinnell-parallel-search file would print:  

pfs.dSYM in .   
pfs.c in .  
pfs.h in .  
pfs in .  
pfs in ./pfs.dSYM/Contents/Resources/DWARF  

though the order of the files might be different. 

#__________________________________________________________________________________________

All commits made during class time were peer programing including all group members.  
All commits outside of class time were individual programming.

#__________________________________________________________________________________________

Run time tests can be run on pfs by using the command ```pfs <search_str> <threads_per_core>``` where <threads_per_core> is either the number of threads you want to run on each core, or l for a linear search.    
This call will result in ```pfs <search_str>``` being run 100 times with max_threads being calculated by multiplying the number of cores in the machine by <threads_per_core>. Then, the test will print the average time it took the function to run once.
