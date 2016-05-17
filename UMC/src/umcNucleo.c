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


void inicializar_programa(int socket) {
	int* id = (int*) recibirStream(todoUMC->socket, sizeof(int));
	int* cantPag = (int*) recibirStream(todoUMC->socket, sizeof(int));
	log_info(todoUMC->umcConfig->loguer, "Alocar Programa empesado");
	alocarPrograma(*cantPag,*id);
	log_info(todoUMC->umcConfig->loguer, "Programa Alocado correctamente");
	free(id);
	free(cantPag);
}

void finalizar_programa(int socket){
	int* id = (int*) recibirStream(todoUMC->socket, sizeof(int));
	log_info(todoUMC->umcConfig->loguer, "Desalojar programa empesado");
	desalojarPrograma(*id);
	log_info(todoUMC->umcConfig->loguer, "Proceso desalojado");
	free(id);
	return;
}

void* solicitar_Bytes_NL(int socket){
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

void change_Proceso(int socket){
	int* idProceso = (int*) recibirStream(todoUMC->socket, sizeof(int));
	cambiarProceso(*idProceso);
	log_info(todoUMC->umcConfig->loguer, "Tabla cambiada");
	free(idProceso);
}

void almacenar_Byte_NL(int socket){
	int* pagina = (int*) recibirStream(todoUMC->socket, sizeof(int));
	int* offset = (int*) recibirStream(todoUMC->socket, sizeof(int));
	int* tamanio =(int*) recibirStream(todoUMC->socket, sizeof(int));
	void* aAlmacenar = recibirStream(todoUMC->socket, *tamanio);
	log_info(todoUMC->umcConfig->loguer, "Almacenar byte comenzado");
	almacenarBytes(*pagina,*offset,*tamanio,aAlmacenar);
	log_info(todoUMC->umcConfig->loguer, "Almacenar byte terminado");
	free(pagina);
	free(offset);
	free(tamanio);
	free(aAlmacenar);
	return;
}







void* conexionNucleo(tempStruct* socketNucleo){
	todoUMC = socketNucleo;
	while(1){
		int* header =leerHeader(socketNucleo->socket);
		switch (*header) {
			case FINALIZACIONPROGRAMA:
				finalizar_programa(socketNucleo->umcConfig->socketSwap);
				break;
			case NUEVOPROGRAMA:
				inicializar_programa(socketNucleo->umcConfig->socketSwap);
				break;
			case CAMBIOPROCESO:
				change_Proceso(socketNucleo->umcConfig->socketSwap);
				break;
			case SOLICITAR:
				solicitar_Bytes_NL(socketNucleo->umcConfig->socketSwap);
				break;
			case ALMACENAR:
				almacenar_Byte_NL(socketNucleo->umcConfig->socketSwap);
				break;
			default:
				break;
		}

		free(header);
	}
}



