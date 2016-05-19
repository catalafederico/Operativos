/*
 * nucleo.c
 *
 *  Created on: 24/4/2016
 *      Author: Explosive Code - Lucas Marino
 */
#include <string.h>
#include "estructuras.h"
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
#include "configuracionesNucleo.h"


// Variables compartidas ---------------------------------------------

t_list* cpus_dispo;
t_list* consolas_dispo;
t_list* proc_New;
t_list* proc_Ready;
t_list* proc_Block;
t_list* proc_Reject;
t_list* proc_Exit;
t_log *logger;
int tamanioPaginaUMC;

// CONSTANTES -----
#define SOY_CPU 	"Te_conectaste_con_CPU____"
#define SOY_UMC 	"Te_conectaste_con_UMC____"
#define SOY_SWAP	"Te_conectaste_con_SWAP___"  // 25 de long sin \0
#define SOY_NUCLEO  "Te_conectaste_con_NUCLEO_"
#define SOY_CONSOLA	"Te_conectaste_con_CONSOLA"


// ****************************************** FUNCIONES.h  ******************************************
void *atender_conexion_consolas(void *socket_desc);

void *atender_conexion_CPU(void *socket_desc);

void *atender_consola(void *socket_desc);

void *atender_CPU(void *socket_desc);

void conectarseConUmc(struct cliente clienteNucleo);

//void roundRobin( int quantum);

// ****************************************** FIN FUNCIONES.h ***************************************

// **************************************************************************************************
// ****************************************** MAIN           ***************************************
// **************************************************************************************************
int main(int argc, char **argv) {
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

		// Leo archivo de configuracion ------------------------------
	t_reg_config reg_config;
	reg_config = get_config_params();


	// Me conecto con la UMC
	/*struct cliente clienteNucleo;
	clienteNucleo = crearCliente(9999, "127.0.0.1");
	conectarseConUmc(clienteNucleo);*/
	t_list* instruccionAUMC = list_create();
	tamanioPaginaUMC = 50;
	indiceCodigo* icNuevo = nuevoPrograma("variables a,b\n variables c\n",instruccionAUMC);
	paginarIC(icNuevo->inst_tamanio);

// Crear socket para CPU  ------------------------------
	/*struct server serverPaCPU;
	serverPaCPU = crearServer(reg_config.puerto_cpu);
	ponerServerEscucha(serverPaCPU);
	log_debug(logger, "Escuchando Cpus en socket %d", serverPaCPU.socketServer);


// Crear thread para atender los procesos CPU
	pthread_t thread_CPU;
	if( pthread_create( &thread_CPU, NULL , atender_conexion_CPU, (void*) serverPaCPU.socketServer) < 0)
	{
		log_debug(logger, "No fue posible crear thread para CPU");
		exit(EXIT_FAILURE);
	}

// Crear socket para procesos (CONSOLA) ------------------------------
	struct server serverPaConsolas;
	serverPaConsolas = crearServer(reg_config.puerto_prog);
	ponerServerEscucha(serverPaConsolas);
	log_debug(logger, "Escuchando Consolas en socket %d", serverPaConsolas.socketServer);



// Crear thread para atender los procesos consola
	pthread_t thread_consola;
	if( pthread_create( &thread_consola , NULL , atender_conexion_consolas, (void*) serverPaConsolas.socketServer) < 0)
	{
		log_debug(logger, "No fue posible crear thread p/ consolas");
		exit(EXIT_FAILURE);
	}

	pthread_join(thread_CPU, NULL);
	pthread_join(thread_consola, NULL);

	log_destroy(logger);*/
	return 0;

}

