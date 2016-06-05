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
#include "semaphore.h"
#include <sockets/header.h>

//FUNCIONES

pcb_t* recibirPCB(int socketCpu);
int* recibirEstadoProceso(int socket_local);


// CONSTANTES -----
#define SOY_CPU 	"Te_conectaste_con_CPU____"
#define SOY_UMC 	"Te_conectaste_con_UMC____"
#define SOY_SWAP	"Te_conectaste_con_SWAP___"  // 25 de long sin \0
#define SOY_NUCLEO  "Te_conectaste_con_NUCLEO_"
#define SOY_CONSOLA	"Te_conectaste_con_CONSOLA"

#define FIN_PROC 	 1
#define FIN_QUANTUM	 2
#define SOLIC_IO 	 3
#define FIN_CPU 	 4
#define OBT_VALOR 	 5
#define GRABA_VALOR  6
#define WAIT_SEM 	 7
#define SIGNAL_SEM 	 8
#define IMPRIMIR	 9
#define IMPRIMIR_TXT 10

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
extern struct server serverPaCPU;
// semaforos Compartidos
extern sem_t sem_READY_dispo;

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
void *atender_conexion_CPU(){
	log_debug(logger, "Crear socket para CPUs");
	// Crear socket para CPU  ------------------------------
	serverPaCPU = crearServer(reg_config.puerto_cpu);

	//Pongo el server a escuchar.
	ponerServerEscucha(serverPaCPU);
	log_debug(logger, "Se han empezado a escuchar cpus.");

	pthread_attr_t attr;
	pthread_t thread_cpu_con;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	int seguir = 1;
	while(seguir){
		int* socket_nuevo = malloc(sizeof(int)); //socket donde va a estar nueva conexion
		struct sockaddr_in direccionEntrante;
		aceptarConexion(socket_nuevo, serverPaCPU.socketServer, &direccionEntrante); //No hace falta chekear si es -1, aceptarConexiones lo hace ya
		log_debug(logger, "Se ha conectado una CPU");

		if(pthread_create(&thread_cpu_con , &attr , (void*) atender_CPU, socket_nuevo) < 0)
		{
			log_debug(logger, "No fue posible crear thread p/ CPU");
			exit(EXIT_FAILURE);
		}
		//No lo destruyo total todos usan el mismo atributo asi no lo creo todo el tiempo
		//pthread_attr_destroy(&attr);
		log_debug(logger, "CPU %d atendido", *socket_nuevo);


		//agrego CPU a la lista de disponibles
		pthread_mutex_lock(&sem_l_cpus_dispo);
		list_add(cpus_dispo, socket_nuevo);
		pthread_mutex_unlock(&sem_l_cpus_dispo);
	}
	return NULL;
}



