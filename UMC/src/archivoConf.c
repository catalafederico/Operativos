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
#include "estructurasUMC.h"

void setearValorEntero(int* valorASetear,char* parametroABuscar);
void setearValorChar(char* valorASetear,char* parametroABuscar);

typedef struct {
	int PUERTO;
	char IP_SWAP[16];
	int PUERTO_SWAP;
	int MARCOS;
	int MARCO_SIZE;
	int MARCO_X_PROC;
	int ENTRADAS_TLB;
	int RETARDO;
} t_reg_config;

t_config * UMC_config = NULL;


t_reg_config* get_config_params(){

	char * UMC_config_path = "umc_config.cfg";
	UMC_config = config_create(UMC_config_path);
	int a = sizeof(int)*7+sizeof(char)*16;
	t_reg_config* puntero_configuracion = malloc(a);
	t_reg_config configuracion = *puntero_configuracion;

	setearValorEntero(&configuracion.PUERTO,"PUERTO");
	setearValorChar(configuracion.IP_SWAP,"IP_SWAP");
	setearValorEntero(&configuracion.PUERTO_SWAP,"PUERTO_SWAP");
	setearValorEntero(&configuracion.MARCOS,"MARCOS");
	setearValorEntero(&configuracion.MARCO_SIZE,"MARCO_SIZE");
	setearValorEntero(&configuracion.MARCO_X_PROC,"MARCO_X_PROC");
	setearValorEntero(&configuracion.ENTRADAS_TLB,"ENTRADAS_TLB");
	setearValorEntero(&configuracion.RETARDO,"RETARDO");

	config_destroy(UMC_config);
	return puntero_configuracion;
}

void setearValorEntero(int* valorASetear,char* parametroABuscar){
	if (config_has_property(UMC_config,parametroABuscar)){
		*valorASetear = config_get_int_value(UMC_config,parametroABuscar);
		printf("%s = %d \n",parametroABuscar, *valorASetear);
	}
	else{
		printf("No se encontro %s\n",parametroABuscar);
	}
}

void setearValorChar(char* valorASetear,char* parametroABuscar){
	if (config_has_property(UMC_config,parametroABuscar)){
		valorASetear = config_get_string_value(UMC_config,parametroABuscar);
		printf("%s = %s \n",parametroABuscar, valorASetear);
	}
	else{
		printf("No se encontro %s\n",parametroABuscar);
	}
}


