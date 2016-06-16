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
#include "umcTlb.h"


void* memoriaPrincipal;
extern umcNucleo umcConfg;
t_log* log_memoria;
tlb* tlbCache;
//Menajo de la memoria
t_list* marcosLibres;
t_dictionary* tabla_actual;
int* idProcesoActual;
t_dictionary* programas_ejecucion;
pthread_mutex_t semaforoMemoria;
int entradasTLB;

//---------fin
void* inicializarMemoria(t_reg_config* configuracionUMC){

	int cantidadDeMarcos = (*configuracionUMC).MARCOS;
	entradasTLB = configuracionUMC->ENTRADAS_TLB;
	tlbCache = list_create();
	inicializarTLB(tlbCache,cantidadDeMarcos);
	log_memoria = log_create("logs/logUmcMemoria.txt","UMC",0,LOG_LEVEL_TRACE);
	pthread_mutex_init(&semaforoMemoria,NULL);
	log_trace(log_memoria,"Creando memoria");
	marcosLibres = list_create();
	programas_ejecucion = dictionary_create();
	idProcesoActual = malloc(sizeof(int));
	int i = 1;
	memoriaPrincipal = calloc(cantidadDeMarcos, umcConfg.configuracionUMC.MARCO_SIZE);
	log_trace(log_memoria,"Memoria inicializada con un tamanio: %d",cantidadDeMarcos*umcConfg.configuracionUMC.MARCO_SIZE);
	for(i = 0;i<cantidadDeMarcos;i++){
		frame* tempFrame = malloc(sizeof(frame));
		tempFrame->nro = i;
		tempFrame->bit_uso = NOUSADO;
		tempFrame->enUMC = 1;
		tempFrame->modif = 0;
		list_add(marcosLibres,tempFrame);
	}
	log_trace(log_memoria,"Marcos libres cargados: %d:",i);
	return memoriaPrincipal;

}

int alocarPrograma(int paginasRequeridas, int id_proceso) {

	if (paginasRequeridas > umcConfg.configuracionUMC.MARCO_X_PROC) {
		log_trace(log_memoria,
				"Rechazo programa id: %d , paginas requeridas: %d \n",
				paginasRequeridas, id_proceso);
		return -1;
	} else if (list_size(marcosLibres) < paginasRequeridas) {
		int paginasRestantesNecesarias = paginasRequeridas - list_size(marcosLibres);
		int i;
		for(i=0;i<paginasRestantesNecesarias;i++){
			//Corro algoritmo clock o clock modificado
			//para liberar la cant n de pag necesarias
		}
	}
	if(1) {
		log_trace(log_memoria, "Comienza alocacion de programa id: %d",
				id_proceso);
		int i = 1;
		t_dictionary* pag_frame = dictionary_create();
		int* idProceso = malloc(sizeof(int));
		*idProceso = id_proceso;
		log_trace(log_memoria, "Agregado en tabla de proceso id: ");
		for (i = 0; i < paginasRequeridas; i++) {
			int* pagina = malloc(sizeof(int));
			*pagina = i;
			//obtengo un marco y lo saco
			frame* tempFrame = list_remove(marcosLibres, 0);
			//asigno marco a la pagina, DICCIONARIO YA DEBE ESTAR CREADO
			log_trace(log_memoria, "Pag: %d \tMarco: %d ", i, tempFrame->nro);
			dictionary_put(pag_frame, pagina, tempFrame);
		}
		tabla_actual = pag_frame;
		*idProcesoActual = id_proceso;
		dictionary_put(programas_ejecucion, idProceso, pag_frame);
		pthread_mutex_unlock(&semaforoMemoria);
		log_trace(log_memoria, "Alocado programa id: %d", id_proceso);
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

	log_trace(log_memoria,"Solcitud - id: %d pag: %d offset: %d tamanio: %d",*idProcesoActual,pagina,offset,tamanio);
	int estaEnTLB = 0;
	frame* marco = buscarFrameEnTLB(*idProcesoActual,pagina);
	void* obtenido = malloc(tamanio);
	if(marco!=NULL){
		estaEnTLB = 1;
	}
	else if(!estaEnTLB)
	{
		marco = dictionary_get(tabla_actual,&pagina);
		if(!marco->enUMC){
		}

		//agrego en la tlb marco
		//lo inserto al principio ya q es el ultimo usado
		insertarEnTLB(*idProcesoActual,pagina,marco,0);
	}
	int posicionDeMemoria = ((marco->nro)*umcConfg.configuracionUMC.MARCO_SIZE) + offset;
	memcpy(obtenido,(memoriaPrincipal + posicionDeMemoria),tamanio);
	marco->bit_uso = USADO;
	pthread_mutex_unlock(&semaforoMemoria);
	return obtenido;
	//Esto es mas de la entrega 3
}

void almacenarBytes(int pagina, int offset, int tamanio, void* buffer){

	int estaEnTlb = 0;
	log_trace(log_memoria,"Almacenar - id: %d pag: %d offset: %d tamanio: %d",*idProcesoActual,pagina,offset,tamanio);
	//CON TLB
	frame* marco;
	marco = buscarFrameEnTLB(*idProcesoActual,pagina);
	if(marco!=NULL){
		estaEnTlb=1;
	}
	else if(!estaEnTlb){
		marco = dictionary_get(tabla_actual,&pagina);
		if(!marco->enUMC){
		}
		insertarEnTLB(*idProcesoActual,pagina,marco,0);
	}

	int posicionDeMemoria = ((marco->nro)*umcConfg.configuracionUMC.MARCO_SIZE) + offset;
	memcpy((memoriaPrincipal+posicionDeMemoria),buffer,tamanio);
	marco->bit_uso = USADO;
	pthread_mutex_unlock(&semaforoMemoria);
	return;
}

void cambiarProceso(int idProceso){
	//Esto es de la entrega 2
		pthread_mutex_lock(&semaforoMemoria);
		*idProcesoActual = idProceso;
		tabla_actual = dictionary_get(programas_ejecucion,&idProceso);
		log_trace(log_memoria,"CambiarProceso - idAnterior: %d , idNuevo: %d",*idProcesoActual,idProceso);
		return;
}














































































