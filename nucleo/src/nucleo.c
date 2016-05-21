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
sem_t sem_NEW_dispo; //semaforo contador
//info: http://man7.org/linux/man-pages/man3/sem_init.3.html
pthread_mutex_t sem_l_cpus_dispo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_l_Ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_l_New = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_l_Exec = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_l_Block = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_l_Reject = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_l_Exit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_log = PTHREAD_MUTEX_INITIALIZER;

int tamanioPaginaUMC;

// CONSTANTES -----
#define SOY_CPU 	"Te_conectaste_con_CPU____"
#define SOY_UMC 	"Te_conectaste_con_UMC____"
#define SOY_SWAP	"Te_conectaste_con_SWAP___"  // 25 de long sin \0
#define SOY_NUCLEO  "Te_conectaste_con_NUCLEO_"
#define SOY_CONSOLA	"Te_conectaste_con_CONSOLA"



// **************************************************************************************************
// ******************************************    MAIN     ***************************************
// **************************************************************************************************
int main(int argc, char **argv) {

	//Threads
	pthread_t thread_UMC;
	pthread_t thread_CPU;

	sem_init(&semaforoProgramasACargar,0,0);
	sem_init(&sem_NEW_dispo,0,0);

	//declaro indice etiquetas
	t_dictionary indiceEtiquetas;

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
//	pthread_t thread_EXIT_admin;
//	if( pthread_create( &thread_consola , NULL , administrar_cola_Exit, NULL) < 0)
//	{
//		log_debug(logger, "No fue posible crear thread Admin de EXIT");
//		exit(EXIT_FAILURE);
//	}

//Crear thread Administrador de cola BLOCK
//	pthread_t thread_BLOCK_admin;
//	if( pthread_create( &thread_consola , NULL , administrar_cola_Block, NULL) < 0)
//	{
//		log_debug(logger, "No fue posible crear thread Admin de EXIT");
//		exit(EXIT_FAILURE);
//	}

	pthread_join(thread_CPU, NULL);
	log_destroy(logger);
	return 0;

}

/*

\void roundRobin( int quantum){
int count,j,n,tiempo,restante,flag=0;
  int tiempo_espera=0,tiempo_cambio=0,at[10],bt[10],rt[10];
  printf("Enter Total Process:\t ");
  scanf("%d",&n);
  restante=n;
  for(count=0;count<n;count++)
  {
    printf("Enter Arrival Time and Burst Time for Process Process Number %d :",count+1);
    scanf("%d",&at[count]);
    scanf("%d",&bt[count]);
    rt[count]=bt[count];
  }
  printf("Enter Time Quantum:\t");
  scanf("%d",&quantum);
  printf("\n\nProcess\t|Turnaround Time|Waiting Time\n\n");

  for(tiempo=0,count=0;restante!=0;)
  {
    if(rt[count]<=quantum && rt[count]>0)
    {
      tiempo+=rt[count];
      rt[count]=0;
      flag=1;
    }
    else if(rt[count]>0)
    {
      rt[count]-=quantum;
      tiempo+=quantum;
    }
    if(rt[count]==0 && flag==1)
    {
      restante--;
      printf("P[%d]\t|\t%d\t|\t%d\n",count+1,tiempo-at[count],tiempo-at[count]-bt[count]);
      tiempo_espera+=tiempo-at[count]-bt[count];
      tiempo_cambio+=tiempo-at[count];
      flag=0;
    }
    if(count==n-1)
      count=0;
    else if(at[count+1]<=tiempo)
      count++;
    else
      count=0;
  }
  printf("\nAverage Waiting Time= %f\n",tiempo_espera*1.0/n);
  printf("Avg Turnaround Time = %f",tiempo_cambio*1.0/n);

  return;

}
*/


