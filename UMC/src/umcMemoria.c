/*
 * umcMemoria.c
 *
 *  Created on: 9/5/2016
 *      Author: utnso
 */
#include "archivoConf.h"
#include <stdio.h>
#include <stdlib.h>



void* inicializarMemoria(t_reg_config* configuracionUMC){
	int cantidadDeMarcos = (*configuracionUMC).MARCOS;
	int tamanioMarcos = (*configuracionUMC).MARCO_SIZE;
	void* memoriaPrincipal = calloc(cantidadDeMarcos, tamanioMarcos);
	return memoriaPrincipal;
}

