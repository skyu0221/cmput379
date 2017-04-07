#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#define POWOF2( n ) ( ( n & n - 1 ) == 0 )

long *tlb,
     *page_table;

void exit_with_error( char* error_message ) {

	printf( "Error: %s\n", error_message );
	free( tlb );
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

// Calculate the offset of address
int get_offset( int number ) {

	if ( number == 1 )
		return 0;
	// Calculate the log2(pgsize)
	return 1 + get_offset( number >> 1 );
}

struct Node {

	long         value;
    int          height;
	struct Node *lhs,
	            *rhs;
};

// Create and initial a node with given value
struct Node* create_node( long value ) {

	struct Node *node = ( struct Node* ) malloc( sizeof( struct Node ) );

	node->value  = value;
	node->lhs    = NULL;
	node->rhs    = NULL;
	node->height = 1;

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
struct Node* insert( struct Node *root, long value ) {

	// Check for the empty tree
	if ( root == NULL )
		return create_node( value );

	// If value is greater than root, target position is on the right
	if ( value > root->value )
		root->rhs = insert( root->rhs, value );

	// If value is smaller than root, target position is on the left
	else if ( value < root->value )
		root->lhs = insert( root->lhs, value );

	// If value is the root, no need for insertion (page table hit)
	else
		return root;

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
struct Node *delete( struct Node *root, long value ) {

	// Check for the empty tree
	if ( root == NULL )
		return root;

	// If value is greater than root, target is on the right
	if ( value > root->value )
		root->rhs = delete( root->rhs, value );

	// If value is smaller than root, target is on the left
	else if ( value < root->value )
		root->lhs = delete( root->lhs, value );

	// If value is the root, remove the root
	else {

		// Check if root has less than two children
		if ( ( root->lhs == NULL ) || ( root->rhs == NULL ) ) {

			struct Node *temp = root->lhs ? root->lhs : root->rhs;

			// If no child
			if ( temp == NULL ) {

				temp = root;
				root = NULL;
			} else
				*root = *temp;

			free( temp );

		} else {

			// Get the inorder successor
			struct Node *temp = min_value( root->rhs );

			root->value = temp->value;
			root->rhs   = delete( root->rhs, temp->value );
		}
	}

	if ( root == NULL )
		return root;

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
	if ( balance < -1 && get_balance( root->lhs ) <= 0 )
		return rotate_lhs( root );

	// RL case
	if ( balance < -1 && get_balance( root->lhs ) >  0 ) {

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
	int  i,
	     offset;
	long pgsize     = atol( argv[1] ),
	     tlbentries = atol( argv[2] ),
	     gp         =      *argv[3],
	     quantum    = atol( argv[4] ),
	     physpages  = atol( argv[5] ),
	     fl         =      *argv[6];

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
	if ( physpages > 1000000 || physpages < 0 )
        exit_with_error( "physpages is not in 0~1000000" );

    // Check fl is 'f' or 'l'
	if ( fl != 'f' && fl != 'l' )
        exit_with_error( "flag for page eviction policy unknown" );

	// Check the existances of all trace file
	for ( i = 7; i < argc; i++ )
		if ( !check_file_existance( argv[i] ) )
			exit_with_error( "trace file does not exist or permission denied" );

	// Initialize tlb table and offest length
	tlb    = (long *) calloc( tlbentries, sizeof( long ) );
	offset = get_offset( pgsize );
}
