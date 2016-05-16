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
#include <commons/collections/list.h>
#include <sockets/socketCliente.h>
#include "umcServer.h"
#include <sockets/socketServer.h>
#include "umcConsola.h"
#include "archivoConf.h"
#include "umcMemoria.h"
#include "estructurasUMC.h"

#define SERVERPORT 9999
#define SERVERCLIENTE 9998


void sockets();


int main(void) {
	umcNucleo umcConfg;
	umcConfg.configuracionUMC = *get_config_params();
	umcConfg.memoriaPrincipal = inicializarMemoria(&(umcConfg.configuracionUMC));

	struct cliente aSwap;
	aSwap = crearCliente(6000,"127.0.0.1");
	conectarConServidor(aSwap);
	umcConfg.socketSwap = aSwap.socketCliente;

	pthread_t consola;
	pthread_t socket;

	pthread_create(&consola,NULL,(void*)consolaUMC,NULL);
	pthread_create(&socket,NULL,(void*)sockets,&umcConfg);

	pthread_join(consola,NULL);
	return 0;
}

void sockets(umcNucleo* umcConfg){
	struct server serverUMC;
	serverUMC = crearServer(umcConfg->configuracionUMC.PUERTO);
	ponerUmcAEscuchar(&serverUMC,umcConfg);
}



