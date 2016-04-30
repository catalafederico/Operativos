#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
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
#include "umcConsola.h"

#include "archivoConf.h"
#define SERVERPORT 9999
#define SERVERCLIENTE 9998

void sockets();
void inicializacionConsola();
t_reg_config configuracionUMC;



int main(void) {

	inicializacionConsola();
	sleep(5);

	pthread_t consola;
	pthread_t socket;

	pthread_create(&consola,NULL,(void*)consolaUMC,NULL);
	pthread_create(&socket,NULL,(void*)sockets,NULL);

	pthread_join()in(consola,NULL);
	pthread_join(socket,NULL);
	return 0;
}

void sockets(){
	struct server serverUMC;
	serverUMC = crearServer(configuracionUMC.PUERTO);
	ponerServerEscuchaSelect(serverUMC);
	//Esto deberia ir en otro hilo
	//Comentar la seccion de arriba y descomentar la de abajo para probar como cliente
}

void inicializacionConsola(){
	printf("Leyendo parametros de configuraion\n");
	configuracionUMC = get_config_params();
	printf("Parametros de configuraion leidos\n");
	printf("Inicializando consola\n");
	printf("Consola inicializada\n");
}
