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

int main(int argc, char **argv) {
/*	  int index;
	  for(index = 0; index < argc; index++) {
	    printf("  Parametro %d: %s\n", index, argv[index]);
	  }*/

	int ch;
	char* buffer;
	int size;
	char* buff;
//	char* rutaArchivo;
//	rutaArchivo=argv[1];
	FILE *archivoAnsisop;
	archivoAnsisop =fopen("/home/utnso/workspace/tp-2016-1c-Explosive-code/consola/facil.ansisop","r");
//hay que abrirlo con el gcc ejecutarlo y pasarle los parametros el primer parametro(argv[0]) es el programa y el otro la rutadearchivo
//	archivoAnsisop =fopen(rutaArchivo,"r");
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

	int consola = CONSOLA;
	enviarStream(clienteConsola.socketCliente,consola,strlen(buffer),buffer);
	free(buffer);
	int seguir = 1;
	int* tamanio;
	char* mensaje;
	while(seguir){
		int* header = leerHeader(clienteConsola.socketCliente);
				switch (*header) {
					case 100://imprimir
						tamanio = recibirStream(clienteConsola.socketCliente,sizeof(int));
						mensaje = recibirStream(clienteConsola.socketCliente,*tamanio);
						free(tamanio);
						free(mensaje);
						printf("%s",mensaje);
						break;
					case 101://imprimirTexto
						tamanio = recibirStream(clienteConsola.socketCliente,sizeof(int));
						mensaje = recibirStream(clienteConsola.socketCliente,*tamanio);
						free(tamanio);
						free(mensaje);
						printf("%s",mensaje);
						break;
					case -1://pierde conexion
						printf("fin");
						printf("desconectado");
						free(header);
						seguir =0;
						break;
				}


	}
	fclose(archivoAnsisop);
	close(clienteConsola.socketCliente);

	return 0;
}




