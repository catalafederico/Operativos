/*
 * archivoConfig.c
 *
 *  Created on: 18/6/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sockets/socketServer.h>
#include <sockets/basicFunciones.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "swapEstrucuturas.h"

extern t_reg_config swap_configuracion;
//---------Funcion para extraer los datos del archivo de configuracion
void get_config_params(void) {

	t_config * swap_config = NULL;
	char * swap_config_path = "swap_config.cfg";

	swap_config = config_create(swap_config_path);

	// 1 get PUERTO_ESCUCHA          --------------
	if (config_has_property(swap_config, "PUERTO_ESCUCHA")) {
		swap_configuracion.PUERTO_ESCUCHA = config_get_int_value(swap_config,
				"PUERTO_ESCUCHA");
		printf("PUERTO_ESCUCHA= %d \n", swap_configuracion.PUERTO_ESCUCHA);

	} else {
		printf("No se encontro PUERTO_ESCUCHA \n");
	}

	// 2 get NOMBRE ARCHIVO         --------------
	if (config_has_property(swap_config, "NOMBRE_SWAP")) {
		char *temp = config_get_string_value(swap_config,
				"NOMBRE_SWAP");
		swap_configuracion.NOMBRE_SWAP = strdup(temp);
		printf("NOMBRE_SWAP= %s \n", swap_configuracion.NOMBRE_SWAP);

	} else {
		printf("No se encontro PUERTO_ESCUCHA \n");
	}

	// 3 get CANTIDAD_PAGINAS
	if (config_has_property(swap_config, "CANTIDAD_PAGINAS")) {
		swap_configuracion.CANTIDAD_PAGINAS = config_get_int_value(swap_config,
				"CANTIDAD_PAGINAS");
		printf("CANTIDAD_PAGINAS= %d \n", swap_configuracion.CANTIDAD_PAGINAS);

	} else {
		printf("No se encontro CANTIDAD_PAGINAS \n");
	}

	// 4 get TAMAÑO_PAGINA
	if (config_has_property(swap_config, "TAMANIO_PAGINA")) {
		swap_configuracion.TAMANIO_PAGINA = config_get_int_value(swap_config,
				"TAMANIO_PAGINA");
		printf("TAMANIO_PAGINA= %d \n", swap_configuracion.TAMANIO_PAGINA);

	} else {
		printf("No se encontro TAMANIO_PAGINA \n");
	}

	// 5 get RETARDO_COMPACTACION
	if (config_has_property(swap_config, "RETARDO_COMPACTACION")) {
		swap_configuracion.RETARDO_COMPACTACION = config_get_int_value(swap_config,
				"RETARDO_COMPACTACION");
		printf("RETARDO_COMPACTACION= %d \n", swap_configuracion.RETARDO_COMPACTACION);

	} else {
		printf("No se encontro RETARDO_COMPACTACION \n");
	}

	config_destroy(swap_config);
}
