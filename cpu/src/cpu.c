/*
 ============================================================================
 Name        : cpu.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <commons/collections/list.h>
#include "socketCliente.h"

#define  SERVERCLIENTE1 9999
#define  SERVERCLIENTE2  5001
int main(void) {
	struct cliente clienteCPU;
	clienteCPU = crearCliente(SERVERCLIENTE1, "127.0.0.1");
	conectarConServidor(clienteCPU);
	char *mensaje =esperarRespuestaServidor(clienteCPU.socketCliente);

	struct cliente clienteCPU1;
	clienteCPU1 = crearCliente(SERVERCLIENTE2, "127.0.0.1");
	conectarConServidor(clienteCPU1);
	enviarMensaje(clienteCPU1.socketCliente, mensaje);

	return 0;

}
/*

 struct cliente clienteCPU;
 clienteCPU = crearCliente(SERVERCLIENTE2,"127.0.0.1");
 conectarConServidor(clienteCPU);
 enviarMensaje(clienteCPU.socketCliente,"hola");



 */
