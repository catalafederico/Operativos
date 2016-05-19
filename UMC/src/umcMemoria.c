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
t_log* log_memoria;

//Menajo de la memoria
t_list* marcosLibres;
t_dictionary* tabla_actual;
int* idProcesoActual;
t_dictionary* programas_ejecucion;
pthread_mutex_t* semaforoMemoria;


void* inicializarMemoria(t_reg_config* configuracionUMC){
	log_memoria = log_create("logs/logUmcMemoria.txt","UMC",0,LOG_LEVEL_TRACE);
	//pthread_mutex_init(semaforoMemoria,NULL);
	log_trace(log_memoria,"Creando memoria");
	marcosLibres = list_create();
	programas_ejecucion = dictionary_create();
	idProcesoActual = malloc(sizeof(int));
	int i = 1;
	int cantidadDeMarcos = (*configuracionUMC).MARCOS;
	memoriaPrincipal = calloc(cantidadDeMarcos, umcConfg.configuracionUMC.MARCO_SIZE);
	log_trace(log_memoria,"Memoria inicializada con un tamanio: %d",cantidadDeMarcos*umcConfg.configuracionUMC.MARCO_SIZE);
	for(i = 0;i<cantidadDeMarcos;i++){
		int* tempMarcoNro = malloc(sizeof(int));
		*tempMarcoNro = i;
		list_add(marcosLibres,tempMarcoNro);
	}
	log_trace(log_memoria,"Marcos libres cargados: %d:",i);
	return memoriaPrincipal;
}


int alocarPrograma(int paginasRequeridas, int id_proceso){
	//Esto va para la entrega 2
	/*notificarASwapPrograma(paginasRequeridas,id_proceso);
	*idProcesoActual = id_proceso;
	return 0;*/
	//Esto es para la entrega 3
	if(list_size(marcosLibres) < paginasRequeridas  && paginasRequeridas < umcConfg.configuracionUMC.MARCO_X_PROC){
		log_trace(log_memoria,"Rechazo programa id: %d , paginas requeridas: %d \n",paginasRequeridas,id_proceso);
		return -1;
	}
	else{
		log_trace(log_memoria,"Comienza alocacion de programa id: %d", id_proceso);
		int* idProceso = malloc(sizeof(int));
		*idProceso = id_proceso;
		int i = 1;
		t_dictionary* pag_frame = dictionary_create();
		//pthread_mutex_lock(semaforoMemoria);
		log_trace(log_memoria,"Agregado en tabla de proceso id: ");
		for(i = 0;i<paginasRequeridas;i++){
			int* pagina = malloc(sizeof(int));
			*pagina = i;
			//obtengo un marco y lo saco
			int* nroMarco = list_remove(marcosLibres,0);
			//asigno marco a la pagina, DICCIONARIO YA DEBE ESTAR CREADO
			log_trace(log_memoria,"Pag: %d \tMarco: %d ",i,*nroMarco);
			dictionary_put(pag_frame,pagina,nroMarco);
		}
		tabla_actual = pag_frame;
		//pthread_mutex_unlock(semaforoMemoria);
		dictionary_put(programas_ejecucion,idProceso,pag_frame);
		log_trace(log_memoria,"Alocado programa id: %d", id_proceso);
		return 0;
	}

}


int desalojarPrograma(int id){

	//Esto es de la entrega 2
	/*notificarASwapFinPrograma(id);
	return 0;*/

	//Esto es mas de la entrega 3
	log_trace(log_memoria,"Comienza desalojo de programa id: %d", id);
	t_dictionary* tabla_desalojar = dictionary_get(programas_ejecucion,&id);
	int cant_paginas = dictionary_size(tabla_desalojar);
	int i;
	//pthread_mutex_lock(semaforoMemoria);
	log_trace(log_memoria,"Agregado a marcos libres:");
	for(i=0;i <cant_paginas; i++){
		int* marcoLibre = dictionary_remove(tabla_desalojar,&i);
		list_add(marcosLibres,marcoLibre);
		log_trace(log_memoria,"Pag: %d \tMarco: %d ",i,*marcoLibre);
	}
	tabla_actual = NULL;
	//pthread_mutex_unlock(semaforoMemoria);
	dictionary_destroy(tabla_desalojar);
	int* idRemovido = dictionary_remove(programas_ejecucion,&id);
	free(idRemovido);
	log_trace(log_memoria,"Programa desalojado");
	return 0;
}


void* obtenerBytesMemoria(int pagina,int offset,int tamanio){
	//Esto es de la entrega 2
	//solicitarEnSwap(*idProcesoActual,pagina,offset,tamanio);


	//Esto es mas de la entrega 3
	log_trace(log_memoria,"Solcitud - id: %d pag: %d offset: %d tamanio: %d",*idProcesoActual,pagina,offset,tamanio);
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

	log_trace(log_memoria,"Almacenar - id: %d pag: %d offset: %d tamanio: %d",*idProcesoActual,pagina,offset,tamanio);
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
	log_trace(log_memoria,"CambiarProceso - idAnterior: %d , idNuevo: %d",*idProcesoActual,idProceso);
	//Esto es mas de la entrega 3
	/*pthread_mutex_lock(semaforoMemoria);
	tabla_actual = dictionary_get(programas_ejecucion,(char*)&idProceso);
	*idProcesoActual = idProceso;
	pthread_mutex_unlock(semaforoMemoria);*/
}
