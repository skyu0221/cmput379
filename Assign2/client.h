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
void clean_entry(int s);
void plaintextEntry(int s);
void encryptedEntry(int s, FILE *keyfile);
int encrypt();
int decrypt();
char *base64encode();
char *base64decode();
