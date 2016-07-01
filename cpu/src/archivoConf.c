/*
 * archivoConf.c
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#include <commons/config.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include "estructurasCPU.h"

void setearValorEntero(int* valorASetear,char* parametroABuscar);
void setearValorChar(char** valorASetear,char* parametroABuscar);

t_config * CPU_config = NULL;


t_reg_config get_config_params(){

	char * Cpu_config_path = "../cpu_config.cfg";
	CPU_config = config_create(Cpu_config_path);
	t_reg_config configuracion;
	setearValorEntero(&configuracion.puertoUMC,"PUMC");
	setearValorEntero(&configuracion.puertoNucleo,"PNUCLEO");
	setearValorChar(&configuracion.IPUMC,"IPUMC");
	setearValorChar(&configuracion.IPNucleo,"IPNUCLEO");
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


