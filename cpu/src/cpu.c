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
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <commons/collections/list.h>
#include <sockets/header.h>
#include "funcionesparsernuevas.h"
#include <sockets/socketCliente.h>
#include <parser/parser.h>


AnSISOP_funciones functions = {
	.AnSISOP_definirVariable	= vardef,
	.AnSISOP_obtenerPosicionVariable= getvarpos,
	.AnSISOP_dereferenciar	= derf,
	.AnSISOP_asignar	= asignar,
	.AnSISOP_imprimir	= imprimir,
	.AnSISOP_imprimirTexto= imptxt,
	.AnSISOP_irAlLabel = goint,
	.AnSISOP_asignarValorCompartida = setglobalvar,
	.AnSISOP_obtenerValorCompartida = getglobalvar,
	.AnSISOP_entradaSalida = ionotif,
	.AnSISOP_retornar = retornar
};
AnSISOP_kernel kernel_functions = { };

struct cliente clienteCpuUmc;
struct cliente clienteCpuNucleo;

#define  SERVERUMC 9999 //puerto de la umc
#define  SERVERNUCLEO 5001 // puerto del nucleo

void conectarseConUMC(struct cliente clienteCpuUmc);
void procesarInstruccion(char* instruccion);

int main(void) {

	//Empieza conexion UMC
	clienteCpuUmc = crearCliente(SERVERUMC, "127.0.0.1");
	conectarseConUMC(clienteCpuUmc);
	//Termina conexion UMC

	//Empieza conexion Nucleo
	clienteCpuNucleo = crearCliente(SERVERNUCLEO, "127.0.0.1");
	conectarseConNucleo(clienteCpuNucleo);
	//Termina conexion Nucleo

	//Le paso el socket de la umc, para no pasarlo x cada pedido
	inicialzarParser(clienteCpuUmc.socketCliente);

	//Empieza la escucha de nucleo
	int seguir = 1;
	while(seguir){
		int* header = leerHeader(clienteCpuNucleo.socketCliente,"127.0.0.1");
		switch (*header) {
			case 163://Recibir PCB

				break;
			default:
				break;
		}
		free(header);
	}
	return 0;
}

void conectarseConUMC(struct cliente clienteCpuUmc){
	conectarConServidor(clienteCpuUmc);
	int cpuid = CPU;
	//Empieza handshake
	if(send(clienteCpuUmc.socketCliente,&cpuid,sizeof(int),0)==-1){
		printf("no se ha podido conectar con UMC.\n");
		perror("no anda:\0");
	}
	int* recibido = recibirStream(clienteCpuUmc.socketCliente,sizeof(int));
	if(*recibido==OK){
		printf("Se ha conectado correctamente con UMC.\n");
	}
	//Termina Handshake
}

void conectarseConNucleo(struct cliente clienteCpuNucleo){
	conectarConServidor(clienteCpuNucleo);
	int cpuid = CPU;
	//Empieza handshake
	if(send(clienteCpuNucleo.socketCliente,&cpuid,sizeof(int),0)==-1){
		printf("no se ha podido conectar con UMC.\n");
		perror("no anda:\0");
	}
	int* recibido = recibirStream(clienteCpuNucleo.socketCliente,sizeof(int));
	if(*recibido==OK){
		printf("Se ha conectado correctamente con UMC.\n");
	}
	//Termina Handshake
}

void procesarInstruccion(char* instruccion){
	analizadorLinea(instruccion,&functions,&kernel_functions);
}

