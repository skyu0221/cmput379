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

#define	MY_PORT	2222

#define BUFFER 999999



int main( int argc, char *argv[] ) {


	int	               serv_sock,
	                   clnt_sock,
	                   opt,
	                   request_entry,
	                   request_length,
	                   test1 = 0,
	                   test2 = 0;
    unsigned long long num_entry = 0,
	                   iterator = 0;
	bool               read_from_file;
	struct sockaddr_in serv_addr,
	                   clnt_addr;
	ssize_t            reader;
	socklen_t          clnt_addr_size = sizeof( clnt_addr );

	if ( argc != 4 ) {

		perror( "Server: Number of arguments are wrong" );
		exit(1);
	}

	while ( ( opt = getopt( argc, argv, "fn" ) ) != -1 ) {

		switch ( opt ) {

			case 'f': read_from_file = true;  break;
			case 'n': read_from_file = false; break;
			default:
				perror( "Server: Cannot find the flag. USAGE(-f,-n)" );
				exit(1);
		}
	}

	if ( read_from_file ) {

		FILE               *fp;
		char               *line = NULL;
		size_t              len = 0;

		fp = fopen( argv[3], "r" );

		if ( fp == NULL ) {

			perror( "Server: cannot find the file" );
			exit(1);
		}

		while ( ( reader = getline( &line, &len, fp ) ) != -1 )
			num_entry++;

		fclose( fp );

	} else
		test2 = atoi( argv[3] );
		
	test1 = test2 + num_entry;

	char entry[num_entry][BUFFER];
	char type[num_entry];

	memset( entry, 0, sizeof( entry ) );
	memset( type, 0, sizeof( type ) );

	if ( read_from_file ) {

		FILE               *fp;
		char               *line = NULL;
		size_t              len = 0;

		fp = fopen( argv[3], "r" );

		for ( ;iterator < num_entry; iterator++ ) {
			getline( &line, &len, fp );
			strcpy( entry[iterator], &line[2] );
			type[iterator] = line[0];
			entry[iterator][strlen( entry[iterator] ) - 1] = '\0';
		}

		fclose( fp );

	}

	serv_sock = socket( AF_INET, SOCK_STREAM, 0 );

	if ( serv_sock < 0 ) {

		perror( "Server: cannot open master socket" );
		exit(1);
	}

	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port        = htons( atoi( argv[2] ) );

	if ( bind( serv_sock, (struct sockaddr*)&serv_addr, sizeof( serv_addr ) ) ){

		perror( "Server: cannot bind master socket" );
		exit(1);
	}
	while ( true ) {
	listen( serv_sock, 5 );

	clnt_sock = accept( serv_sock, (struct sockaddr*)&clnt_addr,
	                    &clnt_addr_size );

	char message[BUFFER] = {0};

	sprintf( message, "CMPUT379 Whiteboard Server v0\n%lld\n", num_entry );

	char request[BUFFER], request_entry_char[BUFFER];

	write( clnt_sock, message, sizeof( message ) );

	while ( true ) {

		memset( request, 0, sizeof( request ) );
		memset( message, 0, sizeof( message ) );
		memset( request_entry_char, 0, sizeof( request_entry_char ) );
		recv( clnt_sock, request, sizeof(request), 0 );
		
		printf( "request: %s\n", request );

		if ( request[0] != '?' && request[0] != '@' ) {

			sprintf( message, "Command illegal" );
			write( clnt_sock, message, sizeof( message ) );
		}

		iterator = 1;

		while ( request[iterator] != '\0' && request[iterator] != 'p' &&
		                                     request[iterator] != 'c' ) {

			request_entry_char[iterator - 1] = request[iterator];
			iterator++;
		}

		request_entry_char[iterator - 1] = '\0';

		request_entry = atoi( request_entry_char );

		if ( request_entry > num_entry || request_entry <= 0 )
			sprintf( message, "!%de14\nNo such entry!\n", request_entry );
		else if ( request[0] == '?' )
			sprintf( message, "!%d%c%d\n%s\n", request_entry,
			                                      type[request_entry - 1],
			                             strlen( entry[request_entry - 1] ),
			                                     entry[request_entry - 1] );
		else if ( request[0] == '@' ) {

			type[request_entry - 1] = request[iterator];
			iterator++;
			
			char temp[BUFFER] = {0};
			int  temp_i       = 0;
			
			while ( request[iterator] != '\n' ) {

				temp[temp_i] = request[iterator];
				iterator++;
				temp_i++;
			}
			
			temp[temp_i] = '\0';

			request_length = atoi( temp );
			
			iterator++;
			
			memset( entry[request_entry - 1], 0, 
			        sizeof( entry[request_entry - 1] ) );

			strncpy( entry[request_entry - 1], &request[iterator], request_length);
			sprintf( message, "!%de0\n\n", request_entry );
		}

		send( clnt_sock, message, strlen( message ), 0 );
		printf( "message: %s\n%zu\n", message, strlen( message ) );

	}

	close( clnt_sock );
	close( serv_sock );
	}

	return 0;
}
