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
extern umcNucleo umcConfg;


void solicitar_Bytes(int socket){
	int* pagina = (int*) recibirStream(socket, sizeof(int));
	int* offset = (int*) recibirStream(socket, sizeof(int));
	int* tamanio = (int*) recibirStream(socket, sizeof(int));
	int* idProceso = (int*) recibirStream(socket,sizeof(int));
	cambiarProceso(*idProceso);
	log_info(umcConfg.loguer, "Obtener bytes iniciado");
	void* obtenido = obtenerBytesMemoria(*pagina,*offset,*tamanio);
	log_info(umcConfg.loguer, "Obtener bytes terminado");
	//le envio lo obtenido a cpu
	send(socket,obtenido,*tamanio,0);
	free(pagina);
	free(offset);
	free(tamanio);
	free(obtenido);
	return;
}

void almacenar_Byte(int socket){
	int* pagina = (int*) recibirStream(socket, sizeof(int));
	int* offset = (int*) recibirStream(socket, sizeof(int));
	int* tamanio =(int*) recibirStream(socket, sizeof(int));
	void* aAlmacenar = recibirStream(socket, *tamanio);
	int* idProceso = (int*) recibirStream(socket,sizeof(int));
	cambiarProceso(*idProceso);
	log_info(umcConfg.loguer, "Almacenar byte comenzado");
	almacenarBytes(*pagina,*offset,*tamanio,aAlmacenar);
	log_info(umcConfg.loguer, "Almacenar byte terminado");
	free(pagina);
	free(offset);
	free(tamanio);
	free(aAlmacenar);
	return;
}

void* conexionCpu(int socketEscuchaCpu){
	int seguir = 1;
	while(seguir){
		int* header = recibirStream(socketEscuchaCpu,sizeof(int));
		switch (*header) {
			case 52://SOLICITAR
				solicitar_Bytes(socketEscuchaCpu);
				break;
			case 53://ALMACENAR
				almacenar_Byte(socketEscuchaCpu);
				break;
			case 666:
				enviarStream(socketEscuchaCpu,666,sizeof(int),&umcConfg.configuracionUMC.MARCO_SIZE);
				break;
			case -1:
				printf("Perdida la conexion con cpu\n");
				close(socketEscuchaCpu);
				seguir = 0;
				break;
			default:
				break;
		}
		free(header);
	}
}



