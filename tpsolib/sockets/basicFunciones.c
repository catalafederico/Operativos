//Sockets tuto: http://www.tyr.unlu.edu.ar/tyr/TYR-trab/satobigal/documentacion/beej/clientserver.html
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
#include <commons/log.h>

void crearSocket(int* socketacrear){
	int yes = 1;
	if((*socketacrear = socket(AF_INET,SOCK_STREAM,0)) == -1 ){
		perror("Error al crear el socket. Fin del programa.");
		exit(EXIT_FAILURE);
	}
	if(setsockopt(*socketacrear,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))){
        perror("Error en setsockopt, en basicFunciones.c");
        exit(1);
	}
}

void abrirPuerto(int socket, struct sockaddr_in* adress){
	if(bind(socket,(struct sockaddr *)adress, sizeof(struct sockaddr))==-1){
		perror("Error al setear el puerto al socket. Fin del programa.");
		exit(EXIT_FAILURE);
	}
}

void escucharConexiones(int socket, int colamax){
	if(listen(socket,colamax)==-1){
		perror("No fue posible empezar la escucha. Fin del programa");
		exit(EXIT_FAILURE);
	}
}

void aceptarConexion(int* socketnuevo, int socketescuchador,struct sockaddr_in* other_adress){
	int sin_size;
	sin_size = sizeof(struct sockaddr_in);
	if((*socketnuevo = accept(socketescuchador,(struct sockaddr *) other_adress,&sin_size)) == -1){
		perror("No se pudo establecer la conexion");
		exit(EXIT_FAILURE);
	}
}

void enviarMensaje(int socketDestino, char* mensaje){
    if (send(socketDestino, mensaje, strlen(mensaje), 0) == -1){
        perror("send");
    close(socketDestino);
    exit(0);
    }
}

char* recibirMensaje(int socketCliente){
//	PRIMERO SE RECIBE EL HEADER CON UN TAMANIO FIJO QUE ES TIPO t_head_mje
	int bytesRecibidos;
	char cantcaracteres[256];
	char* buf = malloc(sizeof(cantcaracteres));
	bytesRecibidos = recv(socketCliente,buf,sizeof(cantcaracteres)-1,0);
	if(bytesRecibidos<=0)
	{
		free(buf);
		return "Se desconecto";
	}
	else
	{
		return buf;
	}
}

void enviarStream(int socketDestino,int header, int tamanioMensaje, void* mensaje){
	send(socketDestino,header,sizeof(int),0);
	send(socketDestino,mensaje,tamanioMensaje,0);

}

void* recibirStream(int socketDondeRecibe, int tamanioEstructuraARecibir){
//	PRIMERO SE RECIBE EL HEADER CON UN TAMANIO FIJO QUE ES TIPO t_head_mje
	int bytesRecibidos;
	void* recibido = malloc(tamanioEstructuraARecibir);
	void* tempRcv;
	bytesRecibidos = recv(socketDondeRecibe,tempRcv,tamanioEstructuraARecibir,0);
	memcpy(recibido,tempRcv,tamanioEstructuraARecibir);
	if(bytesRecibidos<=0)
	{
		free(recibido);
		return NULL;
	}
	else
	{
		return recibido;
	}
}

char* recibirMensaje_tamanio(int socketCliente, int * long_mje){
	int bytesRecibidos;
	char* buf = (char *) malloc(long_mje+1);
	bytesRecibidos = recv(socketCliente,buf,long_mje,0);
	if(bytesRecibidos<=0)
	{
		buf = (char *) realloc(buf,strlen("Se desconecto"));
		strcpy(buf,"Se desconecto");
	}
	else
	{
		buf = (char *) realloc(buf, bytesRecibidos);
		*long_mje = bytesRecibidos;
	}
	return buf;
}

void conectarConDireccion(int* socketMio,struct sockaddr_in* direccionDestino){
	if (connect(*socketMio, (struct sockaddr*)direccionDestino, sizeof(struct sockaddr)) != 0) {
		perror("No se pudo conectar");
		exit(EXIT_FAILURE);
	}
}

int* leerHeader(int socketARecibir){
	int* header = malloc(sizeof(int));
	int bytesRecibidos = -1;
	bytesRecibidos = recv(socketARecibir,header,sizeof(int),0);
	if(bytesRecibidos<=0)
	{
		*header = -1;
		return header;
	}
	else{
		return header;
	}
}



