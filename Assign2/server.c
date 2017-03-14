#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>

#define	MY_PORT	2222
#define BUFFER 999999

// entry information
char               **entry;
char                *type;
sigset_t             mask,
                     default_mask;
unsigned long long   num_entry = 0,
                     iterator = 0;
int	               serv_sock,
                   opt,
                   request_entry,
                   request_length,
                   test1 = 0,
                   test2 = 0;
pid_t              pid = 0;
pid_t              sid = 0;
bool               read_from_file;
struct sockaddr_in serv_addr,
                   clnt_addr;
ssize_t            reader;
socklen_t          clnt_addr_size = sizeof( clnt_addr );
pthread_t          tid;


// handle SIGTERM. want to save all value and free all memory and quit
void handler ( int sig ) {

	// reset signal
	sigprocmask( SIG_SETMASK, &default_mask, &mask );
	signal( SIGINT, SIG_DFL );

	FILE               *fp;
	char               *line = NULL,
	                   *temp_content;
	size_t              len = 0;

	// write current memory to file
	fp = fopen( "whiteboard.all", "w" );

	if ( fp == NULL ) {

		printf( "Server: cannot create the file 'whiteboard.all'\n" );
		exit(1);
	}

	// recode current entries
	for ( iterator = 0; iterator < num_entry; iterator++ ) {

		fprintf( fp, "%c %s\n", type[iterator], entry[iterator] );
		free( entry[iterator] );
	}

	// free all malloced memory
	free( entry );
	free( type );

	fclose( fp );

	// quit the program
	exit(1);
}

void *process_message( void *vargp ) {

	// get sock number
	int clnt_sock = (int)vargp;

	char message[BUFFER] = {0};
	// for the connection, give a greeting message
	sprintf( message, "CMPUT379 Whiteboard Server v0\n%lld\n", num_entry );

	char request[BUFFER], request_entry_char[BUFFER];

	write( clnt_sock, message, sizeof( message ) );

	// memset to all values
	memset( request, 0, sizeof( request ) );
	memset( message, 0, sizeof( message ) );
	memset( request_entry_char, 0, sizeof( request_entry_char ) );
	recv( clnt_sock, request, sizeof(request), 0 );

	// don't close the socket until the client close it
	while ( strlen( request ) != 0 ) {
		printf( "request: %s\n", request );
		// check illegal request
		if ( request[0] != '?' && request[0] != '@' ) {

			sprintf( message, "Command illegal" );
			write( clnt_sock, message, sizeof( message ) );
		}

		iterator = 1;
		// read the entry number
		while ( request[iterator] != '\0' && request[iterator] != 'p' &&
				                             request[iterator] != 'c' ) {

			request_entry_char[iterator - 1] = request[iterator];
			iterator++;
		}

		request_entry_char[iterator - 1] = '\0';
		// read the entry number, convert to int
		request_entry = atoi( request_entry_char );
		// check if the entry is legal
		if ( request_entry > num_entry || request_entry <= 0 )
			sprintf( message, "!%de14\nNo such entry!\n", request_entry );
		else if ( request[0] == '?' )
			sprintf( message, "!%d%c%d\n%s\n", request_entry,
					                              type[request_entry - 1],
					                     strlen( entry[request_entry - 1] ),
					                             entry[request_entry - 1] );
		else if ( request[0] == '@' ) {
			//get the type of new entry
			type[request_entry - 1] = request[iterator];
			iterator++;
	
			char  temp[BUFFER] = {0},
			     *temp_content;
			int   temp_i       = 0;
			//get the length of entry if gonna update the entry
			while ( request[iterator] != '\n' ) {

				temp[temp_i] = request[iterator];
				iterator++;
				temp_i++;
			}
	
			temp[temp_i] = '\0';
			// get the length to int
			request_length = atoi( temp );
	
			iterator++;
	
			memset( entry[request_entry - 1], 0, 
					sizeof( entry[request_entry - 1] ) );
			// get content
			temp_content = realloc( entry[request_entry - 1], request_length *sizeof( char ) );
			strncpy( temp_content, &request[iterator], request_length );
			entry[request_entry - 1] = temp_content;
			sprintf( message, "!%de0\n\n", request_entry );
		}
		// give respond
		send( clnt_sock, message, strlen( message ), 0 );
		printf( "message: %s\n%zu\n", message, strlen( message ) );

		// ready for the next command
		memset( request, 0, sizeof( request ) );
		memset( message, 0, sizeof( message ) );
		memset( request_entry_char, 0, sizeof( request_entry_char ) );
		recv( clnt_sock, request, sizeof(request), 0 );
	}
	close( clnt_sock );
}

