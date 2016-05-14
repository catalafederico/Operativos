/*
 * umcNucleo.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sockets/header.h>
#include <sockets/basicFunciones.h>
#include "archivoConf.h"

typedef struct {
	int identificador;
	int paginas_requeridas;
} __attribute__((packed))
id_programa;

typedef struct{
	t_reg_config configuracionUMC;
	void* memoriaPrincipal;
	int socketSwap;
}umcNucleo;


typedef struct{
	umcNucleo* umcConfig;
	int socket;
}tempStruct;

id_programa* inicializar_programa(int socket){
	id_programa* nuevoPrograma = malloc(sizeof(id_programa));
	int* id = (int*)recibirStream(socket,sizeof(int));
	int* cantPag = (int*)recibirStream(socket,sizeof(int));
	nuevoPrograma->identificador = *id;
	nuevoPrograma->paginas_requeridas=  *cantPag;
	return nuevoPrograma;
}

void* conexionNucleo(tempStruct socketNucleo){
	while(1){
		int* header =(int *) recibirStream(socketNucleo.socket,sizeof(int));
		switch (*header) {
			case FINALIZACIONPROGRAMA:
				break;
			case NUEVOPROGRAMA:
				inicializar_programa(socketNucleo.umcConfig->socketSwap);
				break;
			default:
				break;
		}
	}
}



