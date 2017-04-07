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

bool check_file_existance( char* file_name ) {

	FILE *file = fopen( file_name, "r" );

	if ( file ) {

		fclose( file );
		return true;
	}

	return false;
}

int get_offset( int number ) {

	if ( number == 1 )
		return 0;

	return 1 + get_offset( number >> 1 );
}

struct Node {

	long         value;
    int          height;
	struct Node *lhs,
	            *rhs;
};

struct Node* create_node( long value ) {

	struct Node *node = ( struct Node* ) malloc( sizeof( struct Node ) );

	node->value  = value;
	node->lhs    = NULL;
	node->rhs    = NULL;
	node->height = 1;

	return node;
}

int get_height( struct Node *node ) {

	if ( node == NULL )
		return 0;

	return node->height;
}

int get_balance( struct Node *node ) {

	if ( node == NULL )
		return 0;

	return get_height( node->lhs ) - get_height( node->rhs );
}

struct Node* rotate_rhs( struct Node *root ) {

	struct Node *lhs = root->lhs,
	            *l_r = root->lhs->rhs;

	lhs->rhs  = root;
	root->lhs = l_r;

	root->height = MAX( get_height( root->lhs ), get_height( root->rhs ) ) + 1;
	lhs->height  = MAX( get_height(  lhs->lhs ), get_height(  lhs->rhs ) ) + 1;

	return lhs;
}

struct Node* rotate_lhs( struct Node *root ) {

	struct Node *rhs = root->rhs,
	            *r_l = root->rhs->lhs;

	rhs->lhs  = root;
	root->rhs = r_l;

	root->height = MAX( get_height( root->lhs ), get_height( root->rhs ) ) + 1;
	rhs->height  = MAX( get_height(  rhs->lhs ), get_height(  rhs->rhs ) ) + 1;

	return rhs;
}

struct Node* min_value( struct Node *node ) {

	struct Node *current = node;

	while ( current->lhs != NULL )
		current = current->lhs;

	return current;
}

struct Node* insert( struct Node *node, long value ) {

	if ( node == NULL )
		return create_node( value );

	if ( value > node->value )
		node->rhs = insert( node->rhs, value );

	else if ( value < node->value )
		node->lhs = insert( node->lhs, value );

	else
		return node;

	int balance;

	node->height = MAX( get_height( node->lhs ), get_height( node->rhs ) ) + 1;
	balance      = get_balance( node );

	if ( balance >  1 && value < node->lhs->value )
		return rotate_rhs( node );

	if ( balance >  1 && value > node->lhs->value ) {

		node->lhs = rotate_lhs( node->lhs );
		return rotate_rhs( node );
	}

	if ( balance < -1 && value > node->rhs->value )
		return rotate_lhs( node );

	if ( balance < -1 && value < node->rhs->value ) {

		node->rhs = rotate_rhs( node->rhs );
		return rotate_lhs( node );
	}

	return node;
}

struct Node *delete( struct Node *root, long value ) {

	if ( root == NULL )
		return root;

	if ( value > root->value )
		root->rhs = delete( root->rhs, value );

	else if ( value < root->value )
		root->lhs = delete( root->lhs, value );

	else {

		if ( ( root->lhs == NULL ) || ( root->rhs == NULL ) ) {

			struct Node *temp = root->lhs ? root->lhs : root->rhs;

			if ( temp == NULL ) {

				temp = root;
				root = NULL;
			} else
				*root = *temp;

			free( temp );
		} else {

			struct Node *temp = min_value( root->rhs );

			root->value = temp->value;
			root->rhs   = delete( root->rhs, temp->value );
		}
	}

	if ( root == NULL )
		return root;

	int balance;

	root->height = MAX( get_height( root->lhs ), get_height( root->rhs ) ) + 1;
	balance      = get_balance( root );

	if ( balance >  1 && get_balance( root->lhs ) >= 0 )
		return rotate_rhs( root );

	if ( balance >  1 && get_balance( root->lhs ) <  0 ) {

		root->lhs = rotate_lhs( root->lhs );
		return rotate_rhs( root );
	}

	if ( balance < -1 && get_balance( root->lhs ) <= 0 )
		return rotate_lhs( root );

	if ( balance < -1 && get_balance( root->lhs ) >  0 ) {

		root->rhs = rotate_rhs( root->rhs );
		return rotate_lhs( root );
	}

	return root;
}

int main( int argc, char *argv[] ) {

	// check the initial arguments
	// check number of arguments
	if ( argc < 8 )
		exit_with_error( "Number of arguments are wrong" );

	//tvm379 pgsize tlbentries {g|p} quantum physpages {f|l} trace1 trace2 ...
	int  i,
	     offset;
	long pgsize     = atol( argv[1] ),
	     tlbentries = atol( argv[2] ),
	     gp         =      *argv[3],
	     quantum    = atol( argv[4] ),
	     physpages  = atol( argv[5] ),
	     fl         =      *argv[6];

	// check if pgsize is a power of 2 or in range
	if ( pgsize < 16 || pgsize > 65536 || !POWOF2( pgsize ) )
		exit_with_error( "pagesize is not a power of 2 or not in 16~65536" );

    // check tlbentries is a power of 2 or in range
	if ( tlbentries < 8 || tlbentries > 256 || !POWOF2( tlbentries ) )
        exit_with_error( "tlbentries is not a power of 2 or not in 8~256" );

    // check gp is 'g' or 'p'
	if ( gp != 'g' && gp != 'p' )
        exit_with_error( "flag for TLB entries unknown" );

    // check quantum is positive
	if ( quantum <= 0 )
        exit_with_error( "quantum is not a positive number" );

    // check physpages is in range
	if ( physpages > 1000000 || physpages < 0 )
        exit_with_error( "physpages is not in 0~1000000" );

    // check fl is 'f' or 'l'
	if ( fl != 'f' && fl != 'l' )
        exit_with_error( "flag for page eviction policy unknown" );

	// check the existances of all trace file
	for ( i = 7; i < argc; i++ )
		if ( !check_file_existance( argv[i] ) )
			exit_with_error( "trace file does not exist or permission denied" );

	// initialize tlb table and offest length
	tlb    = (long *) calloc( tlbentries, sizeof( long ) );
	offset = get_offset( pgsize );
}
