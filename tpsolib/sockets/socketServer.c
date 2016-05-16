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
};

struct server crearServer(int puerto){

	struct server* serverProceso = malloc(sizeof(struct server));

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

/*
void atenderConexionNueva(struct server procesosServer){
	int nuevaConexion;
	struct sockaddr_in direccionEntrante;
	aceptarConexion(&nuevaConexion,procesosServer.socketServer, &direccionEntrante);
}*///Funcion reemplazada por atenderConexionNuevaSelect

int atenderConexionNuevaSelect(struct server procesosServer, int* maxfichero){

	int nuevaConexion;//socket donde va a estar nueva conexion
	struct sockaddr_in direccionEntrante;
	aceptarConexion(&nuevaConexion,procesosServer.socketServer, &direccionEntrante);
	// si es mayor sobreescribo
	if(nuevaConexion > *maxfichero){
		*maxfichero=nuevaConexion;
	}
	printf("Se ha conectado alguien\n");
	return nuevaConexion;
}

void ponerServerEscucha(struct server procesosServer){
	escucharConexiones(procesosServer.socketServer,SOMAXCONN);
	return;
}

void enviarMensajeACliente(char* mensaje, int socket){
	enviarMensaje(socket,mensaje);
}

void cerrarConexion(int socketACerrar, fd_set* descriptor){
	close(socketACerrar);
	FD_CLR(socketACerrar,descriptor);
	printf("Se cerro la conexion con %d\n",socketACerrar);
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
	printf("Server escuchando\n");
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
        				cerrarConexion(i,&descriptor_maestro);
        			}
        			else{
        				printf("%s\n",mensaje);
        				if(strcmp("cerrar",mensaje)){//compara si es diferente a cerrar, con cerrar cierra la conexion
        					//Acciones a realizar
        					//enviarMensaje(i,"Hola de nuevo");
        					//POR EJEMPLO SE PUEDE PONER UN SEND PARA RESPONER. ACORDARSE DE PONER EL RECIBE EN EL PROCESO
        					// DESTINO
        				}
        				else{
        					cerrarConexion(i,&descriptor_maestro);
        				}
        			}

        		}
        	}
        }
	}
}


char* hacerHandShake_server(int socketDestino, char * mensaje){

	// Enviar mensaje
	char * mje_retorno = malloc(strlen(mensaje));
    if (send(socketDestino, mje_retorno, strlen(mensaje), 0) == -1){
        perror("send");
        mje_retorno = (char *) realloc(mje_retorno,strlen("Error en el send"));
		strcpy(mje_retorno,"Error en el send");
        close(socketDestino);
    	return mje_retorno;
    }

	// Recibir Respuesta.
    int long_mje = strlen(mensaje);
    mje_retorno = recibirMensaje_tamanio(socketDestino,&long_mje);
	if(!strcmp("Se desconecto",mje_retorno)){
		perror("Se cerro la conexion");
		mje_retorno = (char *) realloc(mje_retorno,strlen("Se cerro la conexion"));
		strcpy(mje_retorno,"Se cerro la conexion");
        close(socketDestino);
    	return mje_retorno;
	}
	return mje_retorno;
}


