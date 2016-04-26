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
#define SERVERPORT 9999
#define SERVERCLIENTE 9998

int main(void) {
	struct server serverUMC;
	serverUMC = crearServer(SERVERPORT);
	ponerServerEscuchaSelect(serverUMC);
	//Esto deberia ir en otro hilo
	//Comentar la seccion de arriba y descomentar la de abajo para probar como cliente
	 /*struct cliente clienteUMC;
	 clienteUMC = crearCliente(SERVERCLIENTE,"127.0.0.1");
	 conectarConServidor(clienteUMC);
	 enviarMensaje(clienteUMC.socketCliente,"hola");*/
	return 0;
}
