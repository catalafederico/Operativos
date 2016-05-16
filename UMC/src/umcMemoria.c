/*
 * umcMemoria.c
 *
 *  Created on: 9/5/2016
 *      Author: utnso
 */
#include "archivoConf.h"
#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include "estructurasUMC.h"

void* memoriaPrincipal;
t_list* marcosLibres;
t_dictionary* tabla_actual;
t_dictionary* programas_ejecucion;

void* inicializarMemoria(t_reg_config* configuracionUMC){
	marcosLibres = list_create();
	programas_ejecucion = dictionary_create();
	int i = 1;
	int cantidadDeMarcos = (*configuracionUMC).MARCOS;
	int tamanioMarcos = (*configuracionUMC).MARCO_SIZE;
	void* memoriaPrincipal = calloc(cantidadDeMarcos, tamanioMarcos);
	for(i = 1;i<=cantidadDeMarcos;i++){
		int* tempMarcoNro = malloc(sizeof(int));
		*tempMarcoNro = i;
		list_add(marcosLibres,tempMarcoNro);
	}
	return memoriaPrincipal;
}


int alocarPrograma(int paginasRequeridas, int id_proceso){
	if(list_size(memoriaPrincipal) < paginasRequeridas){
		return -1;
	}
	else{
		int i = 1;
		t_dictionary* pag_frame = dictionary_create();
		for(i = 1;i<=paginasRequeridas;i++){
			char* pagina = malloc(sizeof(int));
			*pagina = i;
			//obtengo un marco y lo saco
			void* nroMarco = list_remove(memoriaPrincipal,0);
			//asigno marco a la pagina, DICCIONARIO YA DEBE ESTAR CREADO
			dictionary_put(pag_frame,pagina,nroMarco);
		}
		dictionary_put(programas_ejecucion,(char *) &id_proceso,pag_frame);
		tabla_actual = pag_frame;
		return 0;
	}
}


int desalojarPrograma(proceso* proceso_desalojar){
	int cant_paginas = dictionary_size(proceso_desalojar->pag_marco);
	int i;
	for(i=1;i <= 1; i++){
		char* pag = &i;
		void* marcoLibre = dictionary_remove(proceso_desalojar->pag_marco,pag);
		list_add(marcoLibre,marcoLibre);
	}
	return 0;
}


void* obtenerBytes(){

}

void cambiarProceso(){

}
