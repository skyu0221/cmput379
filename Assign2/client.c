#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define	 MY_PORT  2222

void send_query(int s, FILE *keyfile);

/* ---------------------------------------------------------------------
 This is a sample client program for the number server. The client and
 the server need not run on the same machine.
 --------------------------------------------------------------------- */

int main(int argc, char * argv[])
{
	FILE * keyfile;
	if(argc==4){
		//hostname, portnumber
		keyfile=fopen(argv[3], "r+");
	}
	else{
		keyfile=NULL;
	}

	int	s, number;

	struct	sockaddr_in	server;

	struct	hostent		*host;

	host = gethostbyname ("localhost");

	if (host == NULL) {
		perror ("Client: cannot get host description");
		exit (1);
	}

	s = socket (AF_INET, SOCK_STREAM, 0);

	if (s < 0) {
		perror ("Client: cannot open socket");
		exit (1);
	}

	bzero (&server, sizeof (server));
	bcopy (host->h_addr, & (server.sin_addr), host->h_length);
	server.sin_family = AF_INET;//host->h_addrtype;
	server.sin_addr.s_addr=inet_addr(argv[1]);
	server.sin_port = htons (atoi(argv[2]));

	if (connect (s, (struct sockaddr*) & server, sizeof (server))) {
		perror ("Client: cannot connect to server");
		exit (1);
	}

	read (s, &number, sizeof (number));
	close (s);
	fprintf (stderr, "Process %d gets number %d\n", getpid (),
		ntohl (number));
	sleep (2);
	
}
