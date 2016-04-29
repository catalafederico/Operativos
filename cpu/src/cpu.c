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

#define  SERVERUMC 9999 //puerto de la umc
#define  SERVERNUCLEO 8080 // puerto del nucleo
int main(void) {
	struct cliente clienteCpuNucleo;
	clienteCpuNucleo = crearCliente(SERVERNUCLEO, "127.0.0.1");
	conectarConServidor(clienteCpuNucleo);
	char *mensajeRecibidoDelNucleo = esperarRespuestaServidor(
			clienteCpuNucleo.socketCliente);
	printf("Mensaje recibido de Nucleo %s \n", mensajeRecibidoDelNucleo);
	close(clienteCpuNucleo);
	struct cliente clienteCpuUmc;
	clienteCpuUmc = crearCliente(SERVERUMC, "127.0.0.1");
	conectarConServidor(clienteCpuUmc);
	enviarMensaje(clienteCpuUmc.socketCliente, mensajeRecibidoDelNucleo);
	close(clienteCpuUmc);
	return 0;

}
