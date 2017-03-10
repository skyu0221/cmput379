#include "client.h"
/*#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define	 MY_PORT  2222
*/
//void send_query(int s, FILE *keyfile);


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
	char greeting[10000], user_choice[5];
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

	recv(s, greeting, sizeof(greeting),0);
	printf("%s",greeting);

	while(1){
		printf("Pick an action: \n");
		printf("1. Send Query\n");
		printf("2. Update Entry with Encryption\n");
		printf("3. Update Entry without Encryption\n");
		printf("4. Clean Entry\n");
		printf("5. Exit\n");

		fgets(user_choice, sizeof(user_choice),stdin);
		if(strncmp(user_choice, "1",1)==0){
			//send query
		}
		if(strncmp(user_choice, "2",1)==0){
			//update with encryption
		}
		if(strncmp(user_choice, "3",1)==0){
			//update without encryption
		}
		if(strncmp(user_choice, "4",1)==0){
			//clean entry
		}
		if(strncmp(user_choice, "5",1)==0){
			close(s);
			return 1;
		}
	}
	
}

void send_query(int s, FILE * keyfile){
	char query[10000];
	printf("entry of whiteboard: \n");
	fgets(query,sizeof(query),stdin);
}

void clean_entry(int s){

}

void plaintextEntry(int s){

}

void encryptedEntry(int s, FILE *keyfile){

}

int encrypt(){

}

int decrypt(){

}

char *base64encode(){

}

char *base64decode(){
	
}
