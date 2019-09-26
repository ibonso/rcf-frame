#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "rcf-proxy.h"

void * __gxx_personality_v0;

int main( int argc, char * argv [] ) 
{
	char* ptrProxy;
		
    printf( "argc = %d\n", argc );
    for( int i = 0; i < argc; ++i ) {
        printf( "argv[ %d ] = %s\n", i, argv[ i ] );
    }
    
    struct stat st;
	int result = stat(argv[1], &st);
	if( result != 0 ) {
			printf( "File  %s doesn't exist.\n", argv[1]);
			exit(0);
	}
    
    printf("rcf-proxy-tes: se va a llamar a rcf_proxy");
    ptrProxy=rcf_proxy("/fcgi/login/","eyJuYW1lIjoibG9naW4iLCJsb2dpbiI6eyJpZCI6ImFkbSIsInBhc3MiOiJiMDljNjAwZmRkYzU3M2YxMTc0NDliMzcyM2YyM2Q2NCJ9fQ",argc, argv );
    
    if(ptrProxy!=0) {
		printf("rcf-proxy-tes: de vuelta\n");
		printf("Recibido: %s\n", ptrProxy);
		
		free(ptrProxy);
		printf("Hecho free!\n");
	}
	printf(" FIN\n");
	

}


		
