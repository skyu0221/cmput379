#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#define	 MY_PORT  2222

#define BUFFER 999999
#define MIN(a,b) (((a)<(b))?(a):(b))

struct enncodingkey {

	unsigned char salt[16];
	unsigned char key[64];
	unsigned char iv[32];
};

struct  enncodingkey default_keys;

// copyed from eclass
char *base64encode (const void *b64_encode_this, int encode_this_many_bytes){
    BIO *b64_bio, *mem_bio;      //Declares two OpenSSL BIOs: a base64 filter and a memory BIO.
    BUF_MEM *mem_bio_mem_ptr;    //Pointer to a "memory BIO" structure holding our base64 data.
    b64_bio = BIO_new(BIO_f_base64());                      //Initialize our base64 filter BIO.
    mem_bio = BIO_new(BIO_s_mem());                           //Initialize our memory sink BIO.
    BIO_push(b64_bio, mem_bio);            //Link the BIOs by creating a filter-sink BIO chain.
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);  //No newlines every 64 characters or less.
    BIO_write(b64_bio, b64_encode_this, encode_this_many_bytes); //Records base64 encoded data.
    BIO_flush(b64_bio);   //Flush data.  Necessary for b64 encoding, because of pad characters.
    BIO_get_mem_ptr(mem_bio, &mem_bio_mem_ptr);  //Store address of mem_bio's memory structure.
    BIO_set_close(mem_bio, BIO_NOCLOSE);   //Permit access to mem_ptr after BIOs are destroyed.
    BIO_free_all(b64_bio);  //Destroys all BIOs in chain, starting with b64 (i.e. the 1st one).
    BUF_MEM_grow(mem_bio_mem_ptr, (*mem_bio_mem_ptr).length + 1);   //Makes space for end null.
    (*mem_bio_mem_ptr).data[(*mem_bio_mem_ptr).length] = '\0';  //Adds null-terminator to tail.
    return (*mem_bio_mem_ptr).data; //Returns base-64 encoded data. (See: "buf_mem_st" struct).
}
// copyed from eclass
char *base64decode (const void *b64_decode_this, int decode_this_many_bytes){
    BIO *b64_bio, *mem_bio;      //Declares two OpenSSL BIOs: a base64 filter and a memory BIO.
    char *base64_decoded = calloc( (decode_this_many_bytes*3)/4+1, sizeof(char) ); //+1 = null.
    b64_bio = BIO_new(BIO_f_base64());                      //Initialize our base64 filter BIO.
    mem_bio = BIO_new(BIO_s_mem());                         //Initialize our memory source BIO.
    BIO_write(mem_bio, b64_decode_this, decode_this_many_bytes); //Base64 data saved in source.
    BIO_push(b64_bio, mem_bio);          //Link the BIOs by creating a filter-source BIO chain.
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);          //Don't require trailing newlines.
    int decoded_byte_index = 0;   //Index where the next base64_decoded byte should be written.
    while ( 0 < BIO_read(b64_bio, base64_decoded+decoded_byte_index, 1) ){ //Read byte-by-byte.
        decoded_byte_index++; //Increment the index until read of BIO decoded data is complete.
    } //Once we're done reading decoded data, BIO_read returns -1 even though there's no error.
    BIO_free_all(b64_bio);  //Destroys all BIOs in chain, starting with b64 (i.e. the 1st one).
    return base64_decoded;        //Returns base-64 decoded data with trailing null terminator.
}
// copyed from eclass
int encoding(char* request, char *test) {

	unsigned char outbuf[BUFFER];
	int outlen, tmplen;
	EVP_CIPHER_CTX ctx;

	unsigned char intext[BUFFER];
	bzero( intext, BUFFER );
	
	strcpy( intext, request );
	
	intext[strlen( intext ) - 1] = 0;

	EVP_CIPHER_CTX_init(&ctx);
	EVP_EncryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, default_keys.key, default_keys.iv);

	if(!EVP_EncryptUpdate(&ctx, outbuf, &outlen, intext, strlen(intext)))
		return 0;

	if(!EVP_EncryptFinal_ex(&ctx, outbuf + outlen, &tmplen))
		return 0;

	outlen += tmplen;
	EVP_CIPHER_CTX_cleanup(&ctx);

	unsigned char *data_to_encode;
	data_to_encode = malloc( outlen );
	memcpy( data_to_encode, outbuf, outlen );
	int bytes_to_encode = outlen;
	
	//Base-64 encoding.
	unsigned char *base64_encoded = base64encode( data_to_encode, bytes_to_encode );
	strcpy( test, base64_encoded );
	free(base64_encoded);
	return 1;
}
// copyed from eclass
int decoding( char* request, char *test ) {

	unsigned char debuf[1024];
	int delen, tmplen;
	EVP_CIPHER_CTX ctx;

	unsigned char data_to_decode[200];
	bzero( data_to_decode, 200 );
	char buffer[200];
	sprintf( buffer, "Enter a string to be decrypted\n" );
	write( 1, buffer, strlen( buffer ) );
	read( 0, data_to_decode, 199 );
	
	memset( debuf, 0, sizeof( debuf ) );
	
	data_to_decode[strlen( data_to_decode ) - 1] = 0;
	
	int bytes_to_decode = strlen( data_to_decode );
	
	//Base-64 encoding.
	unsigned char *base64_decoded = base64decode( data_to_decode, bytes_to_decode );
	
	delen = strlen(base64_decoded);

	EVP_CIPHER_CTX_init(&ctx);
	EVP_DecryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, default_keys.key, default_keys.iv);
    
	if(!EVP_DecryptUpdate(&ctx, debuf, &delen, base64_decoded, delen))
		return 0;

	int remainingBytes;
    
	if(!EVP_DecryptFinal_ex(&ctx, debuf + delen, &remainingBytes))
		return 0;
	
	delen += remainingBytes;
	EVP_CIPHER_CTX_cleanup(&ctx);
	
	strncpy( test, debuf, delen );
	free(base64_decoded);
	return 1;
}

