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
#include "basicFunciones.h"

struct cliente{
	int socketCliente;
	int socketServer;
	struct sockaddr_in direccionDestino;
};

struct cliente crearCliente(int puerto,char* ip){
	struct cliente* procesoCliente = malloc(sizeof(struct cliente));
	crearSocket(&((*procesoCliente).socketCliente));

	(*procesoCliente).direccionDestino.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	(*procesoCliente).direccionDestino.sin_port = htons(puerto);  // short, Ordenación de bytes de la red
	if(((*procesoCliente).direccionDestino.sin_addr.s_addr = inet_addr(ip))==INADDR_NONE){
		printf("Ip mal pasada, chekearla\n");
		exit(1);
	}
    memset(&((*procesoCliente).direccionDestino.sin_zero),'\0', 8);  // poner a cero el resto de la estructura

	return *procesoCliente;
}


void conectarConServidor(struct cliente procesoCliente){
	conectarConDireccion(&(procesoCliente.socketCliente),&(procesoCliente.direccionDestino));
}

void enviarMensajeServidor(int servidorDestino,char* mensaje){
	enviarMensaje(servidorDestino,mensaje);
}

char* esperarRespuestaServidor(int socketServidor){
	return recibirMensaje(socketServidor);
}
char* chatConProceso(int socketProceso, char* mensaje){
	enviarMensaje(socketProceso,mensaje);
	return esperarRespuestaServidor(socketProceso);
}

char* hacerHandShake_cliente(int socketDestino,char* mensaje){
	char * mje_retorno = malloc(strlen(mensaje));

	// Recibir Saludo.
	int long_mje = strlen(mensaje);
	mje_retorno = recibirMensaje_tamanio(socketDestino,&long_mje);
	if(!strcmp("Se desconecto",mje_retorno)){
		perror("Se cerro la conexion");
		mje_retorno = (char *) realloc(mje_retorno,strlen("Se cerro la conexion"));
		strcpy(mje_retorno,"Se cerro la conexion");
		close(socketDestino);
		return mje_retorno;
	}
	// Enviar mensaje
    if (send(socketDestino, mensaje, strlen(mensaje), 0) == -1){
        perror("send");
        mje_retorno = (char *) realloc(mje_retorno,strlen("Error en el send"));
		strcpy(mje_retorno,"Error en el send");
        close(socketDestino);
    	return mje_retorno;
    }
	return mje_retorno;

}

