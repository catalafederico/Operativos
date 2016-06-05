/*
 * nucleo.c
 *
 *  Created on: 24/4/2016
 *      Author: Explosive Code - Lucas Marino
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "funcionesparsernuevas.h"
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
#include <sockets/header.h>

#include <semaphore.h>

#include "estructurasNUCLEO.h"
#include "configuracionesNucleo.h"
#include "procesosConsola.h"
#include "procesosCPU.h"
#include "procesosUMC.h"

// Variables compartidas ---------------------------------------------
t_reg_config reg_config;
t_list* cpus_dispo;
t_list* consolas_dispo;
t_list* programas_para_procesar;
t_list* proc_New;
t_list* proc_Ready;
t_list* proc_Exec;
t_list* proc_Block;
t_list* proc_Reject;
t_list* proc_Exit;
t_log *logger;

//sockets escuchas
struct cliente clienteNucleoUMC;
struct server serverPaCPU;
struct server serverPaConsolas;

// Semaforos
sem_t semaforoProgramasACargar; //semaforo contador
sem_t sem_READY_dispo; //semaforo contador
sem_t sem_EXIT_dispo; //semaforo contador
sem_t sem_BLOCK_dispo; //semaforo contador
//info: http://man7.org/linux/man-pages/man3/sem_init.3.html

pthread_mutex_t sem_l_cpus_dispo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_l_Ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_l_New = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_l_Exec = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_l_Block = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_l_Reject = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_l_Exit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_log = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t semProgramasAProcesar = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_pid_consola = PTHREAD_MUTEX_INITIALIZER;

int tamanioPaginaUMC;

// indice de consolas que asocia un PID a la consola que lo envio
t_dictionary* pid_consola;

// CONSTANTES -----
#define SOY_CPU 	"Te_conectaste_con_CPU____"
#define SOY_UMC 	"Te_conectaste_con_UMC____"
#define SOY_SWAP	"Te_conectaste_con_SWAP___"  // 25 de long sin \0
#define SOY_NUCLEO  "Te_conectaste_con_NUCLEO_"
#define SOY_CONSOLA	"Te_conectaste_con_CONSOLA"

// FUNCIONES
void *administrar_cola_Exit();
void *administrar_cola_Block();

// **************************************************************************************************
// ******************************************    MAIN     ***************************************
// **************************************************************************************************
int main(int argc, char **argv) {

	//Threads
	pthread_t thread_UMC;
	pthread_t thread_CPU;

	sem_init(&semaforoProgramasACargar,0,0);
	sem_init(&sem_READY_dispo,0,0);
	sem_init(&sem_EXIT_dispo,0,0);
	sem_init(&sem_BLOCK_dispo,0,0);

	//declaro indice etiquetas
	t_dictionary indiceEtiquetas;

	pid_consola = dictionary_create();

	// Inicializa el log.
	logger = log_create("nucleo.log", "NUCLEO", 1, LOG_LEVEL_TRACE);

	//crear listas
	cpus_dispo = list_create();
	proc_New = list_create();
	proc_Ready = list_create();
	proc_Block = list_create();
	proc_Reject = list_create();
	proc_Exit = list_create();
	programas_para_procesar = list_create();

	//Leo archivo de configuracion ------------------------------
	reg_config = get_config_params();

//Administrador de UMC----------------------
	if(pthread_create( &thread_UMC, NULL , procesos_UMC, NULL) < 0)
		{
			log_debug(logger, "No fue posible crear thread para UMC");
			exit(EXIT_FAILURE);
		}
	log_debug(logger, "Creacion Thread para procesos con UMC");




// Crear thread para atender los procesos CPU
	if( pthread_create( &thread_CPU, NULL , atender_conexion_CPU, NULL) < 0)
	{
		log_debug(logger, "No fue posible crear thread para CPU");
		exit(EXIT_FAILURE);
	}

	//Crear thread para atender los procesos consola
	pthread_t thread_consola;
	if( pthread_create( &thread_consola , NULL , atender_conexion_consolas, NULL) < 0)
	{
		log_debug(logger, "No fue posible crear thread p/ consolas");
		exit(EXIT_FAILURE);
	}

//Crear thread Administrador de cola EXIT
	pthread_t thread_EXIT_admin;
	if( pthread_create( &thread_EXIT_admin, NULL , administrar_cola_Exit, NULL) < 0)
	{
		log_debug(logger, "No fue posible crear thread Admin de EXIT");
		exit(EXIT_FAILURE);
	}

//Crear thread Administrador de cola BLOCK
	pthread_t thread_BLOCK_admin;
	if( pthread_create( &thread_BLOCK_admin, NULL , administrar_cola_Block, NULL) < 0)
	{
		log_debug(logger, "No fue posible crear thread Admin de EXIT");
		exit(EXIT_FAILURE);
	}

	pthread_join(thread_CPU, NULL);
	log_destroy(logger);
	return 0;

}

/*administrar_cola_Exit toma los PCB cargados en Exit, Retorna mensaje a la consola,  */
void *administrar_cola_Exit(){
	while (1){
		sem_wait(&sem_EXIT_dispo); // espero que haya un proceso en EXIT disponible
		pthread_mutex_lock(&sem_l_Exit);
			list_remove_by_condition(proc_Exec, (void *) (*pcb_elegido->PID == pid_local) );
			log_debug(logger, "PCB con PID %d sacado de EXEC xfin Quantum",pid_local);
		pthread_mutex_unlock(&sem_l_Exit);
	}

}

void *administrar_cola_Block(){

}
