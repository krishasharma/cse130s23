# CSE 130 Spring 2023 
## Programming Assignment 5: A Caching Simulator
### Short Description 
        This assignment provides you with experience implementing and testing real caching algorithms. In this assignment, you will be implementing a cache. Your cache must support first-in-firstout (FIFO), least-recently-used (LRU), and clock eviction policies. Your program should continuously take items from stdin until stdin is closed. After each lookup, your program should print to stdout specifying whether the item that was accessed is a HIT or MISS . If the lookup was a miss, your cache will add the item to its cache and evict an item based on the eviction policy that the user specified. Before your program exits, and after stdin is closed, you must include a summary line that specifies the total number of compulsory and capacity misses.
### Build 
        Type "make" on the command line, using the Makefile provided.
### Running 
        To run the program on the command line type ./cacher [-N size] 
### Errors
        The current code only passes the last test after running "make" followed with ./test_repo.sh
### Cleaning 
        To clean please type make clean into command line