int main( int argc, char *argv[] ) {

	int	                  sock,
	                      number,
	                      selector,
	                      input_length,
	                      current;
	char                  message[BUFFER],
	                      buff_send[BUFFER] = {0},
	                      buffer[BUFFER],
	                      encrypt;
	bool                  user_length = false;
	struct  enncodingkey  default_keys;
	struct	sockaddr_in	  serv_addr;
	struct	hostent		 *host;
	// set default key if key file is provided
	if ( argc == 4 ) {

		FILE *fp;
		ssize_t reader;
		char *line = NULL;
		size_t len = 0;
		
		fp = fopen( argv[3], "r" );
		getline( &line, &len, fp );
		strcpy( default_keys.salt, &line[5] );
		strcpy( default_keys.key, &line[4] );
		strcpy( default_keys.iv, &line[4] );
	}
	// get link
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
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr( argv[1] );
	serv_addr.sin_port        = htons( atoi(argv[2]) );

	if ( connect( sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr) ) ){

		perror( "Client: cannot connect to server" );
		exit(1);
	}
	// wait for the greeting message
	while ( strlen( message ) == 0  )
		recv( sock, message, sizeof( message ), 0 );

	printf( "%s", message );

	while (1) {
		// User interface for user to work with
		memset( message, 0, sizeof( message ) );
		memset( buff_send, 0, sizeof( buff_send ) );
		
		printf( "= = = = = = = = = =\n" );
		
		printf( "What is the command(1-5):\n" );
		printf( "1. Check whiteboard(?#\\n)\n" );
		printf( "2. Update whiteboard with plain text(@#p#\\nMessage\\n)\n" );
		printf( "3. Update whiteboard with encrypt text(@#c#\\nMessage\\n)\n" );
		printf( "4. Type in by hand\n" );
		printf( "5. Exit\n" );
		
		scanf( "%d", &selector );
		
		while ( selector > 5 || selector < 1 ) {
		
			printf( "Illegal choice! Usage (1-5)\n" );
			scanf( "%d", &selector );
		}

		printf( "= = = = = = = = = =\n" );

		if ( selector == 1 ) {
			// look for entry
			memset( buffer, 0, sizeof( buffer ) );
			printf( "Which entry you want to check?\n" );
			scanf( "%s", buffer );
			sprintf( buff_send, "?%s\n", buffer );

		} else if ( selector == 2 ) {
			// change entry with plain text
			char plain[BUFFER], length_ptr[BUFFER], input_line[BUFFER];
			bool finish = false;
			memset( buffer, 0, sizeof( buffer ) );
			memset( plain, 0, sizeof( plain ) );
			memset( length_ptr, 0, sizeof( length_ptr ) );
			printf( "Which entry you want to update?\n" );
			scanf( "%s", buffer );

			printf( "\nHow do you like to input the content?\n" );
			printf( "1. By type in terminal\n" );
			printf( "2. By file\n" );
			scanf( "%d", &selector );

			while ( selector > 2 || selector < 1 ) {
		
				printf( "Illegal choice! Usage (1,2)\n" );
				scanf( "%d", &selector );
			}

			printf( "\nDo you want to specific the length of input content?\n");
			printf( "Default is the whole input content.\n" );
			printf( "Will be MIN( len(content), user setting )\n" );
			printf( "Type in -1 for default. Number for length.\n" );

			scanf( "%s", length_ptr );

			if ( length_ptr[0] != '-' ) {
				input_length = atoi( length_ptr );
				user_length = true;
			} else
				user_length = false;

			if ( selector == 1 )
				if ( user_length ) {
					printf( "\nWhat content you want to update?\n" );
					while ( input_length > 0 ) {
						memset( input_line, 0, sizeof( input_line ) );
						scanf( "%s", input_line );
						if ( input_length > 0 ) {
							strncat( plain, input_line, 
							        MIN( strlen( input_line ), input_length ) );
							current = current + strlen( input_line );
							input_length = input_length - strlen( input_line );
						}
						if ( input_length > 0 ) {
							strcat( plain, "\n" );
							input_length--;
						}
					}
					plain[current] = '\0';
				} else {
					while ( !finish ) {
						memset( input_line, 0, sizeof( input_line ) );
						printf( "\nWhat content you want to update?\n" );
						scanf( "%s", input_line );
						strcat( plain, input_line );
						printf( "\nDo you want to continue the input?\n" );
						printf( "1. Yes - 2. No\n" );
						scanf( "%d", &selector );
						while ( selector > 2 || selector < 1 ) {
		
							printf( "Illegal choice! Usage (1,2)\n" );
							scanf( "%d", &selector );
						}
						if ( selector == 1 )
							strcat( plain, "\n" );
						else if ( selector == 2 )
							finish = true;
					}
					strcat( plain, "\0" );
				}
			else if ( selector == 2 ) {

				memset( input_line, 0, sizeof( input_line ) );
				printf( "\nWhat is the file name?\n" );
				scanf( "%s", input_line );

				FILE   *fp;
				char   *line = NULL,
					   *temp_content;
				size_t  len = 0;

				fp = fopen( input_line, "r" );

				if ( fp == NULL ) {
					printf( "Cannot find the file\n" );
					continue;
				}
				memset( input_line, 0, sizeof( input_line ) );
				while ( getline( &line, &len, fp ) != -1 )
					strcat( input_line, line );

				if ( user_length )
					strncpy( plain, input_line,
					         MIN( input_length, strlen( input_line ) ) );
				else {
					strcpy( plain, input_line );
					plain[strlen(plain) - 1] = '\0';
				}
			}
			sprintf( buff_send, "@%sp%zu\n%s\n", buffer, strlen(plain), plain );
			
		} else if ( selector == 3 ) {
			// change entry with encrypt content if key file provided
			char plain[BUFFER], length_ptr[BUFFER],
			     input_line[BUFFER], encode[BUFFER];
			bool finish = false;
			memset( buffer, 0, sizeof( buffer ) );
			memset( plain, 0, sizeof( plain ) );
			memset( length_ptr, 0, sizeof( length_ptr ) );
			printf( "Which entry you want to update?\n" );
			scanf( "%s", buffer );

			printf( "\nHow do you like to input the content?\n" );
			printf( "1. By type in terminal\n" );
			printf( "2. By file\n" );
			scanf( "%d", &selector );

			while ( selector > 2 || selector < 1 ) {
		
				printf( "Illegal choice! Usage (1,2)\n" );
				scanf( "%d", &selector );
			}

			printf( "\nDo you want to specific the length of input content?\n");
			printf( "Default is the whole input content.\n" );
			printf( "Will be MIN( len(content), user setting )\n" );
			printf( "Type in -1 for default. Number for length.\n" );

			scanf( "%s", length_ptr );

			if ( length_ptr[0] != '-' ) {
				input_length = atoi( length_ptr );
				user_length = true;
			} else
				user_length = false;

			if ( selector == 1 )
				if ( user_length ) {
					printf( "\nWhat content you want to update?\n" );
					while ( input_length > 0 ) {
						memset( input_line, 0, sizeof( input_line ) );
						scanf( "%s", input_line );
						if ( input_length > 0 ) {
							strncat( plain, input_line, 
							        MIN( strlen( input_line ), input_length ) );
							current = current + strlen( input_line );
							input_length = input_length - strlen( input_line );
						}
						if ( input_length > 0 ) {
							strcat( plain, "\n" );
							input_length--;
						}
					}
					plain[current] = '\0';
				} else {
					while ( !finish ) {
						memset( input_line, 0, sizeof( input_line ) );
						printf( "\nWhat content you want to update?\n" );
						scanf( "%s", input_line );
						strcat( plain, input_line );
						printf( "\nDo you want to continue the input?\n" );
						printf( "1. Yes - 2. No\n" );
						scanf( "%d", &selector );
						while ( selector > 2 || selector < 1 ) {
		
							printf( "Illegal choice! Usage (1,2)\n" );
							scanf( "%d", &selector );
						}
						if ( selector == 1 )
							strcat( plain, "\n" );
						else if ( selector == 2 )
							finish = true;
					}
					strcat( plain, "\0" );
				}
			else if ( selector == 2 ) {

				memset( input_line, 0, sizeof( input_line ) );
				printf( "\nWhat is the file name?\n" );
				scanf( "%s", input_line );

				FILE   *fp;
				char   *line = NULL,
					   *temp_content;
				size_t  len = 0;

				fp = fopen( input_line, "r" );

				if ( fp == NULL ) {
					printf( "Cannot find the file\n" );
					continue;
				}
				memset( input_line, 0, sizeof( input_line ) );
				while ( getline( &line, &len, fp ) != -1 )
					strcat( input_line, line );

				if ( user_length )
					strncpy( plain, input_line,
					         MIN( input_length, strlen( input_line ) ) );
				else {
					strcpy( plain, input_line );
					plain[strlen(plain) - 1] = '\0';
				}
			}
			//encoding( plain, encode );
			sprintf( buff_send, "@%sc%zu\n%s\n", buffer, strlen( encode ), encode );
		
		} else if ( selector == 4 ) {
		
			printf( "Please type in your own command:\n" );
			scanf( "%s", buff_send );
			

		} else {
			// end the connection
			printf( "Shutting down.\n" );
			close( sock );
			printf( "Shutted down.\n" );
			exit(1);
		}
		// let user know what command they did
		printf( "= = = = = = = = = =\n" );
		printf( "Your command is generated:\n-----\n%s-----\n", buff_send );

		send( sock, buff_send, strlen( buff_send ), 0 );
		
		printf( "Respond from the server:\n-----\n" );

		// get response from the server
		read( sock, message, BUFFER );


		printf( "%s", message );
		printf( "= = = = = = = = = =\n" );		

		//close( sock );
	}

	return 0;
}