//------------------------------------------------------------------------------------------
// ---------------------------------- atender_CPU  -----------------------------------------
//Esta funcion representa un thread que trabaja con un CPU conectado por socket
//------------------------------------------------------------------------------------------
void *atender_CPU(int* socket_desc){
	int socket_local = *socket_desc;
	//Confirmo conexio a cpu
	int ok = OK;
	send(socket_local,&ok,sizeof(int),0);

	//Lo libero ya q era un malloc de atender_conexion_CPU
	free(socket_desc);
	int seguir = 1;
	pcb_t* pcb_elegido;
	int pid_local = 0;
	int* estado_proceso;
	while(seguir){
		sem_wait(&sem_READY_dispo); // espero que haya un proceso en READY disponible
		pthread_mutex_lock(&sem_l_Ready);
			pcb_elegido = list_remove(proc_Ready, 0);//Agarro el pcb
			pid_local = *(pcb_elegido->PID);
			log_debug(logger, "PCB con PID %d sacado de NEW",pid_local);
		pthread_mutex_unlock(&sem_l_Ready);

		enviarPCB(pcb_elegido, socket_local, reg_config.quantum, reg_config.quantum_sleep);
		pthread_mutex_lock(&sem_l_Exec);
			list_add(proc_Exec, pcb_elegido);
			log_debug(logger, "PCB con PID %d pasado a EXEC",pid_local);
		pthread_mutex_unlock(&sem_l_Exec);

		pcb_elegido = recibirPCB(socket_local); /* cuando se implementen las opera ansisop esto debe ir
												despues del recibir estado ya que el PCB es necesario solo luego*/
		estado_proceso = recibirEstadoProceso(socket_local);

// se evalua si se solicito una operacion privilegiada de Ansisop
		while(estado_proc_es_Ansisop(*estado_proceso)){
			switch (*estado_proceso) {
	//      Las siguientes son operaciones privilegiadas
				case SOLIC_IO:	//es la primitiva entradaSalida
	//              ansisop_entradaSalida ();
	//				pthread_mutex_lock(&sem_l_Block); // se bloquea
	//				list_add(proc_Block, pcb_elegido);
	//				pthread_mutex_unlock(&sem_l_Block);
	//				log_debug(logger, "El proceso %d de la Consola %d pasa a BLOCK", *pcb_elegido->PID, *pcb_elegido->con_id);
					break;

				case OBT_VALOR:  //es la primitiva obtenerValorCompartida
	//              ansisop_obtenerValorCompartida ();
					break;

				case GRABA_VALOR: //es la primitiva asignarValorCompartida
	//              ansisop_asignarValorCompartida ();
					break;

				case WAIT_SEM:	 // es la primitiva wait
	//              ansisop_wait ();
					break;

				case SIGNAL_SEM: // es la primitiva signal
	//              ansisop_signal ();
					break;

				case IMPRIMIR: // es la primitiva imprimir
	//              ansisop_imprimir();
					break;

				case IMPRIMIR_TXT: // es la primitiva imprimirTexto
	//              ansisop_imprimirTexto ();
					break;

				default:
					break;
			}
			estado_proceso = recibirEstadoProceso(socket_local);
		}
		switch (*estado_proceso) {
			case FIN_QUANTUM:
				pthread_mutex_lock(&sem_l_Exec);
					list_remove_by_condition(proc_Exec, (void *) (*pcb_elegido->PID == pid_local) );
					log_debug(logger, "PCB con PID %d sacado de EXEC xfin Quantum",pid_local);
				pthread_mutex_unlock(&sem_l_Exec);
				//
				pthread_mutex_lock(&sem_l_Ready);
					list_add(proc_Ready, pcb_elegido);
					log_debug(logger, "PCB con PID %d pasado a READY xfin Quantum",pid_local);
				pthread_mutex_unlock(&sem_l_Ready);
				sem_post(&sem_READY_dispo);
				break;

//			case FIN_IO:// VER SI ESTO SE MANEJA DESDE OTRO LADO,
//				pthread_mutex_lock(&sem_l_Block); // lo quito de bloqueados
//				list_remove_by_condition(proc_Block, (void *) (*pcb_elegido->PID == pid_local) );
//				pthread_mutex_unlock(&sem_l_Block);
//
//				pthread_mutex_lock(&sem_l_Ready); // lo agrego a listos
//				list_add(proc_Ready, pcb_elegido);
//				pthread_mutex_unlock(&sem_l_Ready);
//				log_debug(logger, "El proceso %d de la Consola %d pasa a READY", *pcb_elegido->PID, *pcb_elegido->con_id);
//				break;

			case FIN_PROC:
				pthread_mutex_lock(&sem_l_Exec);
					list_remove_by_condition(proc_Exec, (void *) (*pcb_elegido->PID == pid_local) );
					log_debug(logger, "PCB con PID %d sacado de EXEC xfin Proceso",pid_local);
				pthread_mutex_unlock(&sem_l_Exec);
				//
				pthread_mutex_lock(&sem_l_Exit);
					list_add(proc_Exit, pcb_elegido);
					log_debug(logger, "PCB con PID %d pasado a EXIT xfin Proceso",pid_local);
				pthread_mutex_unlock(&sem_l_Exit);
				break;

			case FIN_CPU:
				pthread_mutex_lock(&sem_l_Exec);
					list_remove_by_condition(proc_Exec, (void *) (*pcb_elegido->PID == pid_local) );
					log_debug(logger, "PCB con PID %d sacado de EXEC xfin CPU",pid_local);
				pthread_mutex_unlock(&sem_l_Exec);
				//
				pthread_mutex_lock(&sem_l_Ready);
					list_add_in_index(proc_Ready, 0, pcb_elegido);
					log_debug(logger, "PCB con PID %d pasado al principio de READY xfin CPU",pid_local);
				pthread_mutex_unlock(&sem_l_Ready);

				sem_post(&sem_READY_dispo);
				break;

			default:
				break;
		}

		free(estado_proceso);
	}
	return NULL;
}



