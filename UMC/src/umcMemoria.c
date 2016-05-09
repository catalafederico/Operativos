/*
 * umcMemoria.c
 *
 *  Created on: 9/5/2016
 *      Author: utnso
 */
#include "archivoConf.h"
#include <stdio.h>
#include <stdlib.h>

t_reg_config* configuracionUMC;
void* memoriaPrincipal; //Acordarse de hacer free


void inicializarMemoria(){
	int cantidadDeMarcos = (*configuracionUMC).MARCOS;
	int tamanioMarcos = (*configuracionUMC).MARCO_SIZE;
	memoriaPrincipal = calloc(cantidadDeMarcos, tamanioMarcos);
}

