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
#include "umcCliente.h"
#include <commons/log.h>

void sockets();


int main(void) {
	umcNucleo umcConfg;
	umcConfg.loguer = log_create("logUMC.txt","UMC", false,LOG_LEVEL_INFO);

	log_info(umcConfg.loguer, "Cargando parametros");
	umcConfg.configuracionUMC = *get_config_params();
	log_info(umcConfg.loguer, "Cargado parametros");

	log_info(umcConfg.loguer, "Inicializando memoria");
	umcConfg.memoriaPrincipal = inicializarMemoria(&(umcConfg.configuracionUMC));
	log_info(umcConfg.loguer, "Memoria Inicializada");

	struct cliente aSwap;
	aSwap = crearCliente(umcConfg.configuracionUMC.PUERTO_SWAP,"127.0.0.1");
	log_info(umcConfg.loguer, "Conectando a Swap Puerto: %d Direc: %s",umcConfg.configuracionUMC.PUERTO_SWAP,umcConfg.configuracionUMC.IP_SWAP);
	conectarConServidor(aSwap);
	log_info(umcConfg.loguer, "Conectado a Swap socket: %d", aSwap.socketCliente);
	inicializarSwap(&umcConfg.socketSwap);

	umcConfg.socketSwap = aSwap.socketCliente;

	pthread_t consola;
	pthread_t socket;

	log_info(umcConfg.loguer, "Creando proceso consola");
	pthread_create(&consola,NULL,(void*)consolaUMC,NULL);
	log_info(umcConfg.loguer, "Proceso consola creado");

	log_info(umcConfg.loguer, "Creando proceso umcServer");
	pthread_create(&socket,NULL,(void*)sockets,&umcConfg);
	log_info(umcConfg.loguer, "Proceso Creado");

	pthread_join(consola,NULL);
	return 0;
}

void sockets(umcNucleo* umcConfg){
	struct server serverUMC;
	serverUMC = crearServer(umcConfg->configuracionUMC.PUERTO);
	ponerUmcAEscuchar(serverUMC,umcConfg);
}



