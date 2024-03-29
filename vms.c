/*
 * CMPUT 379 Winter 2017
 *
 * Tianyu Zhang (tzhang6@ualberta.ca) and Di Zhang (dzhang4@ualberta.ca)
 *
 * This is a TLB + Memory simulator.
 *
 * We use a linked list to present the TLB and use sequential search to search
 * the address mapping in the TLB because TLB is a small table.
 *
 * We use a hash table and AVL Tree to stord the mapping in page table. We also
 * added the linked list in the page table to ensure fast page fault handling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#define POWOF2( n ) ( ( n & n - 1 ) == 0 )

struct LinkedList {

	unsigned long      value;
	unsigned int       process;
	struct LinkedList *next;
	struct LinkedList *prev;
};

struct Node {

	unsigned long      value;
	unsigned int       process;
    int                height;
	struct Node       *lhs,
	                  *rhs,
	                  *next,
	                  *prev;
};

unsigned long long *tlbhits,
                   *pf,
                   *pageout,
                   *avs_helper;
long double        *avs;
unsigned long       fl;
bool               *end_of_file,
                    page_table_hit,
                    debug           = false;
struct LinkedList  *tlb             = NULL;
struct Node        *page_table_head = NULL,
                   *page_table_tail = NULL,
                  **page_table_avl  = NULL;
FILE              **trace;

void exit_with_error( char* error_message ) {

	printf( "Error: %s\n", error_message );
	/*
	free( tlbhits );
	free( pf );
	free( pageout );
	free( avs );
	*/
	exit(1);
}

// Check if the trace file exist and if we have the access to the file
bool check_file_existance( char* file_name ) {

	// Check the existance by try to open the file
	FILE *file = fopen( file_name, "r" );

	// If we can open it, close the file
	if ( file ) {

		fclose( file );
		return true;
	}

	return false;
}
// Check if all files are not finished reading
bool keep_reading( unsigned int number_of_files ) {

	// Check if any file is not finished yet
	for ( int i = 0; i < number_of_files; i++ )
		if ( !end_of_file[i] )
			return true;

	return false;
}

// Calculate the offset of address
unsigned int get_offset( unsigned int number ) {

	if ( number == 1 )
		return 0;
	// Calculate the log2(pgsize)
	return 1 + get_offset( number >> 1 );
}

// Create and initial a linked list with given process and value
struct LinkedList* create_linked_list(
                                   unsigned long value, unsigned int process ) {

	struct LinkedList *node = ( struct LinkedList* )
	                              malloc( sizeof( struct LinkedList ) );

	node->value    = value;
	node->process  = process;
	node->next     = NULL;
	node->prev     = NULL;

	return node;
}

// Create and initial a node with given value
struct Node* create_node( unsigned long value, unsigned int process ) {

	struct Node *node = ( struct Node* ) malloc( sizeof( struct Node ) );

	node->value   = value;
	node->process = process;
	node->lhs     = NULL;
	node->rhs     = NULL;
	node->next    = NULL;
	node->prev    = NULL;
	node->height  = 1;

	return node;
}

// Calculate the height of given node
int get_height( struct Node *node ) {

	if ( node == NULL )
		return 0;

	return node->height;
}

// Calculate the balance of given node
int get_balance( struct Node *node ) {

	if ( node == NULL )
		return 0;

	return get_height( node->lhs ) - get_height( node->rhs );
}

// Right rotation in the AVL Tree
// Require: Root of given tree
// Return:  Root of right rotated tree
struct Node* rotate_rhs( struct Node *root ) {

	struct Node *lhs = root->lhs,
	            *l_r = root->lhs->rhs;

	lhs->rhs  = root;
	root->lhs = l_r;

	root->height = MAX( get_height( root->lhs ), get_height( root->rhs ) ) + 1;
	lhs->height  = MAX( get_height(  lhs->lhs ), get_height(  lhs->rhs ) ) + 1;

	return lhs;
}

// Left rotation in the AVL Tree
// Require: Root of given tree
// Return:  Root of left rotated tree
struct Node* rotate_lhs( struct Node *root ) {

	struct Node *rhs = root->rhs,
	            *r_l = root->rhs->lhs;

