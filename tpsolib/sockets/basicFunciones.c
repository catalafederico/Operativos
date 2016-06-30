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
	unsigned int sin_size;
	sin_size = sizeof(struct sockaddr_in);
	if((*socketnuevo = accept(socketescuchador,(struct sockaddr *) other_adress, &sin_size)) == -1){
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

int enviarStream(int socketDestino,int header, int tamanioMensaje, void* mensaje){
	if(send(socketDestino,&header,sizeof(int),0)==-1){
		if(errno = EPIPE){
			return -987;
		}
		return -1;
	}
	if(send(socketDestino,mensaje,tamanioMensaje,0)==-1){
		if(errno = EPIPE){
			return -987;
		}
		return -1;
	}
	return 0;

}

void* recibirStream(int socketDondeRecibe, int tamanioEstructuraARecibir){
	int bytesRecibidos;
	void* recibido = malloc(tamanioEstructuraARecibir);
	void* tempRcv;
	int denuevo;
	do{
		denuevo = 0;
		tempRcv = malloc(tamanioEstructuraARecibir);
		bytesRecibidos = recv(socketDondeRecibe,tempRcv,tamanioEstructuraARecibir,0);
		if(bytesRecibidos==-1 && errno == EINTR)
		{
			denuevo = 1;
			free(tempRcv);
		}
	}while(denuevo);
	//errno = 0;
	if(bytesRecibidos == -1){
		perror("error en recibir stream");
		exit(-1);
	}
	memcpy(recibido,tempRcv,tamanioEstructuraARecibir);
	if(bytesRecibidos<=0)
	{
		free(tempRcv);
		free(recibido);
		return NULL;
	}
	else
	{
		free(tempRcv);
		return recibido;
	}
}

char* recibirMensaje_tamanio(int socketCliente, int * long_mje){
	int bytesRecibidos;
	char* buf = (char *) malloc(*long_mje+1);
	bytesRecibidos = recv(socketCliente,buf,*long_mje,0);
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

int conectarConDireccion(int* socketMio,struct sockaddr_in* direccionDestino){
	if (connect(*socketMio, (struct sockaddr*)direccionDestino, sizeof(struct sockaddr)) != 0) {
		//perror("No se pudo conectar");
		return -1;
	}
	return 0;
}

int* leerHeader(int socketARecibir){
	int* header;
	int bytesRecibidos = -1;
	int denuevo;
	do{
		denuevo = 0;
		header = malloc(sizeof(int));
		bytesRecibidos = recv(socketARecibir,header,sizeof(int),0);
		if(bytesRecibidos==-1 && errno == EINTR){
			printf("ERROR\n");
			denuevo = 1;
			free(header);
		}
	}while(denuevo);
	if(bytesRecibidos<=0)
	{
		*header = -1;
		return header;
	}
	else{
		return header;
	}
}



