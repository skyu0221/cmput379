CMPUT 379 Winter 2017 – Assignment 3

Tianyu Zhang & Di Zhang

Objective: Implementing a simulator to study the TLB behavior and the page 
eviction policy of a virtual memory subsystem by reading number traces of 
32-bit memory references. TLB evictions and the virtual memory page-in page-out
eviction activity is monitored, relevant performance metrics are produced.

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
that should be removed in O(1). 

Each value will be saved in a node and each node has a malloc address. The 
program can ensure that each value will has its own address in memory and will
not be changed unless specified. 

We have tlbentries + physpages number of nodes. The nodes for TLB have 4 
members: reference, process, pointer to next node, pointer to previous node. 
The node for page table have 7 members: reference, process, height, left-child,
right-child, pointer to next node, pointer to previous node. This structure 
can save memory space occupied by the program.  

The program will first convert 4 bytes into 32-bit long integers. It will then
remove the offset, this value is then searched in the TLB table. If hit, 
tlbhits increases, else tlbtable will be updated by LRU, and searched in page
table. If hit page table, nothing happens, else pf increases, and updates the 
page table by given policy. If values that are removed from the page table by 
the given eviction policy belongs to the current process, pageout increases.
-------------------------------------------------------------------------------



