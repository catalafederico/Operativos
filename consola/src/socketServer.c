/*
 * socketServer.c
 *
 *  Created on: 25/4/2016
 *      Author: utnso
 */
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
#include "basicFunciones.h"
#include <commons/collections/list.h>

struct server{
	int socketServer;
	struct sockaddr_in direccion;
	t_list* listaSockets;
};

struct server crearServer(int puerto){

	struct server* serverProceso = malloc(sizeof(struct server));

	//Creacion de estructura contenedora de los sockets q se conectan
	(*serverProceso).listaSockets = list_create();

	//Creacion del socket server
	crearSocket(&((*serverProceso).socketServer));

	//Numero del puerto donde va a escucharse nuevas conexiones
	int puertoServer = puerto;

	//Info de la direccion servidor UMC
	(*serverProceso).direccion.sin_family = AF_INET;         		// Ordenación de bytes de la máquina
	(*serverProceso).direccion.sin_port = htons(puertoServer);     	// short, Ordenación de bytes de la red
	(*serverProceso).direccion.sin_addr.s_addr = INADDR_ANY; 		// Rellenar con mi dirección IP
    memset(&((*serverProceso).direccion.sin_zero), '\0', 8); 		// Poner a cero el resto de la estructura

    //Abro puerto para nuevas conexiones
    abrirPuerto((*serverProceso).socketServer,&(*serverProceso).direccion);


	return (*serverProceso);
}

void atenderConexionNueva(struct server procesosServer){

	int nuevaConexion;
	struct sockaddr_in direccionEntrante;
	aceptarConexion(&nuevaConexion,procesosServer.socketServer, &direccionEntrante);

	//Agrega socket a la lista, seguro despues se haga diccionario con el handshake
	list_add(procesosServer.listaSockets,(int *)&nuevaConexion);
	return;
}

int atenderConexionNuevaSelect(struct server procesosServer, int* maxfichero){

	int nuevaConexion;//socket donde va a estar nueva conexion
	struct sockaddr_in direccionEntrante;
	aceptarConexion(&nuevaConexion,procesosServer.socketServer, &direccionEntrante);
	// si es mayor sobreescribo
	if(nuevaConexion > *maxfichero){
		*maxfichero=nuevaConexion;
	}
	printf("Se ha conectado alguien\n");
	//Agrega socket a la lista, seguro despues se haga diccionario con el handshake
	list_add(procesosServer.listaSockets,(int *)&nuevaConexion);
	return nuevaConexion;
}

void ponerServerEscucha(struct server procesosServer){
	escucharConexiones(procesosServer.socketServer,10);
	return;
}

void enviarMensajeACliente(char* mensaje, int socket){
	enviarMensaje(socket,mensaje);
}

void ponerServerEscuchaSelect(struct server procesosServer){

	//Tuto Select: http://www.tyr.unlu.edu.ar/tyr/TYR-trab/satobigal/documentacion/beej/advanced.html
	int i=0;
	fd_set descriptor_maestro;
	fd_set descriptor_temporal;
	int maxFichero;
	FD_ZERO(&descriptor_maestro);
	FD_ZERO(&descriptor_temporal);
	ponerServerEscucha(procesosServer);
	FD_SET(procesosServer.socketServer,&descriptor_maestro);
	maxFichero = procesosServer.socketServer;
	while(1){
		descriptor_temporal = descriptor_maestro;
        if (select(maxFichero+1, &descriptor_temporal, NULL, NULL, NULL) == -1) {
            perror("Error en select, Check it");
            exit(1);
        }
        	for(i=0;i <= maxFichero;i++){
        		if(FD_ISSET(i,&descriptor_temporal)){
        		if(i==procesosServer.socketServer){
        			int nuevaconexion = atenderConexionNuevaSelect(procesosServer,&maxFichero);
        			FD_SET(nuevaconexion,&descriptor_maestro);
        		}
        		else {
        			char* mensaje = recibirMensaje(i);
        			//si se cerro la conexion
        			if(!strcmp("Se desconecto",mensaje)){
        				close(i);
        			}
        			else{
        				printf("%s\n",mensaje);
        				close(i);
        			}
        			FD_CLR(i,&descriptor_maestro);
        		}
        	}
        }
	}
}