//------------------------------------------------------------------------------------------
// ---------------------------------- atender_conexion_consolas  ---------------------------
void *atender_conexion_consolas(void *socket_desc){

	int nuevaConexion, *socket_nuevo; //socket donde va a estar nueva conexion
	struct sockaddr_in direccionEntrante;
	int socket_local = (int)socket_desc;
	aceptarConexion(&nuevaConexion, socket_local, &direccionEntrante);
	while(nuevaConexion){

		log_debug(logger, "Se ha conectado una Consola");

//		pthread_attr_t attr;
		pthread_t thread_consola_con;
//		pthread_attr_init(&attr);
//		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		socket_nuevo = malloc(sizeof(int));
		*socket_nuevo = nuevaConexion;
		if( pthread_create( &thread_consola_con , NULL /*&attr*/ , atender_consola, (void*) socket_nuevo) < 0)
		{
			log_debug(logger, "No fue posible crear thread p/ consolas");
			exit(EXIT_FAILURE);
		}
		log_debug(logger, "Consola %d atendida", *socket_nuevo);

		socket_local = (int)socket_desc;
		aceptarConexion(&nuevaConexion, socket_local, &direccionEntrante);
		if (nuevaConexion < 0)	{
			log_debug(logger, "accept failed");
			//	exit(EXIT_FAILURE);
			}
	}

	if (nuevaConexion < 0)	{
		log_debug(logger, "Accept failed");
		exit(EXIT_FAILURE);
	}
	return NULL;
}

//------------------------------------------------------------------------------------------
// ---------------------------------- atender_conexion_CPU  --------------------------------
void *atender_conexion_CPU(void *socket_desc){

	int nuevaConexion, *socket_nuevo; //socket donde va a estar nueva conexion
	struct sockaddr_in direccionEntrante;
	int socket_local = (int)socket_desc; //*(int*)socket_desc;
	aceptarConexion(&nuevaConexion, socket_local, &direccionEntrante);
	while(nuevaConexion){
		log_debug(logger, "Se ha conectado una CPU");

		pthread_t thread_cpu_con;
		socket_nuevo = malloc(sizeof(int));
		*socket_nuevo = nuevaConexion;

		if( pthread_create( &thread_cpu_con , NULL , atender_CPU, (void*) socket_nuevo) < 0)
		{
			log_debug(logger, "No fue posible crear thread p/ CPU");
			exit(EXIT_FAILURE);
		}
		log_debug(logger, "CPU %d atendido", *socket_nuevo);


	// agrego CPU a la lista de disponibles
		list_add(cpus_dispo, socket_nuevo);
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
// ---------------------------------- atender_consola---------------------------------------
void *atender_consola(void *socket_desc){
	  //Get the socket descriptor
		int socket_co = *(int*)socket_desc;
//		int read_size;
//		t_head_mje header;

		char * mensajeHandShake = hacerHandShake_server(socket_co, SOY_NUCLEO);
//liberar mensajeHandShake
		// Recibir Mensaje de consola.
		int tamanio_mje = 256;
		char * mje_recibido = recibirMensaje_tamanio(socket_co, &tamanio_mje);

		if(!strcmp("Se desconecto",mje_recibido)){
			log_debug(logger, "Se cerro la conexion");
			free((void *) mje_recibido);
			free(socket_desc);
	        close(socket_co);
	        exit(0);
		}
		log_debug(logger, "Mensaje recibido de consola %d : %s", socket_co, mje_recibido);


		int i = 0;
		// me fijo si hay Cpu disponible

		while (list_is_empty(cpus_dispo)){
			sleep(5);
			log_debug(logger, "No hay CPU disponoble, reintentando...");
						}
		//Envio mensaje a todas las CPU disponibles. verr

		int fin_list = list_size(cpus_dispo);

		int* cpu_destino;
		i = 0;
		while (i<fin_list){
			cpu_destino = list_get(cpus_dispo, i);

			enviarMensaje(*cpu_destino, mje_recibido);
			i++;
		}

		free((void *) mje_recibido);
		free(socket_desc);
		close(socket_co);
		return NULL;
}



//------------------------------------------------------------------------------------------
// ---------------------------------- atender_CPU  -----------------------------------------
void *atender_CPU(void *socket_desc){
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

	//TErmina
}



