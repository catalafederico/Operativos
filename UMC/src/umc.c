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
#include "socketServer.h"
#include <commons/collections/list.h>
#include "socketCliente.h"
/*
struct server{
	int socketServer;
	struct sockaddr_in direccion;
	t_list* listaSockets;
};*/



int main(void) {
	struct server serverUMC;
	serverUMC = crearServer(9000);
	ponerServerEscuchaSelect(serverUMC);
	//Esto deberia ir en otro hilo
	 struct cliente clienteUMC;
	 clienteUMC = crearCliente(9001,"127.0.0.1");
	 conectarConServidor(clienteUMC);
	 enviarMensaje(clienteUMC.socketCliente,"hola");
	return 0;
}