void enviarPCB(pcb_t* pcb,int cpu, int quantum, int quantum_sleep){
	serializablePCB aMandaCPU;
	aMandaCPU.PID = *(pcb->PID);
	aMandaCPU.PC = *(pcb->PC);
	aMandaCPU.SP = *(pcb->SP);
	aMandaCPU.paginasDisponible = *(pcb->paginasDisponibles);
	aMandaCPU.tamanioIC = dictionary_size(pcb->indicie_codigo);
	int enviaPCB = 163;
	enviarStream(cpu,enviaPCB,sizeof(serializablePCB),&aMandaCPU);
	//serializo diccionario y lo mando
	int tamanioIndiceCode =  dictionary_size(pcb->indicie_codigo);
	int i;
	for(i=0;i<tamanioIndiceCode;i++){
		direccionMemoria* aMandar = dictionary_get(pcb->indicie_codigo,&i);
		send(cpu,aMandar,sizeof(direccionMemoria),0);
	}
	//enviar quantum
	send(cpu,&quantum,sizeof(int),0);
	send(cpu,&quantum_sleep,sizeof(int),0);
	return;
}


pcb_t* recibirPCB(int socketCpu){
	pcb_t* pcb_Recibido;
	pcb_Recibido->PID = recibirStream(socketCpu,sizeof(int));
	pcb_Recibido->PC = recibirStream(socketCpu,sizeof(int));
	pcb_Recibido->SP = recibirStream(socketCpu,sizeof(int));
	pcb_Recibido->paginasDisponibles = recibirStream(socketCpu,sizeof(int));
	int* tamanioIC = recibirStream(socketCpu, sizeof(int));
	pcb_Recibido->indicie_codigo = dictionary_create();
	int i;
	for(i=0;i<*tamanioIC;i++){
		int* nuevaPagina = malloc(sizeof(int));
		*nuevaPagina = i;
		direccionMemoria* nuevaDireccionMemoria = recibirStream(socketCpu,sizeof(direccionMemoria));
		dictionary_put(pcb_Recibido->indicie_codigo,nuevaPagina,nuevaDireccionMemoria);
	}
	free(tamanioIC);
	return pcb_Recibido;
}


int* recibirEstadoProceso(int socket_local){
	//se libera estado al final del while, por eso lo cambie a puntero
	int* estado = recibirStream(socket_local,sizeof(int));
	return estado;
}

int estado_proc_es_Ansisop(int estado_proceso){
	return 0; //por ahora que retorne que no es ansisop hasta que se defina junto al CPU el orden
	switch (estado_proceso) {
		case SOLIC_IO:	//es la primitiva entradaSalida
		case OBT_VALOR:  //es la primitiva obtenerValorCompartida
		case GRABA_VALOR: //es la primitiva asignarValorCompartida
		case WAIT_SEM:	 // es la primitiva wait
		case SIGNAL_SEM: // es la primitiva signal
		case IMPRIMIR: // es la primitiva imprimir
		case IMPRIMIR_TXT: // es la primitiva imprimirTexto
			return 1;
			break;

		default:
			return 0;
			break;
	}
}
