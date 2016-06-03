/*
 * procesosUMC.c
 *
 *  Created on: 18/5/2016
 *      Author: Lucas Marino
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<pthread.h>

#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <sockets/socketCliente.h>
#include <sockets/socketServer.h>
#include <sockets/basicFunciones.h>
#include <semaphore.h>
#include "procesarPrograma.h"
#include "procesosUMC.h"
#include "estructurasNUCLEO.h"
#include <sockets/header.h>

// CONSTANTES -----
#define SOY_CPU 	"Te_conectaste_con_CPU____"
#define SOY_UMC 	"Te_conectaste_con_UMC____"
#define SOY_SWAP	"Te_conectaste_con_SWAP___"  // 25 de long sin \0
#define SOY_NUCLEO  "Te_conectaste_con_NUCLEO_"
#define SOY_CONSOLA	"Te_conectaste_con_CONSOLA"

// Variables compartidas ---------------------------------------------

extern t_list* programas_para_procesar;
extern t_list* cpus_dispo;
extern t_list* consolas_dispo;
extern t_list* proc_New;
extern t_log *logger;
extern int tamanioPaginaUMC;
extern t_reg_config reg_config;
extern struct cliente clienteNucleoUMC;


// semaforos Compartidos

extern pthread_mutex_t sem_l_New;
extern pthread_mutex_t sem_l_Ready;
extern pthread_mutex_t sem_l_Exec;
extern pthread_mutex_t sem_l_Block;
extern pthread_mutex_t sem_l_Exit;
extern pthread_mutex_t sem_l_Reject;
extern sem_t semaforoProgramasACargar;


void *procesos_UMC(){
//	clienteNucleoUMC = crearCliente(reg_config.puerto_umc,reg_config.ip_umc);
	clienteNucleoUMC = crearCliente(9999, "127.0.0.1");
	log_debug(logger, "Conexion con UMC");
	conectarseConUmc(clienteNucleoUMC);
	int seguir = 1;
	while(seguir){
		sem_wait(&semaforoProgramasACargar);//Espera a q haya programas a cargar
		programaNoCargado* progParaCargar = list_remove(programas_para_procesar,0);
		char* instrucciones = progParaCargar->instrucciones;
		indiceCodigo* icNuevo;
		t_list* instruccionesPaUMC = list_create();
		icNuevo = nuevoPrograma(instrucciones,instruccionesPaUMC);
		icNuevo->inst_tamanio = paginarIC(icNuevo->inst_tamanio);
		int posicion = (dictionary_size(icNuevo->inst_tamanio)-1);
		direccionMemoria* lastInt = dictionary_get(icNuevo->inst_tamanio,&posicion);
		int ultimaPaginaDeCodigo = lastInt->pagina;
		if(cargarEnUMC(icNuevo->inst_tamanio,instruccionesPaUMC,ultimaPaginaDeCodigo+reg_config.stack_size,clienteNucleoUMC.socketCliente,progParaCargar->PID)==-1){
			//no se pudo cargar notificar a la consola determinda
		}
		pcb_t* pcbNuevo = malloc(sizeof(pcb_t));
		int* pc = pcbNuevo->PC;
		int* pid = pcbNuevo->PID;
		int* sp = pcbNuevo->SP;
		int* pD = pcbNuevo->paginasDisponibles;
		pc = malloc(sizeof(int));
		pid= malloc(sizeof(int));
		sp= malloc(sizeof(int));
		pD = malloc(sizeof(int));
		*pc = 0;
		*pid = progParaCargar->PID;
		*sp = 0;
		*pD = ultimaPaginaDeCodigo+reg_config.stack_size;
		pcbNuevo->indicie_codigo = icNuevo->inst_tamanio;
		pthread_mutex_lock(&sem_l_New);
			printf("pcb creado y agregado\n");
			list_add(proc_New, pcbNuevo);
		pthread_mutex_unlock(&sem_l_New);
	}
	return NULL;
}

void conectarseConUmc(struct cliente clienteNucleo){
	conectarConServidor(clienteNucleo);
	int nucleoID = NUCLEO;
	//Empieza handshake
	if(send(clienteNucleo.socketCliente,&nucleoID,sizeof(int),0)==-1){
		printf("no se ha podido conectar con UMC.\n");
		perror("no anda:\0");
	}
	int* recibido = recibirStream(clienteNucleo.socketCliente,sizeof(int));
	if(*recibido==OK){
		printf("Se ha conectado correctamente con UMC.\n");
	}else{
		printf("No se ha podido conectar con UMC\n");
		exit(-1);
	}
	free(recibido);
	//Termina Handshake

	//Solicito tamanio de pagina, asi calculo las paginas por proceso
	int tamanioPagina = TAMANIOPAGINA;
	if(send(clienteNucleo.socketCliente,&tamanioPagina,sizeof(int),0)==-1){
		printf("no se ha podido solicitar tamanio de pag a la UMC.\n");
		perror("no anda:\n");
	}
	recibido = leerHeader(clienteNucleo.socketCliente);
	if(*recibido==TAMANIOPAGINA){
		int* recibirTamanioDePag = recibirStream(clienteNucleo.socketCliente,sizeof(int));
		tamanioPaginaUMC = *recibirTamanioDePag;
		printf("Tamanio de pagina configurado en: %d \n", tamanioPaginaUMC);
	}
	free(recibido);

	//Termina
}



