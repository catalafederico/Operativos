/*
 * nucleo.c
 *
 *  Created on: 24/4/2016
 *      Author: Explosive Code - Lucas Marino
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/config.h>

#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include "socketServer.h"
#include <commons/collections/list.h>


//#include <netinet/in.h>
//#include <sys/types.h>
//#include <signal.h>
//#include <unistd.h>


// Estructuras
typedef struct {
	int puerto_prog;
	int puerto_cpu;
	int quantum;
	int quantum_sleep;
	char ** io_id;
	int * io_sleep;
	char ** sem_id;
	int * sem_init;
	char ** shared_vars;
} t_reg_config;

struct server{
	int socketServer;
	struct sockaddr_in direccion;
	t_list* listaSockets;
};

// Funciones
t_reg_config get_config_params(void);


int main(int argc, char **argv) {
// Leo archivo de configuracion ------------------------------
	t_reg_config reg_config;
	reg_config = get_config_params();
	printf("parametro puerto prog %d \n", reg_config.puerto_prog);

// Creo socket para procesos (CONSOLA) ------------------------------
	int servidor=0;

	int ret_code=0;
	struct sockaddr_in nucleo_addr_proc;
	nucleo_addr_proc.sin_family = AF_INET;
	nucleo_addr_proc.sin_addr.s_addr = INADDR_ANY;
	nucleo_addr_proc.sin_port = htons(reg_config.puerto_prog);
	memset(&(nucleo_addr_proc.sin_zero),0, sizeof(nucleo_addr_proc));

	struct server serverNucleo;
	serverNucleo = crearServer(reg_config.puerto_prog);
	ponerServerEscuchaSelect(serverNucleo);
	enviarMensajeACliente("hola",(list_get(serverNucleo.listaSockets,1)));

	servidor = socket(AF_INET, SOCK_STREAM, 0);
	if (servidor == -1) {
	    perror("socket");
	    exit(1);
	}
	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));
	ret_code = bind(servidor, (void*) &nucleo_addr_proc, sizeof(nucleo_addr_proc));
	if (ret_code != 0) {
			perror("bind");
			exit(1);
	}
printf("estoy escuchando\n");
	ret_code = listen(servidor, SOMAXCONN);
	if (ret_code != 0) {
			perror("listen");
			exit(1);
	}



	struct sockaddr_in direccionCliente;
	unsigned int tamanioDireccion;

	int cliente = accept(servidor, (void*) &direccionCliente, &tamanioDireccion);

	printf("recibi conexion en el socket %d \n", cliente);
	if (cliente == -1) {
			perror("accept");
			exit(1);
	}

	ret_code = send(cliente, "Enhorabuena! Te conectaste!", 27, 0);
	if (ret_code == -1) {
			perror("send");
			exit(1);
	}


	return 0;

}



// get_config_params ---------------------------------------------------------
t_reg_config get_config_params(void){

	t_config * archivo_config = NULL;
	char * archivo_config_nombre = "archivo_configuracion.txt";
	t_reg_config reg_config;

	archivo_config = config_create(archivo_config_nombre);

	// 1 get PUERTO_PROG          --------------
	if (config_has_property(archivo_config,"PUERTO_PROG")){
		reg_config.puerto_prog = config_get_int_value(archivo_config,"PUERTO_PROG");
		printf("PUERTO_PROG= %d \n", reg_config.puerto_prog);

	}
	else{
			printf("no se encontro PUERTO_PROG \n");
	}

	// 2 get PUERTO_CPU
	if (config_has_property(archivo_config,"PUERTO_CPU")){
		reg_config.puerto_cpu = config_get_int_value(archivo_config,"PUERTO_CPU");
		printf("PUERTO_CPU= %d \n", reg_config.puerto_cpu);

	}
	else{
			printf("no se encontro PUERTO_CPU \n");
	}

	// 3 get QUANTUM
	if (config_has_property(archivo_config,"QUANTUM")){
		reg_config.quantum = config_get_int_value(archivo_config,"QUANTUM");
		printf("QUANTUM= %d \n", reg_config.quantum);

	}
	else{
			printf("no se encontro QUANTUM \n");
	}

	// 4 get QUANTUM_SLEEP
	if (config_has_property(archivo_config,"QUANTUM_SLEEP")){
		reg_config.quantum_sleep = config_get_int_value(archivo_config,"QUANTUM_SLEEP");
		printf("QUANTUM_SLEEP= %d \n", reg_config.quantum_sleep);

	}
	else{
			printf("no se encontro QUANTUM_SLEEP \n");
	}

	// 5 get IO_ID
	if (config_has_property(archivo_config,"IO_ID")){
		reg_config.io_id = config_get_array_value(archivo_config,"IO_ID");

		printf("IO_ID= %s \n", reg_config.io_id);

	}
	else{
			printf("no se encontro IO_ID \n");
	}

	// 6 get IO_SLEEP
	if (config_has_property(archivo_config,"IO_SLEEP")){
		reg_config.io_sleep = (int *) config_get_array_value(archivo_config,"IO_SLEEP");
		printf("IO_SLEEP= %d \n", reg_config.io_sleep);

	}
	else{
			printf("no se encontro IO_SLEEP \n");
	}

	// 7 get SEM_ID
	if (config_has_property(archivo_config,"SEM_ID")){
		reg_config.sem_id = config_get_array_value(archivo_config,"SEM_ID");
		printf("SEM_ID= %d \n", reg_config.sem_id);

	}
	else{
			printf("no se encontro SEM_ID \n");
	}

	// 8 get SEM_INIT
	if (config_has_property(archivo_config,"SEM_INIT")){
		reg_config.sem_init = (int *) config_get_array_value(archivo_config,"SEM_INIT");
		printf("SEM_INIT= %d \n", *reg_config.sem_init);

	}
	else{
			printf("no se encontro SEM_INIT \n");
	}

	// 9 get SHARED_VARS
	if (config_has_property(archivo_config,"SHARED_VARS")){
		reg_config.shared_vars = config_get_array_value(archivo_config,"SHARED_VARS");
		printf("SHARED_VARS= %d \n", reg_config.shared_vars);

	}
	else{
			printf("no se encontro SHARED_VARS \n");
	}

	config_destroy(archivo_config);
	return reg_config;
}
