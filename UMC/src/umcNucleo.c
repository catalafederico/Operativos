/*
 * umcNucleo.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */
#include "estructurasUMC.h"
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
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "archivoConf.h"
#include "umcMemoria.h"
#include "umcCliente.h"

t_list* programasEjecucion;
tempStruct* todoUMC;


void inicializar_programa(int socket) {
	id_programa nuevoPrograma;
	int* id = (int*) recibirStream(socket, sizeof(int));
	int* cantPag = (int*) recibirStream(socket, sizeof(int));
}

void finalizar_programa(int socket){
	int* id = (int*) recibirStream(socket, sizeof(int));
	int i = 0;
	while(i<list_size(programasEjecucion)){
		proceso* tempPro = list_get(programasEjecucion,i);
		if(*(tempPro->id_programa)==*id){
			desalojarPrograma(tempPro);
			notificarASwapFinPrograma(*id,todoUMC->umcConfig->socketSwap);
			return;
		}
	}
	return;
}


void* conexionNucleo(tempStruct* socketNucleo){
	todoUMC = socketNucleo;
	while(1){
		int* header =(int *) recibirStream(socketNucleo->socket,sizeof(int));
		switch (*header) {
			case FINALIZACIONPROGRAMA:
				finalizar_programa(socketNucleo->umcConfig->socketSwap);
				break;
			case NUEVOPROGRAMA:
				inicializar_programa(socketNucleo->umcConfig->socketSwap);
				break;
			default:
				break;
		}
	}
}



