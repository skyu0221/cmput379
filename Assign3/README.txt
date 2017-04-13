CMPUT 379 Winter 2017 – Assignment 3

Tianyu Zhang & Di Zhang

Objective: Implementing a simulator to study the TLB behavior and the page 
eviction policy of a virtual memory subsystem by reading number traces of 
32-bit memory references. TLB evictions and the virtual memory page-in page-out
eviction activity is monitored, relevant performance metrics are produced.

-------------------------------------------------------------------------------

makefile:

make
	This will compile the vms.c as ./tvm379

make clean
    This will remove ./tvm379 and all temporary files.

-------------------------------------------------------------------------------

Usage

	./tvm379 pgsize tlbentries {g|p} quantum physpages {f|l} tracefiles
	
	pgsize:		16~65536
	tlbentries:	8~256
	quantum:	1~infinity
	physpages:	1~1000000

Output

	tlbhits1 pf1 pageout1 avs1
	tlbhits2 pf2 pageout2 avs2
	...

-------------------------------------------------------------------------------

In our program, we implemented TLB with a linked list and check for tlbhits 
with a sequential search. The runtime for this mechanism is O(n), but the
range of TLB entries is from 8-256. This is relatively similar to the O(log n)
runtime due to its small range.

The page table is implemented with a hash table, an AVL tree, and a linked
list. We first divide the values into different AVL trees by using a hash 
function, in our case, we will hash it based on the process number. Then we 
use the AVL tree to search for page table hits. The linked list will record 
the order of values based on time (FIFO) or least recently used (LRU). We 
choose to use an AVL search tree because search, insert, and delete functions 
are all O(log n) in average and worst case. The linked list can find the value
that should be removed and take the value that recently seen (if LRU) in O(1). 

Each value will be saved in a node and each node has a malloc address. The 
program can ensure that each value will has its own address in memory and will
not be changed unless specified. 

We have tlbentries + physpages number of nodes. The nodes for TLB have 4 
members: reference, process, pointer to next node, pointer to previous node. 
There are tlbentries number of that node. The node for page table have 7 
members: reference, process, height, left-child, right-child, pointer to next 
node, pointer to previous node. This structure can save memory space occupied 
by the program.

This program saved lots of space because we use a size physpages number of 
nodes to implement page table base on the fact that the number of valid entries
in page table will always less than or equal to the number of actual frames in 
the memory.

The program will first convert 4 bytes input into 32-bit long integers. It will
then remove the offset, this value is then searched in the TLB table. If hit, 
tlbhits increases and update the page table FIFO or LRU order information, else
TLB table will be updated by LRU, and searched in page table. If hit page
table, nothing happens, else pf increases, and updates the page table by given 
policy. If a value is removed from the page table by the given eviction policy,
pageout of the removed value's process increases.

If we cannot read the next 4 bytes from a trace file, that means the process 
for that trace file should be terminated. When a process is terminated, we will
clean-up the page table and TLB by remove all content belongs to this process. 
When all processes are terminated, the program will print the result and exit.

We are using unsigned long long int to count the number of references which has
the largest space to save number. We use unsigned long int to save process 
number and the address because the maximum number of arguments we can have in
the bash is less than 2^32.

The run time for the longest (slowest) case is around 15 second in Intel(R) 
Core(TM)2 Duo CPU E6750  @ 2.66GHz (CSC 2-19).

-------------------------------------------------------------------------------
