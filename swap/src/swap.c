/*
 ============================================================================
 Name        : swap.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/config.h>


// Estructuras
typedef struct {
	int PUERTO_ESCUCHA;
	int CANTIDAD_PAGINAS;
	int TAMANIO_PAGINA;
	int RETARDO_COMPACTACION;
} t_reg_config;

// Funciones
t_reg_config get_config_params(void);

int main(void) {

	//Archivo configuración

t_reg_config swap_config = get_config_params();

	//Conexion

	struct sockaddr_in direccionServidor;
		direccionServidor.sin_family = AF_INET;
		direccionServidor.sin_addr.s_addr = INADDR_ANY;
		direccionServidor.sin_port = htons(swap_config.PUERTO_ESCUCHA);

		int servidor = socket(AF_INET, SOCK_STREAM, 0);

		int activado = 1;
		setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

		if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
			perror("Falló el bind");
			return 1;
		}

		printf("Estoy escuchando\n");
		listen(servidor, 100);

		struct sockaddr_in direccionCliente;
		unsigned int tamanioDireccion;
		int cliente = accept(servidor, (void*) &direccionCliente, &tamanioDireccion);

		printf("Recibí una conexión en %d!!\n", cliente);
		send(cliente, "Hola!", 6, 0);

		return 0;
}

t_reg_config get_config_params(void){

	t_config * swap_config = NULL;
	char * swap_config_path = "swap_config.txt";
	t_reg_config reg_config;

	swap_config = config_create(swap_config_path);

	// 1 get PUERTO_ESCUCHA          --------------
	if (config_has_property(swap_config,"PUERTO_ESCUCHA")){
		reg_config.PUERTO_ESCUCHA = config_get_int_value(swap_config,"PUERTO_ESCUCHA");
		printf("PUERTO_ESCUCHA= %d \n", reg_config.PUERTO_ESCUCHA);

	}
	else{
			printf("No se encontro PUERTO_ESCUCHA \n");
	}

	// 2 get CANTIDAD_PAGINAS
	if (config_has_property(swap_config,"CANTIDAD_PAGINAS")){
		reg_config.CANTIDAD_PAGINAS = config_get_int_value(swap_config,"CANTIDAD_PAGINAS");
		printf("CANTIDAD_PAGINAS= %d \n", reg_config.CANTIDAD_PAGINAS);

	}
	else{
			printf("No se encontro CANTIDAD_PAGINAS \n");
	}

	// 3 get TAMAÑO_PAGINA
	if (config_has_property(swap_config,"TAMANIO_PAGINA")){
		reg_config.TAMANIO_PAGINA = config_get_int_value(swap_config,"TAMANIO_PAGINA");
		printf("TAMANIO_PAGINA= %d \n", reg_config.TAMANIO_PAGINA);

	}
	else{
			printf("No se encontro TAMANIO_PAGINA \n");
	}

	// 4 get RETARDO_COMPACTACION
	if (config_has_property(swap_config,"RETARDO_COMPACTACION")){
		reg_config.RETARDO_COMPACTACION = config_get_int_value(swap_config,"RETARDO_COMPACTACION");
		printf("RETARDO_COMPACTACION= %d \n", reg_config.RETARDO_COMPACTACION);

	}
	else{
			printf("No se encontro RETARDO_COMPACTACION \n");
	}

	config_destroy(swap_config);
	return reg_config;
}
