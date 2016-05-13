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
	int ch;
	char* buffer;
	int size;
	FILE *archivoAnsisop;
	archivoAnsisop =
			fopen(
					"/home/utnso/Escritorio/TPOperativos/tp-2016-1c-Explosive-code/TestParser/facil.ansisop",
					"r");

	if (archivoAnsisop == NULL) {
		perror("Error al tratar de leer archivo");
		exit(EXIT_FAILURE);
	}

	fseek (archivoAnsisop , 0 , SEEK_END);
	size = ftell (archivoAnsisop);
	rewind (archivoAnsisop);
	//es mejor poner size en vez del puntero q estaba antes, le puse un mas 5 por las dudas
	buffer = malloc(size+5);

	struct cliente clienteConsola;
	clienteConsola = crearCliente(8080, "127.0.0.1");
	//conectarConServidor(clienteConsola);
	/*char* msje="Hola";
	 char* handshake= hacerHandShake_server(clienteConsola.socketServer,msje);
	 printf("%p/n",handshake);*/

	//fgetc devuelve un int, quizas por eso tira error al acceder a memoria
	ch = fgetc(archivoAnsisop);
	while ((ch) != EOF) {
		//Le paso la direccion de ch xq strcat recibe dos punteros char
		strcat(buffer, (char*) &ch);
		ch = fgetc(archivoAnsisop);
	}

	//al usar strlen, no cuenta los espacios asi q hay q ve otra forma, quizas usando size de antes, pero nose
	send((clienteConsola.socketCliente), buffer, strlen(buffer), 0);
	close(clienteConsola.socketCliente);
	free(buffer);

	return 0;
}




