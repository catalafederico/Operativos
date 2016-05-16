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
t_reg_config* config_UMC;
t_list* marcosLibres;
t_dictionary* tabla_actual;
t_dictionary* programas_ejecucion;
int tamanioMarcos;

void* inicializarMemoria(t_reg_config* configuracionUMC){
	config_UMC = configuracionUMC;
	marcosLibres = list_create();
	programas_ejecucion = dictionary_create();
	int i = 1;
	int cantidadDeMarcos = (*configuracionUMC).MARCOS;
	tamanioMarcos = (*configuracionUMC).MARCO_SIZE;
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


int desalojarPrograma(int id){
	t_dictionary* tabla_desalojar = dictionary_get(programas_ejecucion,(char *)&id);
	int cant_paginas = dictionary_size(tabla_desalojar);
	int i;
	for(i=1;i <= cant_paginas; i++){
		char* pag = &i;
		void* marcoLibre = dictionary_remove(tabla_desalojar,pag);
		list_add(marcoLibre,marcoLibre);
	}
	dictionary_destroy(tabla_desalojar);
	int idRemovido = dictionary_remove(programas_ejecucion,(char *)&id);
	return 0;
}


void* obtenerBytesMemoria(int pagina,int offset,int tamanio){
	void* obtenido = malloc(tamanio);
	int marco = dictionary_get(tabla_actual,(char*)pagina);
	int posicionDeMemoria = (marco*tamanioMarcos) + offset;
	memcpy(obtenido,(memoriaPrincipal + posicionDeMemoria),tamanio);
	return obtenido;
}

void almacenarBytes(int pagina, int offset, int tamanio, void* buffer){
	int marco = dictionary_get(tabla_actual,(char*)pagina);
	int posicionDeMemoria = (marco*tamanioMarcos) + offset;
	memcpy((memoriaPrincipal+posicionDeMemoria),buffer,tamanio);
}

void cambiarProceso(int idProceso){
	//Ak un mutex para q no cambie y acceda
	tabla_actual = dictionary_get(programas_ejecucion,(char*)&idProceso);
}
