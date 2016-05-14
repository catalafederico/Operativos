/*
 * umcServer.C
 *
 *  Created on: 9/5/2016
 *      Author: utnso
 */


#include <sockets/socketServer.h>
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


//Headers
#define CONSOLA 1
#define NUCLEO 2
#define CPU 3
#define UMC 4
#define SWAP 5
#define NUEVOPROGRAMA 50
#define FINALIZACIONPROGRAMA 51

umcNucleo *umcConfg;

void ponerUmcAEscuchar(struct server procesosServer,umcNucleo* umcTodo){

	umcConfg = umcTodo;
	//Atributo para no hacer join de los thread
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
        if (select(maxFichero+1, &descriptor_temporal, NULL, NULL, NULL) == -1) {
            perror("Error en select, Check it");
            exit(1);
        }
        	for(nroSocket=0;nroSocket <= maxFichero;nroSocket++){
        		if(FD_ISSET(nroSocket,&descriptor_temporal)){
        		if(nroSocket==procesosServer.socketServer){
        			int nuevaconexion = atenderConexionNuevaSelect(procesosServer,&maxFichero);
        			FD_SET(nuevaconexion,&descriptor_maestro);
        		}
        		else {
        			int* header = (int*)leerHeader(nroSocket);
        			switch (*header) {
						case CONSOLA:
							enviarMensaje(nroSocket,"Te intentaste conectar a UMC\0");
							break;
						case NUCLEO:
							enviarMensaje(nroSocket,"Te has conectado a UMC correctamente\0");
							atenderNucleo(noJoin,nroSocket);
							FD_CLR(nroSocket,&descriptor_maestro);
							break;
						case CPU:
							enviarMensaje(nroSocket,"Te has conectado a UMC correctamente\0");
							atenderCpu(noJoin, nroSocket);
							FD_CLR(nroSocket,&descriptor_maestro);
							break;
							//VA A RECIBIR MAS INFO DE CPU, HACER OTRO RCV
						case SWAP:
							enviarMensaje(nroSocket,"Te intentaste conectar a UMC\0");
							break;
						default:
							break;
					}

        		}
        	}
        }
	}
}




void atenderNucleo(pthread_attr_t atributo,int socketNucleo){
	pthread_t nucleo;
	tempStruct aMandar;
	aMandar.umcConfig = umcConfg;
	aMandar.socket = socketNucleo;
	pthread_create(&nucleo,&atributo,conexionNucleo,&aMandar);
}

void atenderCpu(pthread_attr_t atributo,int socketCPU){
	pthread_t cpu;
	tempStruct aMandar;
	aMandar.umcConfig = umcConfg;
	aMandar.socket = socketCPU;
	pthread_create(&cpu,&atributo,conexionNucleo,&aMandar);
}


