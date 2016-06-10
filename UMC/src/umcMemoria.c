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
#include "umcMemoria.h"


void* memoriaPrincipal;
extern umcNucleo umcConfg;
t_log* log_memoria;

//Menajo de la memoria
t_list* marcosLibres;
t_dictionary* tabla_actual;
int* idProcesoActual;
t_dictionary* programas_ejecucion;
pthread_mutex_t semaforoMemoria;

//---------fin
void* inicializarMemoria(t_reg_config* configuracionUMC){

	//int cantEntradas= umcConfg.configuracionUMC.ENTRADAS_TLB;
	log_memoria = log_create("logs/logUmcMemoria.txt","UMC",0,LOG_LEVEL_TRACE);
	pthread_mutex_init(&semaforoMemoria,NULL);
	log_trace(log_memoria,"Creando memoria");
	marcosLibres = list_create();
	programas_ejecucion = dictionary_create();
	idProcesoActual = malloc(sizeof(int));
	int i = 1;
	int cantidadDeMarcos = (*configuracionUMC).MARCOS;
	memoriaPrincipal = calloc(cantidadDeMarcos, umcConfg.configuracionUMC.MARCO_SIZE);
	log_trace(log_memoria,"Memoria inicializada con un tamanio: %d",cantidadDeMarcos*umcConfg.configuracionUMC.MARCO_SIZE);
	for(i = 0;i<cantidadDeMarcos;i++){
		frame* tempFrame = malloc(sizeof(frame));
		tempFrame->nro = i;
		tempFrame->bit_uso = NOUSADO;
		list_add(marcosLibres,tempFrame);
	}
	log_trace(log_memoria,"Marcos libres cargados: %d:",i);
	return memoriaPrincipal;
}

int alocarPrograma(int paginasRequeridas, int id_proceso){

	if(list_size(marcosLibres) < paginasRequeridas  && paginasRequeridas < umcConfg.configuracionUMC.MARCO_X_PROC){
		log_trace(log_memoria,"Rechazo programa id: %d , paginas requeridas: %d \n",paginasRequeridas,id_proceso);
		return -1;
	}
	else{
		log_trace(log_memoria,"Comienza alocacion de programa id: %d", id_proceso);
		int i = 1;
		t_dictionary* pag_frame = dictionary_create();
		int* idProceso = malloc(sizeof(int));
		*idProceso = id_proceso;
		log_trace(log_memoria,"Agregado en tabla de proceso id: ");
		for(i = 0;i<paginasRequeridas;i++){
			int* pagina = malloc(sizeof(int));
			*pagina = i;
			//obtengo un marco y lo saco
			frame* tempFrame = list_remove(marcosLibres,0);
			//asigno marco a la pagina, DICCIONARIO YA DEBE ESTAR CREADO
			log_trace(log_memoria,"Pag: %d \tMarco: %d ",i,tempFrame->nro);
			dictionary_put(pag_frame,pagina,tempFrame);
		}
		tabla_actual = pag_frame;
		*idProcesoActual = id_proceso;
		dictionary_put(programas_ejecucion,idProceso,pag_frame);
		pthread_mutex_unlock(&semaforoMemoria);
		//habria que poner en 1 a los marcos de las pag asignadas
		//faltaria saber si ya no se pueden asignar mas paginas si esto no se puede hay que desalojar con el algoritmo clock
		//habria que dejar un puntero marcando la ultima pagina que se asigno para poder realizar el algoritmo
		log_trace(log_memoria,"Alocado programa id: %d", id_proceso);
		notificarASwapPrograma(id_proceso,paginasRequeridas);
		//Esto es para la entrega 3
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
	log_trace(log_memoria,"Agregado a marcos libres:");
	for(i=0;i <cant_paginas; i++){
		frame* marcoLibre = dictionary_remove(tabla_desalojar,&i);
		list_add(marcosLibres,marcoLibre);
		log_trace(log_memoria,"Pag: %d \tMarco: %d ",i,marcoLibre->nro);
	}
	tabla_actual = NULL;
	dictionary_destroy(tabla_desalojar);
	int* idRemovido = dictionary_remove(programas_ejecucion,&id);
	free(idRemovido);
	pthread_mutex_unlock(&semaforoMemoria);
	log_trace(log_memoria,"Programa desalojado");
	return 0;
}

void* obtenerBytesMemoria(int pagina,int offset,int tamanio){
	solicitarEnSwap(*idProcesoActual,pagina);
	//Esto es mas de la entrega 3
	log_trace(log_memoria,"Solcitud - id: %d pag: %d offset: %d tamanio: %d",*idProcesoActual,pagina,offset,tamanio);
	void* obtenido = malloc(tamanio);
	frame* marco = dictionary_get(tabla_actual,&pagina);
	int posicionDeMemoria = ((marco->nro)*umcConfg.configuracionUMC.MARCO_SIZE) + offset;
	memcpy(obtenido,(memoriaPrincipal + posicionDeMemoria),tamanio);
	marco->bit_uso = USADO;
	pthread_mutex_unlock(&semaforoMemoria);
	return obtenido;
}

void almacenarBytes(int pagina, int offset, int tamanio, void* buffer){
	//Esto es de la entrega 2
	log_trace(log_memoria,"Almacenar - id: %d pag: %d offset: %d tamanio: %d",*idProcesoActual,pagina,offset,tamanio);
	//Esto es mas de la entrega 3
	frame* marco = dictionary_get(tabla_actual,&pagina);
	int posicionDeMemoria = ((marco->nro)*umcConfg.configuracionUMC.MARCO_SIZE) + offset;
	memcpy((memoriaPrincipal+posicionDeMemoria),buffer,tamanio);
	marco->bit_uso = USADO;
	pthread_mutex_unlock(&semaforoMemoria);
	void* bufferALLPAGE = obtenerBytesMemoria(pagina,0,umcConfg.configuracionUMC.MARCO_SIZE);
	almacenarEnSwap(*idProcesoActual,pagina,bufferALLPAGE);
	//Probar CPU
	/*int a = 5;
	void* asd = &a;
	almacenarEnSwap(*idProcesoActual,pagina,asd);*/
	return;
}

void cambiarProceso(int idProceso){
	//Esto es de la entrega 2
		pthread_mutex_lock(&semaforoMemoria);
		*idProcesoActual = idProceso;
		log_trace(log_memoria,"CambiarProceso - idAnterior: %d , idNuevo: %d",*idProcesoActual,idProceso);
		//Esto es mas de la entrega 3
		/*
		 * pthread_mutex_lock(semaforoMemoria);
		tabla_actual = dictionary_get(programas_ejecucion,(char*)&idProceso);
		*idProcesoActual = idProceso;
		pthread_mutex_unlock(semaforoMemoria);
		*/
		return;
}














































































