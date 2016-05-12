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
#include <sockets/socketServer.h>
#include <commons/txt.h>
int main(void) {
	char* buffer;
	int numDeElemLeidos;
	struct cliente clienteConsola;
	clienteConsola=crearCliente(8080,"127.0.0.1");
/*	conectarConServidor(clienteConsola);
	char* msje="Hola";
	char* handshake= hacerHandShake_server(clienteConsola.socketServer,msje);
	printf("%p/n",handshake);*/
	FILE* archivoAnsisop = txt_open_for_append("/home/utnso/workspace/tp-2016-1c-Explosive-code/TestParser/facil.ansisop");


	if(archivoAnsisop ==NULL){
		perror("Error al leer archivo");
	}
	else{
	while (!feof(archivoAnsisop)) {
		//char mensaje[SOMAXCONN];
		//scanf("%s", mensaje);
		buffer = (char*) malloc (sizeof(char)*ftell(archivoAnsisop));
		numDeElemLeidos=fread(buffer,1,ftell(archivoAnsisop),archivoAnsisop);
		send((clienteConsola.socketCliente), buffer, ftell(archivoAnsisop) , 0);
	}}

	fclose(clienteConsola.socketCliente);
	txt_close_file(archivoAnsisop);
	free (buffer);



	return 0;
}




