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

void ponerServerEscucha(struct server procesosServer){
	escucharConexiones(procesosServer.socketServer,1);
	atenderConexionNueva(procesosServer);
	return;
}

void enviarMensajeACliente(char* mensaje, int socket){
	enviarMensaje(socket,mensaje);
}