int main( int argc, char *argv[]) {

	int clnt_sock;

	sigfillset( &mask );
	sigdelset( &mask, SIGTERM );
	sigprocmask( SIG_SETMASK, &mask, &default_mask );

	struct sigaction act;

	act.sa_handler = handler;
	act.sa_flags   = 0;
	sigemptyset( &act.sa_mask );
	sigaction( SIGTERM, &act, 0 );

	printf( "Main Server is running on PID: %d\n", getpid() );

	//  Check the number of arguments
	if ( argc != 4 ) {

		printf( "Server: Number of arguments are wrong\n" );
		exit(1);
	}

	// check if a statefile is provided or not
	while ( ( opt = getopt( argc, argv, "fn" ) ) != -1 ) {

		switch ( opt ) {

			case 'f': read_from_file = true;  break;
			case 'n': read_from_file = false; break;
			default:
				printf( "Server: Cannot find the flag. USAGE(-f,-n)\n" );
				exit(1);
		}
	}

	// if statefile is provided, read the content
	if ( read_from_file ) {

		FILE               *fp;
		char               *line = NULL,
		                   *temp_content;
		size_t              len = 0;

		fp = fopen( argv[3], "r" );

		if ( fp == NULL ) {

			printf( "Server: cannot find the file\n" );
			exit(1);
		}

		while ( ( reader = getline( &line, &len, fp ) ) != -1 ) {

			if ( num_entry == 0 ) {
				entry = malloc( sizeof( char* ) );
				type = (char*) malloc( sizeof( char ) );
			}
			else {
				entry = realloc( entry, ( num_entry + 1 ) * sizeof( char* ) );
				type = realloc( type, ( num_entry + 1 ) * sizeof( char ) );
			}
			temp_content = malloc( strlen( &line[2] ) *sizeof( char ) );
			strcpy( temp_content, &line[2] );
			type[num_entry] = line[0];
			entry[num_entry] = temp_content;
			entry[num_entry][strlen( entry[num_entry] ) - 1] = '\0';
			num_entry++;
		}

		fclose( fp );

	} else {

		char *temp_content;

		num_entry = atoi(argv[3]);
		entry = malloc( num_entry * sizeof( char* ) );
		type = (char*) malloc( num_entry * sizeof( char ) );

		for ( iterator = 0; iterator < num_entry; iterator++ ) {

			temp_content = malloc( 0 * sizeof( char ) );
			entry[iterator] = temp_content;
			type[iterator] = 'p';
		}
	}

	// Prepare for the connection.
	// create Daemon process

	pid = fork();

	if ( pid < 0 ) {

		printf( "fork failed!\n" );
		exit(1);
	}

	if ( pid > 0 ) {
    	// in the parent
		printf( "pid of child process %d \n", pid );
		exit(0);
    }

	serv_sock = socket( AF_INET, SOCK_STREAM, 0 );

	if ( serv_sock < 0 ) {

		printf( "Server: cannot open master socket\n" );
		exit(1);
	}

	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port        = htons( atoi( argv[2] ) );

	if ( bind( serv_sock, (struct sockaddr*)&serv_addr, sizeof( serv_addr ) ) ){

		printf( "Server: cannot bind master socket\n" );
		exit(1);
	}

	listen( serv_sock, 5 );

	while ( true ) {

		clnt_sock = accept( serv_sock, (struct sockaddr*)&clnt_addr,
	                    &clnt_addr_size );

		pthread_create( &tid, NULL, process_message, (void *)clnt_sock );
	}

	return 0;
}
