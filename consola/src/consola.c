/*
 ============================================================================
 Name        : consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sockets/socketCliente.h>
#include <sockets/basicFunciones.h>
#include <commons/txt.h>
#include <string.h>

int main(void) {
	char* ch;
	char* buffer;
	int size;
	FILE *archivoAnsisop;
     archivoAnsisop = fopen("/home/utnso/workspace/tp-2016-1c-Explosive-code/TestParser/facil.ansisop","r");

	 if(  archivoAnsisop== NULL )
		   {
		      perror("Error al tratar de leer archivo");
		      exit(EXIT_FAILURE);
		   }


	 fseek (archivoAnsisop , 0 , SEEK_END);
	 size = ftell (archivoAnsisop);
	 rewind (archivoAnsisop);
	 buffer = (char*) malloc (sizeof(char)*size);

	 struct cliente clienteConsola;
	 clienteConsola=crearCliente(8080,"127.0.0.1");
     conectarConServidor(clienteConsola);
/*	char* msje="Hola";
	char* handshake= hacerHandShake_server(clienteConsola.socketServer,msje);
	printf("%p/n",handshake);*/



	  while( ( ch = fgetc(archivoAnsisop) ) != EOF ){
	  strcat(buffer,ch);

		  }

       send((clienteConsola.socketCliente), buffer, strlen(buffer), 0);
       close(clienteConsola.socketCliente);
       free(buffer);

	return 0;
}




