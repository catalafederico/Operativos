/*
 * umcServer.C
 *
 *  Created on: 9/5/2016
 *      Author: utnso
 */

#include <sockets/header.h>
//#include <sockets/socketServer.h>
//#include <sockets/basicFunciones.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include "umcNucleo.h"
#include <commons/config.h>
#include "archivoConf.h"
#include "umcCpu.h"

extern umcNucleo umcConfg;

struct server{
	int socketServer;
	struct sockaddr_in direccion;
	t_list* listaSockets;
};

void atenderNucleo(pthread_attr_t atributo,int socketNucleo);
void atenderCpu(pthread_attr_t atributo,int socketCPU);
int atenderConexionNuevaSelect(struct server procesosServer, int* maxfichero);
void enviarConfirmacion(int socket);
extern t_log* logConexiones;

void ponerUmcAEscuchar(struct server procesosServer){

	//Atributo para no hacer join de los thread
	log_trace(logConexiones,"Empiesa la escucha de nucleo y cpu.");
	pthread_attr_t noJoin;
	pthread_attr_init(&noJoin);
	pthread_attr_setdetachstate(&noJoin,PTHREAD_CREATE_DETACHED);

	//Tuto Select: http://www.tyr.unlu.edu.ar/tyr/TYR-trab/satobigal/documentacion/beej/advanced.html
	int nroSocket=0;
	fd_set descriptor_maestro;
	fd_set descriptor_temporal;
	int maxFichero;
	FD_ZERO(&descriptor_maestro);
	FD_ZERO(&descriptor_temporal);
	ponerServerEscucha(procesosServer);
	printf("Server escuchando\n");
	FD_SET(procesosServer.socketServer,&descriptor_maestro);
	maxFichero = procesosServer.socketServer;
	while(1){
		descriptor_temporal = descriptor_maestro;
		log_info(umcConfg.loguer, "Umc escuchando");
        if (select(maxFichero+1, &descriptor_temporal, NULL, NULL, NULL) == -1) {
            perror("Error en select, Check it");
            exit(1);
        }
    	log_trace(logConexiones,"Se escucho conexion.");
        	for(nroSocket=0;nroSocket <= maxFichero;nroSocket++){
        		if(FD_ISSET(nroSocket,&descriptor_temporal)){
        		if(nroSocket==procesosServer.socketServer){
        			int* tempMF = &maxFichero;
        			int nuevaconexion = atenderConexionNuevaSelect(procesosServer,tempMF);
        			FD_SET(nuevaconexion,&descriptor_maestro);
        			log_info(logConexiones, "Se conecto alguien.");
        		}
        		else {
        			int* header = (int*)leerHeader(nroSocket);
        			switch (*header) {
						case CONSOLA:
							enviarError(nroSocket);
							break;
						case NUCLEO:
							log_trace(logConexiones,"Empiesa la handshake nucleo.");
							atenderNucleo(noJoin,nroSocket);
							enviarConfirmacion(nroSocket);
							log_trace(logConexiones,"Termina la handshake nucleo.");
							break;
						case CPU:
							log_trace(logConexiones,"Empiesa la handshake cpu.");
							atenderCpu(noJoin, nroSocket);
							enviarConfirmacion(nroSocket);
							log_trace(logConexiones,"Termina la handshake cpu.");
							break;
							//VA A RECIBIR MAS INFO DE CPU, HACER OTRO RCV
						case SWAP:
							enviarError(nroSocket);
							break;
						default:
							break;
					}
        			FD_CLR(nroSocket,&descriptor_maestro);

        		}
        	}
        }
	}
}




void atenderNucleo(pthread_attr_t atributo,int socketNucleo){
	pthread_t nucleo;
	printf("Se ha conectado NUCLEO\n");
	log_info(umcConfg.loguer, "Creando proceso nucleo.");
	pthread_create(&nucleo,&atributo,conexionNucleo,socketNucleo);
	log_info(umcConfg.loguer, "Creado proceso nucleo.");
}

void atenderCpu(pthread_attr_t atributo,int socketCPU){
	pthread_t cpu;
	printf("Se ha conectado un  CPU\n");
	log_info(umcConfg.loguer, "Creando proceso cpu.");
	pthread_create(&cpu,&atributo,conexionCpu,socketCPU);
	log_info(umcConfg.loguer, "Creano proceso cpu.");
}

int atenderConexionNuevaSelect(struct server procesosServer, int* maxfichero){

	int nuevaConexion;//socket donde va a estar nueva conexion
	struct sockaddr_in direccionEntrante;
	aceptarConexion(&nuevaConexion,procesosServer.socketServer, &direccionEntrante);
	// si es mayor sobreescribo
	if(nuevaConexion > *maxfichero){
		*maxfichero=nuevaConexion;
	}
	return nuevaConexion;
}

void enviarConfirmacion(int socket){
	int ok = 6;
	if(send(socket,&ok,sizeof(int),0)==-1){
		perror("no anda:\0");
	}
}

void enviarError(int socket){
	int error = 7;
	if(send(socket,&error,sizeof(int),0)==-1){
		perror("no anda:\0");
	}
}

