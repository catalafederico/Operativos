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
pthread_mutex_t memoriaLibre;

void* recibirCpu(int socketCPU, int tamanio, int pid_local);
void enviarCPU(int socketCPU, int tamanio, void* mensaje,int pid_local);
void enviarCPUConHeader(int socketCPU,int header, int tamanio, void* mensaje,int pid_local);


void terminaDeFormaAbortiva(int pid_local){
	printf("Perdida la conexion con cpu\n");
	int a = 100;
	pthread_exit(&a);
}

void solicitar_Bytes(int socket){
	int* pagina = (int*) recibirCpu(socket, sizeof(int), -1);
	int* offset = (int*) recibirCpu(socket, sizeof(int), -1);
	int* tamanio = (int*) recibirCpu(socket, sizeof(int), -1);
	int* idProceso = (int*) recibirCpu(socket, sizeof(int), -1);
	usleep(umcConfg.configuracionUMC.RETARDO*1000);
	cambiarProceso(*idProceso);
	log_info(umcConfg.loguer, "Obtener bytes iniciado");
	void* obtenido = obtenerBytesMemoria(*pagina,*offset,*tamanio);
	if(obtenido==NULL){
		int segFault = 777;
		enviarCPU(socket,sizeof(int),&segFault,-1);
		//send(socket,&segFault,sizeof(int),0);
		free(pagina);
		free(offset);
		free(tamanio);
		return;
	}else{
		int ok = OK;
		//send(socket,&ok,sizeof(int),0);
		enviarCPU(socket,sizeof(int),&ok,-1);
		log_info(umcConfg.loguer, "Obtener bytes terminado");
		//le envio lo obtenido a cpu
		enviarCPU(socket,*tamanio,obtenido,-1);
		//send(socket,obtenido,*tamanio,0);
		free(pagina);
		free(offset);
		free(tamanio);
		free(obtenido);
		return;
	}
}

void almacenar_Byte(int socket){
	int* pagina = (int*) recibirCpu(socket, sizeof(int), -1);
	int* offset = (int*) recibirCpu(socket, sizeof(int), -1);
	int* tamanio =(int*) recibirCpu(socket, sizeof(int), -1);
	void* aAlmacenar = recibirCpu(socket, *tamanio, -1);
	int* idProceso = (int*) recibirCpu(socket, sizeof(int), -1);
	usleep(umcConfg.configuracionUMC.RETARDO*1000);
	cambiarProceso(*idProceso);
	log_info(umcConfg.loguer, "Almacenar byte comenzado");
	int conf = almacenarBytes(*pagina,*offset,*tamanio,aAlmacenar);
	if(conf==0){
		int segFault = 777;
		enviarCPU(socket,sizeof(int),&segFault,-1);
		//send(socket,&segFault,sizeof(int),0);
	}else{
		int ok = OK;
		enviarCPU(socket,sizeof(int),&ok,-1);
		//send(socket,&ok,sizeof(int),0);
		log_info(umcConfg.loguer, "Obtener bytes terminado");
		//le envio lo obtenido a cpu
	}
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
		if(header==NULL){
			header = malloc(sizeof(int));
			*header = 4;
		}
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
			case 4:
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

void* recibirCpu(int socketCPU, int tamanio, int pid_local){
	void* obtenido = recibirStream(socketCPU, tamanio);
	if(obtenido==NULL){
		terminaDeFormaAbortiva(pid_local);
	}
	else{
		return obtenido;
	}
}

void enviarCPU(int socketCPU, int tamanio, void* mensaje,int pid_local){
	int resultado = send(socketCPU,mensaje,tamanio,0);
	if(resultado == -1){
		if(errno == EPIPE){
			terminaDeFormaAbortiva(pid_local);
		}
	}
}

void enviarCPUConHeader(int socketCPU,int header, int tamanio, void* mensaje,int pid_local){
	int resultado = enviarStream(socketCPU,header,tamanio,mensaje);
	if(resultado == -987){
			terminaDeFormaAbortiva(pid_local);
	}
}

