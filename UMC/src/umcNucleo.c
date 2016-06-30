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
int socketNucleo;
extern t_log* logConexiones;
extern pthread_mutex_t semaforoMemoria;
extern int alocandoPrograma;

void inicializar_programa() {
	t_dictionary* codigo_programa = dictionary_create();
	int* id = (int*) recibirStream(socketNucleo, sizeof(int));
	int* cantPag = (int*) recibirStream(socketNucleo, sizeof(int));
	log_info(umcConfg.loguer, "Alocar Programa empezado");
	int seguir = 1;
	while(seguir){
		int* pagina = recibirStream(socketNucleo,sizeof(int));//si es menos uno no hay mas paginas
		if(*pagina==-1){
			seguir = 0;
		}
		else{
			void* buffer = recibirStream(socketNucleo,umcConfg.configuracionUMC.MARCO_SIZE);
			dictionary_put(codigo_programa,pagina,buffer);
		}
	}
	usleep(umcConfg.configuracionUMC.RETARDO*1000);
	if(alocarPrograma(*cantPag,*id,codigo_programa)==-1){
		int error = ERROR;
		send(socketNucleo,&error, sizeof(int),0);
		log_info(umcConfg.loguer, "Programa no ha sido alocado correctamente");
	}else{
		int ok = OK;
		send(socketNucleo,&ok, sizeof(int),0);
		log_info(umcConfg.loguer, "Programa alocado correctamente");
	}
	free(id);
	free(cantPag);
}

void finalizar_programa(){
	int* id = (int*) recibirStream(socketNucleo, sizeof(int));
	log_info(umcConfg.loguer, "Desalojar programa empesado");
	usleep(umcConfg.configuracionUMC.RETARDO*1000);
	desalojarPrograma(*id);
	log_info(umcConfg.loguer, "Proceso desalojado");
	free(id);
	return;
}

void* solicitar_Bytes_NL(){
	int* pagina = (int*) recibirStream(socketNucleo, sizeof(int));
	int* offset = (int*) recibirStream(socketNucleo, sizeof(int));
	int* tamanio = (int*) recibirStream(socketNucleo, sizeof(int));
	log_info(umcConfg.loguer, "Obtener bytes iniciado.\n");
	usleep(umcConfg.configuracionUMC.RETARDO*1000);
	void* obtenido = obtenerBytesMemoria(*pagina,*offset,*tamanio);
	log_info(umcConfg.loguer, "Obtener bytes terminado.\n");
	free(pagina);
	free(offset);
	free(tamanio);
	return obtenido;
}

void change_Proceso(){
	int* idProceso = (int*) recibirStream(socketNucleo, sizeof(int));
	cambiarProceso(*idProceso);
	log_info(umcConfg.loguer, "Tabla cambiada.\n");
	free(idProceso);
}

void almacenar_Byte_NL(){
	int* pagina = (int*) recibirStream(socketNucleo, sizeof(int));
	int* offset = (int*) recibirStream(socketNucleo, sizeof(int));
	int* tamanio =(int*) recibirStream(socketNucleo, sizeof(int));
	void* aAlmacenar = recibirStream(socketNucleo, *tamanio);
	log_info(umcConfg.loguer, "Almacenar byte comenzado.\n");
	usleep(umcConfg.configuracionUMC.RETARDO*1000);
	almacenarBytes(*pagina,*offset,*tamanio,aAlmacenar);
	log_info(umcConfg.loguer, "Almacenar byte terminado.\n");
	free(pagina);
	free(offset);
	free(tamanio);
	free(aAlmacenar);
	return;
}

void* conexionNucleo(int  socketEscuchaNucleo){
	socketNucleo = socketEscuchaNucleo;
	int seguir = 1;
	int tamanioPag = umcConfg.configuracionUMC.MARCO_SIZE;
	while(seguir){
		int* header = recibirStream(socketEscuchaNucleo,sizeof(int));
		/*int* id = leerHeader(socketEscuchaNucleo);
		cambiarProceso(*id);*/
		if(header==NULL){
			header = malloc(sizeof(int));
			*header = -1;
		}
		log_trace(logConexiones,"Header recibido en Nucleo: %d\n", *header);
		switch (*header) {
			case FINALIZACIONPROGRAMA:
				finalizar_programa();
				break;
			case NUEVOPROGRAMA:
				inicializar_programa();
				break;
			case CAMBIOPROCESO:
				change_Proceso();
				break;
			case SOLICITAR:
				solicitar_Bytes_NL();
				break;
			case ALMACENAR:
				almacenar_Byte_NL();
				break;
			case 666:
				enviarStream(socketEscuchaNucleo,666,sizeof(int),&tamanioPag);
				break;
			case -1:
				printf("Se desconecto Nucleo, terminado thread\n");
				log_trace(logConexiones,"Nucleo desconectado.");
				seguir = 0;
				break;
			default:
				break;
		}
		free(header);
		//free(id);
	}
}