	rhs->lhs  = root;
	root->rhs = r_l;

	root->height = MAX( get_height( root->lhs ), get_height( root->rhs ) ) + 1;
	rhs->height  = MAX( get_height(  rhs->lhs ), get_height(  rhs->rhs ) ) + 1;

	return rhs;
}

// Find the node with minimum value in the AVL Tree
// Require: Root of given tree
// Return:  Node with the minimum value
struct Node* min_value( struct Node *node ) {

	struct Node *current = node;

	while ( current->lhs != NULL )
		current = current->lhs;

	return current;
}

// Insert a node with certain value in the given AVL Tree
// Require: Root of given tree, value want to insert
// Return:  Root of AVL tree inserted target value
struct Node* insert( struct Node   *root,
                     unsigned long  value,
                     unsigned int   process ) {

	//printf( "Try to find %lu", value );
	// Check for the empty tree
	if ( root == NULL ) {

		//printf( "\n" );

		struct Node *temp = create_node( value, process );

		temp->next = page_table_head;

		if ( page_table_head != NULL )
			page_table_head->prev = temp;

		page_table_head = temp;

		//printf( "Head: %lu\n", page_table_head->value );

		return temp;
	}

	//printf( " Now: %lu\n", root->value );

	// If value is greater than root, target position is on the right
	if ( value > root->value )
		root->rhs = insert( root->rhs, value, process );

	// If value is smaller than root, target position is on the left
	else if ( value < root->value )
		root->lhs = insert( root->lhs, value, process );

	// If value is the root, no need for insertion (page table hit)
	else {

		//printf( "It is already in the list\n" );

		page_table_hit = true;

		if ( fl == 'l' && root != page_table_head ) {

			if ( root == page_table_tail && root->prev != NULL )
				page_table_tail       = root->prev;

			if ( root->prev != NULL )
				root->prev->next      = root->next;

			if ( root->next != NULL )
				root->next->prev      = root->prev;

			root->next                = page_table_head;

			if ( page_table_head != NULL )
				page_table_head->prev = root;

			root->prev                = NULL;

			page_table_head           = root;
		}

		return root;
	}

	int balance;

	// Update the height for the current root and check balance
	root->height = MAX( get_height( root->lhs ), get_height( root->rhs ) ) + 1;
	balance      = get_balance( root );

	// LL case
	if ( balance >  1 && value < root->lhs->value )
		return rotate_rhs( root );

	// LR case
	if ( balance >  1 && value > root->lhs->value ) {

		root->lhs = rotate_lhs( root->lhs );
		return rotate_rhs( root );
	}

	// RR case
	if ( balance < -1 && value > root->rhs->value )
		return rotate_lhs( root );

	// RL case
	if ( balance < -1 && value < root->rhs->value ) {

		root->rhs = rotate_rhs( root->rhs );
		return rotate_lhs( root );
	}

	return root;
}

