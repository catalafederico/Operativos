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
t_list* consolas_dispo;
t_list* programas_para_procesar;
t_list* proc_New;
t_list* proc_Ready;
t_list* proc_Exec;
t_list* proc_Block;
t_list* proc_Reject;
t_list* proc_Exit;
t_log *logger;

// logs para pruebas
t_log * log_procesador_Exit;
t_log * log_procesador_Block;
t_log * log_procesador_Reject;

//sockets escuchas
struct cliente clienteNucleoUMC;
struct server serverPaCPU;
struct server serverPaConsolas;

// Semaforos
sem_t semaforoProgramasACargar; //semaforo contador
sem_t sem_READY_dispo; //semaforo contador
sem_t sem_EXIT_dispo; //semaforo contador
sem_t sem_BLOCK_dispo; //semaforo contador
sem_t sem_REJECT_dispo; //semaforo contador
//sem_t sem_cpus_dispo;
//info: http://man7.org/linux/man-pages/man3/sem_init.3.html

//pthread_mutex_t sem_l_cpus_dispo = PTHREAD_MUTEX_INITIALIZER;
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
t_dictionary* dict_pid_consola;

// CONSTANTES -----
#define SOY_CPU 	"Te_conectaste_con_CPU____"
#define SOY_UMC 	"Te_conectaste_con_UMC____"
#define SOY_SWAP	"Te_conectaste_con_SWAP___"  // 25 de long sin \0
#define SOY_NUCLEO  "Te_conectaste_con_NUCLEO_"
#define SOY_CONSOLA	"Te_conectaste_con_CONSOLA"

#define MJE_RTA	256       // tamanio fijo para los mensajes de retorno a consola

// FUNCIONES
void *administrar_cola_Exit();
void *administrar_cola_Block();
void *administrar_cola_Reject();
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
//	sem_init(&sem_cpus_dispo,0,0);

	//declaro indice etiquetas
	t_dictionary indiceEtiquetas;

	dict_pid_consola = dictionary_create();

	// Inicializa el log.
	logger = log_create("nucleo.log", "NUCLEO", 1, LOG_LEVEL_TRACE);

	//inizializa logs de prueba
	log_procesador_Exit = log_create("procesador_EXIT.log", "procesador_EXIT", 1, LOG_LEVEL_TRACE);
	log_procesador_Block = log_create("procesador_BLOCK.log", "procesador_BLOCK", 1, LOG_LEVEL_TRACE);
	log_procesador_Reject = log_create("procesador_REJECT.log", "procesador_REJECT", 1, LOG_LEVEL_TRACE);
	//crear listas
//	cpus_dispo = list_create();
	proc_New = list_create();
	proc_Ready = list_create();
	proc_Block = list_create();
	proc_Reject = list_create();
	proc_Exit = list_create();
	programas_para_procesar = list_create();
//	EXIT
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
	log_debug(logger, "Creacion Thread para Atender Conexiones CPU");


	//Crear thread para atender los procesos consola
	pthread_t thread_consola;
	if( pthread_create( &thread_consola , NULL , atender_conexion_consolas, NULL) < 0)
	{
		log_debug(logger, "No fue posible crear thread p/ consolas");
		exit(EXIT_FAILURE);
	}
	log_debug(logger, "Creacion Thread para Atender Conexiones Consola");

//Crear thread Administrador de cola EXIT
	pthread_t thread_EXIT_admin;
	if( pthread_create( &thread_EXIT_admin, NULL , administrar_cola_Exit, NULL) < 0)
	{
		log_debug(logger, "No fue posible crear thread Admin de EXIT");
		exit(EXIT_FAILURE);
	}
	log_debug(logger, "Creacion Thread para Procesar cola EXIT");

//Crear thread Administrador de cola BLOCK
	pthread_t thread_BLOCK_admin;
	if( pthread_create( &thread_BLOCK_admin, NULL , administrar_cola_Block, NULL) < 0)
	{
		log_debug(logger, "No fue posible crear thread Admin de EXIT");
		exit(EXIT_FAILURE);
	}
	log_debug(logger, "Creacion Thread para Procesar cola BLOCK");

//Crear thread Administrador de cola REJECT
	pthread_t thread_REJECT_admin;
	if( pthread_create( &thread_REJECT_admin, NULL , administrar_cola_Reject, NULL) < 0)
	{
		log_debug(logger, "No fue posible crear thread Admin de REJECT");
		exit(EXIT_FAILURE);
	}
	log_debug(logger, "Creacion Thread para Procesar cola REJECT");


	pthread_join(thread_CPU, NULL);
	log_destroy(logger);
	return 0;

}

