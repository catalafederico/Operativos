/*
 * configConsola.c

 *
 *  Created on: 1/7/2016
 *      Author: utnso
 */


#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include "estructurasConsola.h"
void setearValorEntero(int* valorASetear,char* parametroABuscar);
void setearValorChar(char** valorASetear,char* parametroABuscar);

t_config * CPU_config = NULL;
t_reg_config get_config_params(){

	char * consola_config_path = "../configConsola.cfg";
	CPU_config = config_create(consola_config_path);
	int a = sizeof(int)+sizeof(char*);
	t_reg_config* puntero_configuracion = malloc(a);
	t_reg_config configuracion = *puntero_configuracion;
	setearValorChar(&configuracion.IP,"IP");
	setearValorEntero(&configuracion.PUERTO,"PUERTO");
	config_destroy(CPU_config);
		return configuracion;

}
void setearValorEntero(int* valorASetear,char* parametroABuscar){
	if (config_has_property(CPU_config,parametroABuscar)){
		*valorASetear = config_get_int_value(CPU_config,parametroABuscar);
		printf("%s = %d \n",parametroABuscar, *valorASetear);
	}
	else{
		printf("No se encontro %s\n",parametroABuscar);
	}
}

void setearValorChar(char** valorASetear,char* parametroABuscar){
	if (config_has_property(CPU_config,parametroABuscar)){
		*valorASetear = strdup(config_get_string_value(CPU_config,parametroABuscar));
		printf("%s = %s \n",parametroABuscar, *valorASetear);
	}
	else{
		printf("No se encontro %s\n",parametroABuscar);
	}
}
