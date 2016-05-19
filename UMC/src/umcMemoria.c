/*
 * umcMemoria.c
 *
 *  Created on: 9/5/2016
 *      Author: utnso
 */
#include "archivoConf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include "estructurasUMC.h"
#include "umcCliente.h"
#include <pthread.h>

void* memoriaPrincipal;
extern umcNucleo umcConfg;

//Menajo de la memoria
t_list* marcosLibres;
t_dictionary* tabla_actual;
int* idProcesoActual;
t_dictionary* programas_ejecucion;
pthread_mutex_t* semaforoMemoria;


void* inicializarMemoria(t_reg_config* configuracionUMC){
	//pthread_mutex_init(semaforoMemoria,NULL);
	marcosLibres = list_create();
	programas_ejecucion = dictionary_create();
	idProcesoActual = malloc(sizeof(int));
	int i = 1;
	int cantidadDeMarcos = (*configuracionUMC).MARCOS;
	memoriaPrincipal = calloc(cantidadDeMarcos, umcConfg.configuracionUMC.MARCO_SIZE);
	for(i = 0;i<cantidadDeMarcos;i++){
		int* tempMarcoNro = malloc(sizeof(int));
		*tempMarcoNro = i;
		list_add(marcosLibres,tempMarcoNro);
	}
	return memoriaPrincipal;
}


int alocarPrograma(int paginasRequeridas, int id_proceso){
	//Esto va para la entrega 2
	/*notificarASwapPrograma(paginasRequeridas,id_proceso);
	*idProcesoActual = id_proceso;
	return 0;*/
	//Esto es para la entrega 3
	if(list_size(marcosLibres) < paginasRequeridas  && paginasRequeridas < umcConfg.configuracionUMC.MARCO_X_PROC){
		return -1;
	}
	else{
		int* idProceso = malloc(sizeof(int));
		*idProceso = id_proceso;
		int i = 1;
		t_dictionary* pag_frame = dictionary_create();
		//pthread_mutex_lock(semaforoMemoria);
		for(i = 0;i<paginasRequeridas;i++){
			int* pagina = malloc(sizeof(int));
			*pagina = i;
			//obtengo un marco y lo saco
			int* nroMarco = list_remove(marcosLibres,0);
			//asigno marco a la pagina, DICCIONARIO YA DEBE ESTAR CREADO
			dictionary_put(pag_frame,pagina,nroMarco);
		}
		tabla_actual = pag_frame;
		//pthread_mutex_unlock(semaforoMemoria);
		dictionary_put(programas_ejecucion,idProceso,pag_frame);

		return 0;
	}

}


int desalojarPrograma(int id){

	//Esto es de la entrega 2
	/*notificarASwapFinPrograma(id);
	return 0;*/

	//Esto es mas de la entrega 3
	t_dictionary* tabla_desalojar = dictionary_get(programas_ejecucion,&id);
	int cant_paginas = dictionary_size(tabla_desalojar);
	int i;
	//pthread_mutex_lock(semaforoMemoria);
	for(i=0;i <cant_paginas; i++){
		void* marcoLibre = dictionary_remove(tabla_desalojar,&i);
		list_add(marcosLibres,marcoLibre);
	}
	tabla_actual = NULL;
	//pthread_mutex_unlock(semaforoMemoria);
	dictionary_destroy(tabla_desalojar);
	int* idRemovido = dictionary_remove(programas_ejecucion,&id);
	free(idRemovido);
	return 0;
}


void* obtenerBytesMemoria(int pagina,int offset,int tamanio){
	//Esto es de la entrega 2
	//solicitarEnSwap(*idProcesoActual,pagina,offset,tamanio);


	//Esto es mas de la entrega 3
	void* obtenido = malloc(tamanio);
	int* marco = dictionary_get(tabla_actual,&pagina);
	int posicionDeMemoria = ((*marco)*umcConfg.configuracionUMC.MARCO_SIZE) + offset;
	//pthread_mutex_lock(semaforoMemoria);
	memcpy(obtenido,(memoriaPrincipal + posicionDeMemoria),tamanio);
	//pthread_mutex_unlock(semaforoMemoria);
	return obtenido;
}

void almacenarBytes(int pagina, int offset, int tamanio, void* buffer){
	//Esto es de la entrega 2
	//almacenarEnSwap(*idProcesoActual,pagina,offset,tamanio,buffer);


	//Esto es mas de la entrega 3
	int* marco = dictionary_get(tabla_actual,&pagina);
	int posicionDeMemoria = ((*marco)*umcConfg.configuracionUMC.MARCO_SIZE) + offset;
	//pthread_mutex_lock(semaforoMemoria);
	memcpy((memoriaPrincipal+posicionDeMemoria),buffer,tamanio);
	//pthread_mutex_unlock(semaforoMemoria);
	//almacenarEnSwap(*idProcesoActual,pagina,offset,tamanio,buffer,40);
	return;
}

void cambiarProceso(int idProceso){
	//Esto es de la entrega 2
	*idProcesoActual = idProceso;
	//Esto es mas de la entrega 3
	/*pthread_mutex_lock(semaforoMemoria);
	tabla_actual = dictionary_get(programas_ejecucion,(char*)&idProceso);
	*idProcesoActual = idProceso;
	pthread_mutex_unlock(semaforoMemoria);*/
}
