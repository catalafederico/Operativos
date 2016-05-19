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

void inicializar_programa() {
	int* id = (int*) recibirStream(socketNucleo, sizeof(int));
	int* cantPag = (int*) recibirStream(socketNucleo, sizeof(int));
	log_info(umcConfg.loguer, "Alocar Programa empesado");
	if(alocarPrograma(*cantPag,*id)==-1){
		int error = ERROR;
		send(socketNucleo,&error, sizeof(int),0);
	}else{
		int ok = OK;
		send(socketNucleo,&ok, sizeof(int),0);
	}
	log_info(umcConfg.loguer, "Programa Alocado correctamente");
	free(id);
	free(cantPag);
}

void finalizar_programa(){
	int* id = (int*) recibirStream(socketNucleo, sizeof(int));
	log_info(umcConfg.loguer, "Desalojar programa empesado");
	desalojarPrograma(*id);
	log_info(umcConfg.loguer, "Proceso desalojado");
	free(id);
	return;
}

void* solicitar_Bytes_NL(){
	int* pagina = (int*) recibirStream(socketNucleo, sizeof(int));
	int* offset = (int*) recibirStream(socketNucleo, sizeof(int));
	int* tamanio = (int*) recibirStream(socketNucleo, sizeof(int));
	log_info(umcConfg.loguer, "Obtener bytes iniciado");
	void* obtenido = obtenerBytesMemoria(*pagina,*offset,*tamanio);
	log_info(umcConfg.loguer, "Obtener bytes terminado");
	free(pagina);
	free(offset);
	free(tamanio);
	return obtenido;
}

void change_Proceso(){
	int* idProceso = (int*) recibirStream(socketNucleo, sizeof(int));
	cambiarProceso(*idProceso);
	log_info(umcConfg.loguer, "Tabla cambiada");
	free(idProceso);
}

void almacenar_Byte_NL(){
	int* pagina = (int*) recibirStream(socketNucleo, sizeof(int));
	int* offset = (int*) recibirStream(socketNucleo, sizeof(int));
	int* tamanio =(int*) recibirStream(socketNucleo, sizeof(int));
	void* aAlmacenar = recibirStream(socketNucleo, *tamanio);
	log_info(umcConfg.loguer, "Almacenar byte comenzado");
	almacenarBytes(*pagina,*offset,*tamanio,aAlmacenar);
	printf("Nucleo me pidio q almacene\n:");
	printf("pagina:%d\n",*pagina);
	printf("offset:%d\n",*offset);
	printf("tamanio:%d\n",*tamanio);
	printf("mensaje:%s\n",(char*)aAlmacenar);
	log_info(umcConfg.loguer, "Almacenar byte terminado");
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
		int* header =leerHeader(socketEscuchaNucleo);
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
				seguir = 0;
				break;
			default:
				break;
		}

		free(header);
	}
}



