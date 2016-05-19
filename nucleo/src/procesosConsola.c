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
//------------------------------------------------------------------------------------------
// ---------------------------------- atender_conexion_consolas  ---------------------------
void *atender_conexion_consolas(void *socket_desc){

	int nuevaConexion, *socket_nuevo; //socket donde va a estar nueva conexion
	struct sockaddr_in direccionEntrante;
	int socket_local = (int)socket_desc;
	aceptarConexion(&nuevaConexion, socket_local, &direccionEntrante);
	while(nuevaConexion){

		log_debug(logger, "Se ha conectado una Consola");

		pthread_attr_t attr;
		pthread_t thread_consola_con;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		socket_nuevo = malloc(sizeof(int));
		*socket_nuevo = nuevaConexion;
		if( pthread_create( &thread_consola_con , NULL /*&attr*/ , atender_consola, (void*) socket_nuevo) < 0)
		{
			log_debug(logger, "No fue posible crear thread p/ consolas");
			exit(EXIT_FAILURE);
		}
		pthread_attr_destroy(&attr);

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
// ---------------------------------- atender_consola---------------------------------------
void *atender_consola(void *socket_desc){
	  //Get the socket descriptor
		int socket_co = *(int*)socket_desc;
//		int read_size;
//		t_head_mje header;

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