/* administrar_cola_Exit toma los PCB cargados en Exit, Retorna mensaje a la consola  */
void *administrar_cola_Exit(){
	log_debug(log_procesador_Exit, "administrar_cola_Exit esta corriendo");

	pcb_t* pcb_elegido;
	int pid_local = 0;
	while (1){
		sem_wait(&sem_EXIT_dispo); // espero que haya un proceso en EXIT disponible
		log_debug(log_procesador_Exit, "Se empezo a procesar un PCB de EXIT");
		// quito el proceso de la cola EXIT
		pthread_mutex_lock(&sem_l_Exit);
			pcb_elegido = list_remove(proc_Exit, 0);//Agarro el pcb
			pid_local = *(pcb_elegido->PID);
		pthread_mutex_unlock(&sem_l_Exit);

		log_debug(log_procesador_Exit, "Se removio el PCB de EXIT: %d", pid_local);
		// quito el proceso del Diccionario, obtengo Consola_Id y mensaje de respuesta
		t_sock_mje* datos_a_consola;
		pthread_mutex_lock(&sem_pid_consola);
			datos_a_consola = dictionary_get(dict_pid_consola,string_itoa(pid_local));
			dictionary_remove(dict_pid_consola,string_itoa(pid_local));
		pthread_mutex_unlock(&sem_pid_consola);
		log_debug(log_procesador_Exit, "Se removio el PID ( %d ) del dicc y se envio el mje ( %s ) a consola: %d", pid_local, datos_a_consola->mensaje, datos_a_consola->socket_dest);
		log_debug(logger, "PCB con PID %d sacado de EXIT y se respondio a la consola %d",pid_local, datos_a_consola->socket_dest);
		log_debug(logger, "Se envio a consola: %d el mensaje: %s", datos_a_consola->socket_dest,datos_a_consola->mensaje);
	    if (send(datos_a_consola->socket_dest, datos_a_consola->mensaje, MJE_RTA, 0) == -1){
	    	log_debug(logger, "se intento enviar mensaje a consola: %d, pero el Send dio Error", datos_a_consola->socket_dest);
	    	log_debug(log_procesador_Exit, "se intento enviar mensaje a consola: %d, pero el Send dio Error", datos_a_consola->socket_dest);
	    }
	    log_debug(log_procesador_Exit, "Envio correcto a consola: %d", datos_a_consola->socket_dest);

	}

}

/* administrar_cola_Block toma los procesos que se encuentran bloqueados y procesa su bloqueo*/
void *administrar_cola_Block(){
	log_debug(log_procesador_Block, "administrar_cola_Block esta corriendo");

//	pcb_t* pcb_elegido;
	int pid_local = 0;
	while (1){
		sem_wait(&sem_BLOCK_dispo); // espero que haya un proceso en BLOCK disponible
		log_debug(log_procesador_Block, "Se empezo a procesar un PCB de BLOCK");

		// quito el proceso de la cola Block


	}

}

/* administrar_cola_Reject toma los procesos que se encuentran Rechazados y se lo informa a su Consola */
void *administrar_cola_Reject (){
	log_debug(log_procesador_Reject , "administrar_cola_Reject  esta corriendo");

	pcb_t* pcb_elegido;
	int pid_local = 0;
	while (1){
		sem_wait(&sem_REJECT_dispo); // espero que haya un proceso en REJECT disponible
		log_debug(log_procesador_Reject, "Se empezo a procesar un PCB de REJECT");
		// quito el proceso de la cola REJECT
		pthread_mutex_lock(&sem_l_Reject);
			pcb_elegido = list_remove(proc_Reject, 0);//Agarro el pcb
			pid_local = *(pcb_elegido->PID);
		pthread_mutex_unlock(&sem_l_Reject);

		log_debug(log_procesador_Reject, "Se removio el PCB de REJECT: %d", pid_local);

		// quito el proceso del Diccionario, obtengo Consola_Id y mensaje de respuesta
		t_sock_mje* datos_a_consola;
		pthread_mutex_lock(&sem_pid_consola);
			datos_a_consola = dictionary_get(dict_pid_consola,string_itoa(pid_local));
			dictionary_remove(dict_pid_consola,string_itoa(pid_local));
		pthread_mutex_unlock(&sem_pid_consola);

		char *mje_Rej = string_new();
		string_append(&mje_Rej, "SIN ESPACIO");
		int cant_pasada = MJE_RTA - 11;
		string_append(&mje_Rej, string_repeat(" ", cant_pasada));
		log_debug(log_procesador_Reject, "Se removio el PID ( %d ) del dicc y se envio el mje ( SIN ESPACIO ) a consola: %d", pid_local, datos_a_consola->socket_dest);
		log_debug(logger, "PCB con PID %d sacado de REJECT y se respondio a la consola %d",pid_local, datos_a_consola->socket_dest);
		log_debug(logger, "Se envio a consola: %d el mensaje: %s", datos_a_consola->socket_dest, mje_Rej);
	    if (send(datos_a_consola->socket_dest, datos_a_consola->mensaje, MJE_RTA, 0) == -1){
	    	log_debug(logger, "se intento enviar mensaje a consola: %d, pero el Send dio Error", datos_a_consola->socket_dest);
	    	log_debug(log_procesador_Reject, "se intento enviar mensaje a consola: %d, pero el Send dio Error", datos_a_consola->socket_dest);
	    }
	    log_debug(log_procesador_Reject, "Envio correcto a consola: %d", datos_a_consola->socket_dest);

	}

}
