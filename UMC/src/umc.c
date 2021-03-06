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
umcNucleo umcConfg;
void sockets();
t_log* logConexiones;



int main(void) {
//	tlb tablaPag[umcConfg.configuracionUMC.ENTRADAS_TLB]; creo la tabla para poder manejarla con lru , falta iniciar los amrcos y paginas

	umcConfg.loguer = log_create("../logs/logUMC.txt","UMC", false,LOG_LEVEL_INFO);
	logConexiones = log_create("../logs/conexiones.txt","UMC",false,LOG_LEVEL_TRACE);
	//umcConfg.loguer = log_create("../logs/logUMC.txt","UMC", false,LOG_LEVEL_INFO);
	//logConexiones = log_create("../logs/conexiones.txt","UMC",false,LOG_LEVEL_TRACE);


	log_info(umcConfg.loguer, "Cargando parametros");
	umcConfg.configuracionUMC = get_config_params();
	log_info(umcConfg.loguer, "Cargado parametros");

	log_info(umcConfg.loguer, "Inicializando memoria");
	umcConfg.memoriaPrincipal = inicializarMemoria(&(umcConfg.configuracionUMC));
	log_info(umcConfg.loguer, "Memoria Inicializada");

	struct cliente aSwap;
	aSwap = crearCliente(umcConfg.configuracionUMC.PUERTO_SWAP,umcConfg.configuracionUMC.IP_SWAP);
	log_info(umcConfg.loguer, "Conectando a Swap Puerto: %d Direc: %s",umcConfg.configuracionUMC.PUERTO_SWAP,umcConfg.configuracionUMC.IP_SWAP);
	int a = 1;
	while(conectarConServidor(aSwap)==-1)
	{
		printf("UMC: No se pudo conectar con swap reintentando de 5 segundos, intento nro: %d\n", a );
		sleep(5);
		a++;
	}
	log_info(umcConfg.loguer, "Conectado a Swap socket: %d", aSwap.socketCliente);
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

void sockets(){
	struct server serverUMC;
	serverUMC = crearServer(umcConfg.configuracionUMC.PUERTO);
	ponerUmcAEscuchar(serverUMC);
}



