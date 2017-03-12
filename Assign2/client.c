#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	 MY_PORT  2222

#define BUFFER 999999

/* ---------------------------------------------------------------------
 This is a sample client program for the number server. The client and
 the server need not run on the same machine.
 --------------------------------------------------------------------- */

int main() {

	int	                 sock,
	                     number;
	char                 message[BUFFER],
	                     buff_send[BUFFER] = {0};
	struct	sockaddr_in	 serv_addr;
	struct	hostent		*host;

	host = gethostbyname( "localhost" );

	if ( host == NULL ) {

		perror( "Client: cannot get host description" );
		exit (1);
	}


	sock = socket( AF_INET, SOCK_STREAM, 0 );

	if ( sock < 0 ) {

		perror( "Client: cannot open socket" );
		exit(1);
	}

	//serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	bzero( &serv_addr, sizeof( serv_addr ) );
	bcopy( host->h_addr, &( serv_addr.sin_addr ), host->h_length );

	serv_addr.sin_family = host->h_addrtype;
	serv_addr.sin_port   = htons(MY_PORT);

	if ( connect( sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr) ) ){

		perror( "Client: cannot connect to server" );
		exit(1);
	}

	while ( strlen( message ) == 0  )
		recv( sock, message, sizeof( message ), 0 );

	printf( "%s", message );

	while (1) {

		memset( message, 0, sizeof( message ) );
		memset( buff_send, 0, sizeof( buff_send ) );

		printf( "What is the command: " );
		scanf( "%s", buff_send );

		write( sock, buff_send, strlen( buff_send ) );

		if ( buff_send[0] == '@' ) {

			memset( buff_send, 0, sizeof( buff_send ) );
			printf( "What is new string?: " );
			scanf( "%s", buff_send );
			write( sock, buff_send, strlen( buff_send ) );
		}

		while ( strlen( message ) == 0 )
			read( sock, message, sizeof( message ) );

		printf( "%s", message );

		//close( sock );
	}

	return 0;
}
