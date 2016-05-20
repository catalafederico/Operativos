/*
 * procesosCPU.c
 *
 *  Created on: 18/5/2016
 *      Author: Lucas Marino
 */

#include <stdio.h>
//#include <string.h>
#include <stdlib.h>
#include<pthread.h>

#include <unistd.h>
//#include <errno.h>
//#include <netdb.h>
//#include <sys/types.h>
#include <netinet/in.h>
//#include <sys/socket.h>
//#include <sys/wait.h>
//#include <signal.h>
#include <commons/collections/list.h>
//#include <commons/config.h>
//#include <commons/string.h>
#include <commons/log.h>
#include <sockets/socketCliente.h>
#include <sockets/socketServer.h>
#include <sockets/basicFunciones.h>
#include "estructurasNUCLEO.h"
#include "procesosCPU.h"

// CONSTANTES -----
#define SOY_CPU 	"Te_conectaste_con_CPU____"
#define SOY_UMC 	"Te_conectaste_con_UMC____"
#define SOY_SWAP	"Te_conectaste_con_SWAP___"  // 25 de long sin \0
#define SOY_NUCLEO  "Te_conectaste_con_NUCLEO_"
#define SOY_CONSOLA	"Te_conectaste_con_CONSOLA"

#define FIN_PROC 	1
#define FIN_QUANTUM	2
#define FIN_IO		3
#define SOLIC_IO 	4
#define FIN_CPU 	5
// Variables compartidas ---------------------------------------------
extern t_reg_config reg_config;
extern t_list* cpus_dispo;
extern t_list* consolas_dispo;
extern t_list* proc_New;
extern t_list* proc_Ready;
extern t_list* proc_Exec;
extern t_list* proc_Block;
extern t_list* proc_Reject;
extern t_list* proc_Exit;
extern t_log *logger;

// semaforos Compartidos
extern sem_t* sem_NEW_dispo;

extern pthread_mutex_t sem_l_cpus_dispo;

extern pthread_mutex_t sem_l_New;
extern pthread_mutex_t sem_l_Ready;
extern pthread_mutex_t sem_l_Exec;
extern pthread_mutex_t sem_l_Block;
extern pthread_mutex_t sem_l_Exit;
extern pthread_mutex_t sem_l_Reject;
//extern pthread_mutex_t sem_log;
//------------------------------------------------------------------------------------------
// ---------------------------------- atender_conexion_CPU  --------------------------------
// Funcion que correra en un unico thread encargado de aceptar conexiones entrantes y generara
// un thread por cada CPU conectado y liberaran recursos automaticamente cuando dejen de ser utiles.
//------------------------------------------------------------------------------------------
void *atender_conexion_CPU(void *socket_desc){

	int nuevaConexion, *socket_nuevo; //socket donde va a estar nueva conexion
	struct sockaddr_in direccionEntrante;
	int socket_local = (int)socket_desc; //*(int*)socket_desc;
	aceptarConexion(&nuevaConexion, socket_local, &direccionEntrante);
	while(nuevaConexion){
		log_debug(logger, "Se ha conectado una CPU");

		pthread_attr_t attr;
		pthread_t thread_cpu_con;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		socket_nuevo = malloc(sizeof(int));
		*socket_nuevo = nuevaConexion;

		if( pthread_create( &thread_cpu_con , &attr , atender_CPU, (void*) socket_nuevo) < 0)
		{
			log_debug(logger, "No fue posible crear thread p/ CPU");
			exit(EXIT_FAILURE);
		}
		pthread_attr_destroy(&attr);

		log_debug(logger, "CPU %d atendido", *socket_nuevo);


	// agrego CPU a la lista de disponibles
		pthread_mutex_lock(&sem_l_cpus_dispo);
		list_add(cpus_dispo, socket_nuevo);
		pthread_mutex_unlock(&sem_l_cpus_dispo);

		socket_local = (int)socket_desc; //*(int*)socket_desc;
		aceptarConexion(&nuevaConexion, socket_local, &direccionEntrante);
		if (nuevaConexion < 0)	{
				log_debug(logger, "accept failed");
			//	exit(EXIT_FAILURE);
			}
	}

	if (nuevaConexion < 0)	{
		log_debug(logger, "accept failed");
		exit(EXIT_FAILURE);
	}
	return NULL;
}



