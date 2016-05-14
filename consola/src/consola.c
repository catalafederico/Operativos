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
#include <sockets/header.h>
#include <commons/txt.h>
#include <string.h>

int main(void) {
	int ch;
	char* buffer;
	int size;
	FILE *archivoAnsisop;
	archivoAnsisop =fopen("/home/utnso/workspace/tp-2016-1c-Explosive-code/consola/facil.ansisop","r");

	if (archivoAnsisop == NULL) {
		perror("Error al tratar de leer archivo");
		exit(EXIT_FAILURE);
	}

	fseek (archivoAnsisop , 0 , SEEK_END);
	size = ftell (archivoAnsisop);
	rewind (archivoAnsisop);

	buffer = malloc(size+5);

	struct cliente clienteConsola;
	clienteConsola = crearCliente(8080, "127.0.0.1");
	conectarConServidor(clienteConsola);


	ch = fgetc(archivoAnsisop);
	while ((ch) != EOF) {
		//Le paso la direccion de ch xq strcat recibe dos punteros char
		strcat(buffer, (char*) &ch);
		ch = fgetc(archivoAnsisop);
	}

//send((clienteConsola.socketCliente), buffer, strlen(buffer), 0);
	enviarStream(clienteConsola.socketCliente,CONSOLA,strlen(buffer),buffer);
	free(buffer);
	close(clienteConsola.socketCliente);




	return 0;
}




