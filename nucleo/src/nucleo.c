/*
 * nucleo.c
 *
 *  Created on: 24/4/2016
 *      Author: Explosive Code - Lucas Marino
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <commons/config.h>
//#include <netinet/in.h>
//#include <sys/types.h>
//#include <signal.h>
//#include <unistd.h>

// Estructuras
//struct reg_configuracion {
//	char campo[15];
//	void * resto;
//};


int main(int argc, char **argv) {
// Leo archivo de configuracion ------------------------------
	t_config * archivo_config = NULL;
	char * archivo_config_nombre = "archivo_configuracion.txt";
//	struct reg_configuracion linea_config;
	int eofile=0;
	int puerto_prog, puerto_cpu, quantum, quantum_sleep;
	char ** io_id;
	char ** sem_id;
	char ** shared_vars;
	int * io_sleep;
	int * sem_init;
	void * restolocal;

	archivo_config = config_create(archivo_config_nombre);

	if (config_has_property(archivo_config,"PUERTO_PROG")){
		puerto_prog = config_get_int_value(archivo_config,"PUERTO_PROG");
		printf("PUERTO_PROG= %d \n", puerto_prog);

	}
	else{
			printf("no se encontro PUERTO_PROG \n");
			return -1;

	}

	config_destroy(archivo_config);


	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(8080);
//	while (TRUE)


//	fprintf(miarchivo, "mensaje de prueba1 \n");
//	fprintf(miarchivo, "mensaje de prueba2 \n");

	return 0;
}