// Delete a node with certain value in the given AVL Tree
// Require: Root of given tree, value want to delete
// Return:  Root of AVL tree removed target value
struct Node *delete( struct Node   *root,
	                 unsigned long  value,
                     bool           change_tail ) {

	//printf(" - %lu-%u", root->value, root->process);
	//printf( "*Current Tail: %lu-%u Previous Tail: %lu-%u\n",
	                             //page_table_tail->value,
	                             //page_table_tail->process,
	                             //page_table_tail->prev->value,
	                             //page_table_tail->prev->process );

	// Check for the empty tree
	if ( root == NULL )
		return root;

	// If value is greater than root, target is on the right
	if ( value > root->value )
		root->rhs = delete( root->rhs, value, change_tail );

	// If value is smaller than root, target is on the left
	else if ( value < root->value )
		root->lhs = delete( root->lhs, value, change_tail );

	// If value is the root, remove the root
	else {

		//if (page_table_tail->prev == NULL)
			//printf( "it is null\n" );

		// Move our marker for the tail one step forward
		if ( change_tail ) {

			//printf( "\nentered! Value: %lu\n", root->value );

			page_table_tail             = page_table_tail->prev;
			page_table_tail->next->prev = NULL;
			page_table_tail->next       = NULL;

			//printf( "\nTail Value: %lu-%u\n",
			               //page_table_tail->value, page_table_tail->process );
		}

		//if ( debug )
			//printf( "\nCheck\n" );

		// Check if root has less than two children
		if ( ( root->lhs == NULL ) || ( root->rhs == NULL ) ) {

			struct Node *temp = root->lhs ? root->lhs : root->rhs;

			// If no child
			if ( temp == NULL ) {

				temp = root;
				root = NULL;

			} else {

				//if ( debug )
					//printf( "Tail failed. Tail: %lu-%u\n",
										           //page_table_tail->value,
										           //page_table_tail->process );

				if ( !change_tail && page_table_tail->prev != NULL )
					page_table_tail = page_table_tail->prev->next;


				//printf( "*Current Tail: %lu-%u Previous Tail: %lu-%u\n",
				                             //page_table_tail->value,
				                             //page_table_tail->process,
				                             //page_table_tail->prev->value,
				                             //page_table_tail->prev->process );

				*root                = *temp;
				if ( temp->prev != NULL )
					temp->prev->next = root;

				//printf( "*Current Tail: %lu-%u Previous Tail: %lu-%u\n",
				                             //page_table_tail->value,
				                             //page_table_tail->process,
				                             //page_table_tail->prev->value,
				                             //page_table_tail->prev->process );

				if ( temp->next != NULL )
					temp->next->prev = root;

				//printf( "*Current Tail: %lu-%u Previous Tail: %lu-%u\n",
				                             //page_table_tail->value,
				                             //page_table_tail->process,
				                             //page_table_tail->prev->value,
				                             //page_table_tail->prev->process );

				if ( temp == page_table_head )
					page_table_head  = root;

				if ( temp == page_table_tail )
					page_table_tail  = root;

				//printf( "*Current Tail: %lu-%u Previous Tail: %lu-%u\n",
				                             //page_table_tail->value,
				                             //page_table_tail->process,
				                             //page_table_tail->prev->value,
				                             //page_table_tail->prev->process );
				//if ( debug )
					//printf( "Tail failed. Tail! %lu-%u\n",
										           //page_table_tail->value,
										           //page_table_tail->process );
			}

			free( temp );
			temp = NULL;

			//if ( debug )
				//printf( "Tail failed. Tail- %lu-%u\n",
										           //page_table_tail->value,
										           //page_table_tail->process );

		} else {

			//printf( "Handling here %p %p\n", root, page_table_tail );
			//printf( "%lu %lu", root->value, page_table_tail->value );

			// Get the inorder successor
			struct Node *temp = min_value( root->rhs );

			root->value           = temp->value;
			root->next            = temp->next;
			root->prev            = temp->prev;

			if ( temp->prev != NULL )
				temp->prev->next  = root;

			if ( temp->next != NULL )
				temp->next->prev  = root;

			if ( temp == page_table_head )
				page_table_head   = root;

			if ( temp == page_table_tail )
				page_table_tail   = root;
			
			//if ( debug )
				//printf( "Tail failed. Tail is %lu-%u\n",
										           //page_table_tail->value,
										           //page_table_tail->process );

			root->rhs             = delete( root->rhs, temp->value, false );
		}
	}

	if ( root == NULL )
		return root;
	//printf( "Current: %lu\n", root->value );

	int balance;

	// Update the height for the current root and check balance
	root->height = MAX( get_height( root->lhs ), get_height( root->rhs ) ) + 1;
	balance      = get_balance( root );

	// LL case
	if ( balance >  1 && get_balance( root->lhs ) >= 0 )
		return rotate_rhs( root );

	// LR case
	if ( balance >  1 && get_balance( root->lhs ) <  0 ) {

		root->lhs = rotate_lhs( root->lhs );
		return rotate_rhs( root );
	}

	// RR case
	if ( balance < -1 && get_balance( root->rhs ) <= 0 )
		return rotate_lhs( root );

	// RL case
	if ( balance < -1 && get_balance( root->rhs ) >  0 ) {

		root->rhs = rotate_rhs( root->rhs );
		return rotate_lhs( root );
	}

	return root;
}