//------------------------------------------------------------------------------------------
// ---------------------------------- atender_CPU  -----------------------------------------
//Esta funcion representa un thread que trabaja con un CPU conectado por socket
//------------------------------------------------------------------------------------------
void *atender_CPU(void *socket_desc){
	int socket_local = (int)socket_desc;
	int seguir = 1;
	pcb_t* pcb_elegido;
	int pid_local = 0;
	int estado_proceso;
	while(seguir){
		sem_wait(sem_NEW_dispo); // espero que haya un proceso en EXEC disponible
		pthread_mutex_lock(&sem_l_Exec);
	//	pcb_elegido = list_get(proc_New, 0);
		pcb_elegido = list_remove(proc_Exec, 0);
		pid_local = pcb_elegido->PID;
		pthread_mutex_unlock(&sem_l_Exec);

		enviarPCB(pcb_elegido, socket_local, reg_config.quantum, reg_config.quantum_sleep);
		pcb_elegido = recibirPCB(socket_local);
		estado_proceso = recibirEstadoProceso(socket_local);
		switch (estado_proceso) {
			case FIN_QUANTUM:
				pthread_mutex_lock(&sem_l_Ready);
				list_add(proc_Ready, pcb_elegido);
				pthread_mutex_unlock(&sem_l_Ready);
				break;

			case FIN_IO:// VER SI ESTO SE MANEJA DESDE OTRO LADO,
				pthread_mutex_lock(&sem_l_Block); // lo quito de bloqueados
				list_remove_by_condition(proc_Block, (void *)(pcb_elegido->PID = pid_local) );
				pthread_mutex_unlock(&sem_l_Block);

				pthread_mutex_lock(&sem_l_Ready); // lo agrego a listos
				list_add(proc_Ready, pcb_elegido);
				pthread_mutex_unlock(&sem_l_Ready);
				break;

			case SOLIC_IO://VER SI ESTO SE MANEJA DESDE OTRO LADO
				pthread_mutex_lock(&sem_l_Block); // se bloquea
				list_add(proc_Block, pcb_elegido);
				pthread_mutex_unlock(&sem_l_Block);
				break;

			case FIN_PROC:
				pthread_mutex_lock(&sem_l_Exit);
				list_add(proc_Exit, pcb_elegido);
				pthread_mutex_unlock(&sem_l_Exit);
				break;

			case FIN_CPU:
				pthread_mutex_lock(&sem_l_Ready); // lo agrego al principio de listos
				list_add_in_index(proc_Ready, pcb_elegido, 0);
				pthread_mutex_unlock(&sem_l_Ready);
				break;

			default:
				break;
		}
	}

//Get the socket descriptor
//	int socket_co = *(int*)socket_desc;
//		int read_size;
//		t_head_mje header;

//	char * mensajeHandShake = hacerHandShake_server(socket_co, SOY_NUCLEO); por ahora se saca

/*	// Recibir Mensaje de cpu.
	int tamanio_mje = 256;
	char * mje_recibido = recibirMensaje_tamanio(socket_co, &tamanio_mje);

	if(!strcmp("Se desconecto",mje_recibido)){
		perror("Se cerro la conexion");
		free((void *) mje_recibido);
      close(socket_co);
      exit(0);
	}
	printf ("Mensaje recibido de consola %d : %s \n", socket_co, mje_recibido);

	int i = 0;
	// me fijo si hay Cpu disponible o espero un ratito, si no hay se retorna error
	while (list_is_empty(cpus_dispo)&&(i<=10000)) i++;
	if (i>10000){
		perror("No hay CPU disponible");
					free((void *) mje_recibido);
			        close(socket_co);
			        exit(0);
	}

	//Envio mensaje a todas las CPU disponibles.
	int fin_list = list_size(cpus_dispo);
	int cpu_destino;
	i = 1;

	while (i<=fin_list){
		cpu_destino = list_get(cpus_dispo, i);
		enviarMensaje(cpu_destino, mje_recibido);
		i++;
	}

	free((void *) mje_recibido);*/
//	close(socket_co);
	return NULL;
}



void enviarPCB(pcb_t* pcb,int cpu, int quantum, int quantum_sleep){
	serializablePCB aMandaCPU;
	aMandaCPU.id = pcb->id;
	aMandaCPU.ip = pcb->PC;
	aMandaCPU.sp = pcb->SP;
	aMandaCPU.paginasDisponible = pcb->paginasDisponibles;
	aMandaCPU.tamanioIC = dictionary_size(pcb->indicie_codigo);
	int enviaPCB = ENVIOPCB;
	enviarStream(cpu,enviaPCB,sizeof(serializablePCB),&aMandaCPU);
	//serializo diccionario y lo mando
	int tamanioIndiceCode =  dictionary_size(pcb->indicie_codigo);
	int i;
	for(i=0;i<tamanioIndiceCode;i++){
		direccionMemoria* aMandar = dictionary_get(pcb->indicie_codigo,&i);
		send(cpu,aMandar,sizeof(direccionMemoria),0);
	}
// enviar quantum
	send(cpu,quantum,sizeof(int),0);
	send(cpu,quantum_sleep,sizeof(int),0);
	return;
}
