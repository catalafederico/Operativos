/*
 * umcNucleo.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */
#include "estructurasUMC.h"
#include <stdio.h>
#include <sockets/header.h>
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
#include <sockets/basicFunciones.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "archivoConf.h"
#include "umcMemoria.h"
#include "umcCliente.h"

t_list* programasEjecucion;
tempStruct* todoUMC;

void* solicitar_Bytes(int socket){
	int* pagina = (int*) recibirStream(todoUMC->socket, sizeof(int));
	int* offset = (int*) recibirStream(todoUMC->socket, sizeof(int));
	int* tamanio = (int*) recibirStream(todoUMC->socket, sizeof(int));
	log_info(todoUMC->umcConfig->loguer, "Obtener bytes iniciado");
	void* obtenido = obtenerBytesMemoria(*pagina,*offset,*tamanio);
	log_info(todoUMC->umcConfig->loguer, "Obtener bytes terminado");
	free(pagina);
	free(offset);
	free(tamanio);
	return obtenido;
}

void almacenar_Byte(int socket){
	int* pagina = (int*) recibirStream(socket, sizeof(int));
	int* offset = (int*) recibirStream(socket, sizeof(int));
	int* tamanio =(int*) recibirStream(socket, sizeof(int));
	void* aAlmacenar = recibirStream(socket, *tamanio);
	log_info(todoUMC->umcConfig->loguer, "Almacenar byte comenzado");
	almacenarBytes(*pagina,*offset,*tamanio,aAlmacenar);
	log_info(todoUMC->umcConfig->loguer, "Almacenar byte terminado");
	free(pagina);
	free(offset);
	free(tamanio);
	free(aAlmacenar);
	return;
}

void* conexionCpu(tempStruct* socketNucleo){
	todoUMC = socketNucleo;
	int seguir = 1;
	while(seguir){
		int* header = leerHeader(socketNucleo->socket);
		switch (*header) {
			case 52://SOLICITAR
				solicitar_Bytes(socketNucleo->umcConfig->socketSwap);
				break;
			case 53://ALMACENAR
				almacenar_Byte(socketNucleo->umcConfig->socketSwap);
				break;
			case -1:
				printf("Perdida la conexion con cpu\n");
				close(socketNucleo->socket);
				seguir = 0;
				break;
			default:
				break;
		}
		free(header);
	}
}



