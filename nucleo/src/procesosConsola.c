/*
 * procesosConsola.............c
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
#include <sys/types.h>
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

#include "procesosConsola.h"

// CONSTANTES -----
#define SOY_CPU 	"Te_conectaste_con_CPU____"
#define SOY_UMC 	"Te_conectaste_con_UMC____"
#define SOY_SWAP	"Te_conectaste_con_SWAP___"  // 25 de long sin \0
#define SOY_NUCLEO  "Te_conectaste_con_NUCLEO_"
#define SOY_CONSOLA	"Te_conectaste_con_CONSOLA"

// Variables compartidas ---------------------------------------------

extern t_list* cpus_dispo;
extern t_list* consolas_dispo;
extern t_list* proc_New;
extern t_list* proc_Ready;
extern t_list* proc_Block;
extern t_list* proc_Reject;
extern t_list* proc_Exit;
extern t_log *logger;
extern struct server serverPaConsolas;

// semaforos Compartidos
extern pthread_mutex_t sem_l_cpus_dispo;
extern pthread_mutex_t sem_l_Ready;
extern pthread_mutex_t sem_l_New;
extern pthread_mutex_t sem_l_Exec;
extern pthread_mutex_t sem_l_Block;
extern pthread_mutex_t sem_l_Reject;
extern pthread_mutex_t sem_l_Exit;
extern pthread_mutex_t sem_log;
//------------------------------------------------------------------------------------------
// ---------------------------------- atender_conexion_consolas  ---------------------------
void *atender_conexion_consolas(void *socket_desc){

	//Pongo el server a escuchar.
	ponerServerEscucha(serverPaConsolas);
	log_debug(logger, "Se han empezado a escuchar consolas.");
	int seguir = 1;

	pthread_attr_t attr;
	pthread_t thread_consola_con;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	while(seguir){
		int *socket_nuevo = malloc(sizeof(int)); //socket donde va a estar nueva conexion
		struct sockaddr_in direccionEntrante;
		aceptarConexion(socket_nuevo, serverPaConsolas.socketServer, &direccionEntrante); //No hace falta chekear si es -1, aceptarConexiones lo hace ya
		log_debug(logger, "Se ha conectado una Consola");

		if( pthread_create( &thread_consola_con , &attr , atender_consola, socket_nuevo) < 0)
		{
			log_debug(logger, "No fue posible crear thread p/ consolas");
			exit(EXIT_FAILURE);
		}
		//No lo destruyo total todos usan el mismo atributo asi no lo creo todo el tiempo
		//pthread_attr_destroy(&attr);

		log_debug(logger, "Consola %d atendida", *socket_nuevo);
	}
	return NULL;
}

//------------------------------------------------------------------------------------------
// ---------------------------------- atender_consola---------------------------------------
void *atender_consola(int* socket_desc){
	  	//Get the socket descriptor
		int socket_co = *(int*)socket_desc;
		//int read_size;
		//t_head_mje header;

		//char * mensajeHandShake =
				hacerHandShake_server(socket_co, SOY_NUCLEO);

		//liberar mensajeHandShake
		// Recibir Mensaje de consola.
		int* tamanio_mje = leerHeader(socket_co);
		char * mje_recibido = recibirMensaje_tamanio(socket_co, tamanio_mje);

		if(!strcmp("Se desconecto",mje_recibido)){
			log_debug(logger, "Se cerro la conexion");
			free((void *) mje_recibido);
			free(socket_desc);
	        close(socket_co);
	        exit(0);
		}
		log_debug(logger, "Mensaje recibido de consola %d : %s", socket_co, mje_recibido);



		free((void *) mje_recibido);
		free(socket_desc);
		return NULL;
}