int main( int argc, char *argv[] ) {

	// Check the initial arguments
	// Check number of arguments
	if ( argc < 8 )
		exit_with_error( "Number of arguments are wrong" );

	// tvm379 pgsize tlbentries {g|p} quantum physpages {f|l} trace1 trace2 ...
	unsigned int       i,
	                   offset,
	                   result,
	                   file_number,
	                   counter_for_file,
	                   tlb_recorder        = 0,
	                   number_of_files     = argc - 7;
	char               input_bin[4],
	                   temp;
	unsigned long      pgsize              = atol( argv[1] ),
	                   tlbentries          = atol( argv[2] ),
	                   gp                  =      *argv[3],
	                   quantum             = atol( argv[4] ),
	                   physpages           = atol( argv[5] ),
	                   address             = 0,
	                   page_table_recorder = 0;
	unsigned long long references          = 0;
	struct LinkedList *current,
	                  *previous;
	bool               marker;
    fl                                     =      *argv[6];

	// Check if pgsize is a power of 2 or in range
	if ( pgsize < 16 || pgsize > 65536 || !POWOF2( pgsize ) )
		exit_with_error( "pagesize is not a power of 2 or not in 16~65536" );

    // Check tlbentries is a power of 2 or in range
	if ( tlbentries < 8 || tlbentries > 256 || !POWOF2( tlbentries ) )
        exit_with_error( "tlbentries is not a power of 2 or not in 8~256" );

    // Check gp is 'g' or 'p'
	if ( gp != 'g' && gp != 'p' )
        exit_with_error( "flag for TLB entries unknown" );

    // Check quantum is positive
	if ( quantum <= 0 )
        exit_with_error( "quantum is not a positive number" );

    // Check physpages is in range
	if ( physpages > 1000000 || physpages < tlbentries )
        exit_with_error( "physpages is not in tlbentries~1000000" );

    // Check fl is 'f' or 'l'
	if ( fl != 'f' && fl != 'l' )
        exit_with_error( "flag for page eviction policy unknown" );

	// Check the existances of all trace file
	for ( i = 0; i < number_of_files; i++ )
		if ( !check_file_existance( argv[i + 7] ) )
			exit_with_error( "trace file does not exist or permission denied" );

	// Initialize tlb table, page table, offest length and output counters
	tlbhits        = ( unsigned long long* )
	                     calloc( number_of_files, sizeof( long long ) );
	pf             = ( unsigned long long* )
	                     calloc( number_of_files, sizeof( long long ) );
	pageout        = ( unsigned long long* )
	                     calloc( number_of_files, sizeof( long long ) );
	avs_helper     = ( unsigned long long* )
	                     calloc( number_of_files, sizeof( long long ) );
	avs            = ( long double* )
	                     calloc( number_of_files, sizeof( long double ) );
	end_of_file    = ( bool* )
	                     calloc( number_of_files, sizeof( bool ) );
	page_table_avl = ( struct Node** )
	                     calloc( number_of_files, sizeof( struct Node* ) );
	trace          = ( FILE** )
	                     calloc( number_of_files, sizeof( FILE* ) );
	offset         = get_offset( pgsize );

	// Open all files
	for ( i = 0; i < number_of_files; i++ )
		trace[i] = fopen( argv[i + 7], "r" );

	// Start read file
	// Follow the Round Robin rule and each file read quantum number of address
	while ( keep_reading( number_of_files ) ) {

		// Rotate between each file
		for ( file_number = 0; file_number < number_of_files; file_number++ ) {

			if ( end_of_file[file_number] )
				continue;

			if ( gp == 'p' ) {

				current = tlb;
			
				while ( current != NULL ) {

					if ( tlb_recorder == 1 ) {

						free( tlb );
						tlb = NULL;
						break;
					}

					if ( current == tlb && tlb_recorder > 1 )
						tlb = current->next;

					if ( current->next != NULL )
						current->next->prev = current->prev;

					if ( current->prev != NULL )
						current->prev->next = current->next;

					previous = current;
					current  = current->next;
				
					free( previous );
					previous = NULL;
					tlb_recorder--;
				
				}
				
				tlb = NULL;
				tlb_recorder = 0;
			}

			// Read quantum number of addresses
			for ( long j = 0; j < quantum; j++ ) {

				// Read 4 * 8 bit = 32 bit from the file
				result = fread( input_bin, 1, 4, trace[file_number] );

				// Ignore the invalid ending bytes
				if ( result != 4 ) {
				
					//printf( "Here\n" );

					end_of_file[file_number] = true;
					fclose( trace[file_number] );

					// Clean TLB when a process is terminated
					current = tlb;

					while ( current != NULL ) {

						/*
						previous = tlb;
						while ( previous != NULL && previous->next != previous ) {
						
							printf( "%lu-%u -> ", previous->value, previous->process );
							previous = previous->next;
						}
						*/
						//printf( "\n%lu-%u\n", tlb->value, tlb->process );

						if ( current->process == file_number ) {

							if ( tlb_recorder == 1 ) {

								free( tlb );
								tlb = NULL;
								break;
							}

							if ( current == tlb && tlb_recorder > 1 )
								tlb = current->next;

							//printf( "\n%lu-%u\n", tlb->value, tlb->process );
							//printf( "Current Next: %lu-%u\n", current->next->next->value, current->next->next->process );

							if ( current->next != NULL )
								current->next->prev = current->prev;
							//printf( "Current: %lu-%u\n", current->value, current->process );
							//printf( "Current Next: %lu-%u\n", current->next->next->value, current->next->next->process );

							//if ( current->prev != NULL )
								//printf( "Current prev: %lu-%u\n", current->prev->value, current->prev->process );
							if ( current->prev != NULL )
								current->prev->next = current->next;
								
							//printf( "Current: %lu-%u\n", current->value, current->process );
							//printf( "Current Next: %lu-%u\n", current->next->next->value, current->next->next->process );

							previous = current;
							current  = current->next;

							//printf( "Current: %lu-%u\n", current->value, current->process );
							//printf( "Current Next: %lu-%u\n", current->next->value, current->next->process );
							
							free( previous );
							previous = NULL;
							tlb_recorder--;
							
							/*
							previous = tlb;
							while ( previous != NULL && previous->next != previous ) {
						
								printf( "%lu-%u -> ", previous->value, previous->process );
								previous = previous->next;
							}
							printf( "\n%lu-%u\n", tlb->value, tlb->process );
							*/
						}
					}

					//printf( "Error not in TLB cleaning\n" );

					// Clean page table when a process is terminated
					while ( page_table_avl[file_number] != NULL ) {

						/*
						printf( "%lu-%u\n",
                                         page_table_avl[file_number]->value,
						                 page_table_avl[file_number]->process );

						if ( page_table_recorder > 1 ) {

							struct Node *c   = page_table_tail;
							unsigned int cou = 0;

							while ( c != NULL && cou <= 10 ) {

								cou++;
								c = c->prev;
							}

							printf( "Length of the list is: %u\n", cou );

							if ( cou >= 6 ) {

								struct Node *c = page_table_tail;

								for( unsigned int ddd = 0; ddd < 5; ddd++ ){

									printf( "%lu-%u -> ", c->value, \
									                      c->process );
									c = c->prev;
								}

								c = page_table_head;
								printf("\n");

								for( unsigned int ddd = 0; ddd < 5; ddd++ ){

									printf( "%lu-%u <- ", c->value, \
									                      c->process );
									c = c->next;
								}
							}

							if ( page_table_head->next != NULL &&
								 page_table_head->prev == NULL ) {

								printf( "\nHead is correct, Head: %lu-%u \n",
									                 page_table_head->value,
									                 page_table_head->process );

								if ( page_table_tail->prev != NULL &&
									 page_table_tail->next == NULL )
									printf( "Tail is correct, Tail: %lu-%u\n",
										             page_table_tail->value,
										             page_table_tail->process );

								else {
									printf( "Tail failed. Tail is %lu-%u\n",
										             page_table_tail->value,
										             page_table_tail->process );
									printf( "%lu\n", page_table_recorder );
									exit(1);
								}
							} else {

								printf( "%lu\n", page_table_recorder );
								exit(1);
							}

						}
						*/
						

						if ( page_table_avl[file_number]->next != NULL )
							page_table_avl[file_number]->next->prev = 
							                  page_table_avl[file_number]->prev;

						if ( page_table_avl[file_number]->prev != NULL )
							page_table_avl[file_number]->prev->next = 
							                  page_table_avl[file_number]->next;

						if ( page_table_avl[file_number] == page_table_tail )
							page_table_tail = page_table_avl[file_number]->prev;

						if ( page_table_avl[file_number] == page_table_head )
							page_table_head = page_table_avl[file_number]->next;
						
						/*
						if ( page_table_head != NULL )
							printf( "Head: %lu-%u \n",
							                         page_table_head->value,
							                         page_table_head->process );

						if ( page_table_tail != NULL )
							printf( "Tail: %lu-%u\n",
							                         page_table_tail->value,
									                 page_table_tail->process );
						*/

						//debug = true;

						page_table_avl[file_number] = 
					                 delete( page_table_avl[file_number],
					                         page_table_avl[file_number]->value,
						                     false );

						//printf( "Finished\n" );
						//printf( "%lu\n", page_table_recorder - 1 );
						//printf("-----\n");

						page_table_recorder--;
						avs_helper[file_number]--;
					}

					if ( avs_helper[file_number] != 0 )
						printf( "Page Table is not cleaned\n" );

					debug = true;

					break;
				}

				references++;

				temp         = input_bin[0];
				input_bin[0] = input_bin[3];
				input_bin[3] = temp;
				temp         = input_bin[1];
				input_bin[1] = input_bin[2];
				input_bin[2] = temp;

				// Store in a 32 bit variable, remove the offset
				address = *(unsigned long*)input_bin>>offset;
				
				//printf( "%lu\n", address );

				// Try to hit TLB
				current  = tlb;
				previous = NULL;
				marker   = false;

				while ( current != NULL ) {

					if ( current->value   == address &&
					     current->process == file_number ) {

						// Add one on the tlbhits
						tlbhits[file_number]++;
						marker = true;

						// Update the list base on LRU
						if ( previous != NULL ) {

							// Take the hitted entry to the front
							previous->next          = current->next;

							if ( current->next != NULL )
								current->next->prev = previous;

							current->next           = tlb;
							current->prev           = NULL;

							if ( tlb != NULL )
								tlb->prev           = current;

							tlb                     = current;
						}
						break;
					}

					previous = current;
					current  = current->next;
				}

				// TLB hit
				if ( marker )
					continue;

				// TLB miss
				current = create_linked_list( address, file_number );

				//printf( "Value readed (want to add): %lu-%u\n",
				                                       //address, file_number );

				// Check if TLB is full
				if ( tlb_recorder == tlbentries ) {

					// Remove the least recent unit
					previous->prev->next = NULL;
					free( previous );
					previous = NULL;
					tlb_recorder--;
				}

				// Take the new entry to the front of TLB
				current->next = tlb;
				if ( tlb != NULL )
					tlb->prev = current;
				tlb           = current;
				tlb_recorder++;

				// Try to hit page table
				// Hash for the process
				page_table_hit              = false;
				page_table_avl[file_number] = 
				    insert( page_table_avl[file_number], address, file_number );

				// Take the linked list for page table with this address to the
				// front of the list (LRU).

				// Check if we hit the page table or we updated the table
				// Page table hit
				if ( page_table_hit ) {

					/*
					printf( "This new value is in the list\n" );

					if ( page_table_recorder < 9 && page_table_recorder > 1 ) {

						struct Node *c   = page_table_tail;
						unsigned int cou = 0;

						while ( c != NULL ) {

							cou++;
							c = c->prev;
						}

						printf( "Length of the list is: %u\n", cou );

						if ( cou >= 6 ) {

							struct Node *c = page_table_tail;

							for( unsigned int ddd = 0; ddd < 5; ddd++ ){

								printf( "%lu-%u -> ", c->value, c->process );
								c = c->prev;
							}

							c = page_table_head;
							printf("\n");

							for( unsigned int ddd = 0; ddd < 5; ddd++ ){

								printf( "%lu-%u <- ", c->value, c->process );
								c = c->next;
							}
						}

						if ( page_table_head->next != NULL &&
						     page_table_head->prev == NULL ) {

							printf( "\nHead is correct, Head: %lu-%u \n",
							                         page_table_head->value,
							                         page_table_head->process );

							if ( page_table_tail->prev != NULL &&
							     page_table_tail->next == NULL )
								printf( "Tail is correct, Tail: %lu-%u\n",
								                     page_table_tail->value,
								                     page_table_tail->process );

							else {
								printf( "Tail failed. Tail is %lu-%u\n",
								                     page_table_tail->value,
								                     page_table_tail->process );
								printf( "%lu\n", page_table_recorder );
								exit(1);
							}
						} else {

							printf( "%lu\n", page_table_recorder );
							exit(1);
						}

					}

					printf("-----\n");
					*/

					continue;
				}

				// Page table miss
				pf[file_number]++;
				avs_helper[file_number]++;

				if ( page_table_recorder == 0 )
					page_table_tail = page_table_avl[file_number];

				page_table_recorder++;

				// Remove one entry by FIFO or LRU
				if ( page_table_recorder > physpages ) {

					//printf( "Head is %lu\n", page_table_head->value );
					//printf( "Try to remove: %lu\n", page_table_tail->value );

					// Add one on pageout of the removed process
					pageout[page_table_tail->process]++;

					//printf("%lu", page_table_tail->value);

					avs_helper[page_table_tail->process]--;

					page_table_avl[page_table_tail->process] =
					           delete( page_table_avl[page_table_tail->process],
					                   page_table_tail->value,
					                   true );

					//printf( "Head is %lu\n", page_table_head->value );
					//printf( "Tail is %lu\n", page_table_tail->value );

					page_table_recorder--;
				}

				//printf( "%llu\n", avs_helper[0] + avs_helper[1] );

				if ( page_table_tail != NULL && page_table_tail->prev != NULL )
					page_table_tail = page_table_tail->prev->next;

				/*
				if ( page_table_recorder < 9 && page_table_recorder > 1 ) {

					struct Node *c  = page_table_tail;
					unsigned int cou = 0;

					while ( c != NULL ) {

						cou++;
						c = c->prev;
					}

					printf( "Length of the list is: %u\n", cou );

					if ( cou >= 6 ) {

						struct Node *c = page_table_tail;

						for( unsigned int ddd = 0; ddd < 5; ddd++ ){

							printf( "%lu-%u -> ", c->value, c->process );
							c = c->prev;
						}

						c = page_table_head;
						printf("\n");

						for( unsigned int ddd = 0; ddd < 5; ddd++ ){

							printf( "%lu-%u <- ", c->value, c->process );
							c = c->next;
						}
					}

					if ( page_table_head->next != NULL && 
					     page_table_head->prev == NULL ) {

						printf( "\nHead is correct, Head: %lu-%u \n",
						     page_table_head->value, page_table_head->process );

						if ( page_table_tail->prev != NULL &&
						     page_table_tail->next == NULL )
							printf( "Tail is correct, Tail: %lu-%u\n",
							                         page_table_tail->value,
							                         page_table_tail->process );

						else {

							printf( "Tail failed. Tail is %lu-%u\n",
							                         page_table_tail->value,
							                         page_table_tail->process );
							printf( "%lu\n", page_table_recorder );
							exit(1);
						}
					} else {

						printf( "%lu\n", page_table_recorder );
						exit(1);
					}

				}

				printf("\n-----\n");
				*/

				for ( counter_for_file = 0;
				      counter_for_file < number_of_files;
				      counter_for_file++ )
					avs[counter_for_file] += ( avs_helper[counter_for_file] -
					                           avs[counter_for_file] ) /
					                         ( references + 1 );
			}
		}
	}

	// Finished reading trace files
	// Print out the result to stdout
	for ( file_number = 0; file_number < number_of_files; file_number++ )
		printf( "%llu %llu %llu %Lf\n", tlbhits[file_number],
		                                pf[file_number],
		                                pageout[file_number],
		                                avs[file_number] );
}
