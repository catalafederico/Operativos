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
#include "umcClockV2.h"

extern umcNucleo umcConfg;
t_log* log_memoria;
tlb* tlbCache;
pthread_mutex_t semaforoMemoria;
void* memoriaPrincipal;
t_list* marcosLibres;
t_dictionary* tabla_actual;
t_dictionary* programas_ejecucion;
t_dictionary* programas_paraClock;
int* idProcesoActual;
int entradasTLB;
int alocandoPrograma;
int clockModificado;


//---------fin
void* inicializarMemoria(t_reg_config* configuracionUMC){

	clockModificado = configuracionUMC->ALGORITMO;
	int cantidadDeMarcos = configuracionUMC->MARCOS;
	entradasTLB = configuracionUMC->ENTRADAS_TLB;
	tlbCache = list_create();
	inicializarTLB(tlbCache,cantidadDeMarcos);
	log_memoria = log_create("logs/logUmcMemoria.txt","UMC",0,LOG_LEVEL_TRACE);
	pthread_mutex_init(&semaforoMemoria,NULL);
	log_trace(log_memoria,"Creando memoria");
	marcosLibres = list_create();
	programas_ejecucion = dictionary_create();
	programas_paraClock = dictionary_create();
	idProcesoActual = malloc(sizeof(int));
	int i = 1;
	memoriaPrincipal = calloc(cantidadDeMarcos, umcConfg.configuracionUMC.MARCO_SIZE);
	int tamanio = cantidadDeMarcos*umcConfg.configuracionUMC.MARCO_SIZE;
	log_trace(log_memoria,"Memoria inicializada con un tamanio: %d",tamanio);
	for(i = 0;i<cantidadDeMarcos;i++){
		infoPagina* tempFrame = malloc(sizeof(infoPagina));
		tempFrame->nroMarco = i;
		tempFrame->bit_uso = NOUSADO;
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
	}
	if(1) {
		log_trace(log_memoria, "Comienza alocacion de programa id: %d",
				id_proceso);
		int i = 1;
		t_dictionary* pag_frame = dictionary_create();
		//Creo Elemento para el manejo de clock
		reloj* nuevoElemClock = malloc(sizeof(reloj));
		nuevoElemClock->paginasMemoria = list_create();
		nuevoElemClock->puntero = 0;
		int* idClock = malloc(sizeof(int));
		*idClock = id_proceso;
		dictionary_put(programas_paraClock,idClock,nuevoElemClock);
		//
		int* idProceso = malloc(sizeof(int));
		*idProceso = id_proceso;
		log_trace(log_memoria, "Agregado en tabla de proceso id: ");
		tabla_actual = pag_frame;
		*idProcesoActual = id_proceso;
		dictionary_put(programas_ejecucion, idProceso, pag_frame);
		log_trace(log_memoria, "Alocado programa id: %d", id_proceso);
		alocandoPrograma = 1;
		notificarASwapPrograma(id_proceso,paginasRequeridas);
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
		infoPagina* marcoLibre = dictionary_remove(tabla_desalojar,&i);
		list_add(marcosLibres,marcoLibre);
		log_trace(log_memoria,"Pag: %d \tMarco: %d ",i,marcoLibre->nroMarco);
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
	infoPagina* paginaInfo = buscarFrameEnTLB(*idProcesoActual,pagina);
	void* obtenido = malloc(tamanio);
	if(paginaInfo!=NULL){
		estaEnTLB = 1;
	}
	else if(!estaEnTLB)
	{
		paginaInfo = dictionary_get(tabla_actual,&pagina);
		if(paginaInfo->nroMarco == -1){
			reloj* elem =  dictionary_get(programas_paraClock,idProcesoActual);
			//Veo q pagina reemplzar
			int paginaARemplazar = buscarPagAReemplazar(elem->paginasMemoria,&(elem->puntero));
			//Agarro info de la pagina a reemplazar
			infoPagina* paginaAReemplazar = dictionary_get(tabla_actual,&paginaARemplazar);
			//Obtengo toda la pagina para almacenar en swap
			int posicionDeMemoria = ((paginaAReemplazar->nroMarco)*umcConfg.configuracionUMC.MARCO_SIZE);
			void* aAlmacenarEnSwap = malloc(umcConfg.configuracionUMC.MARCO_SIZE);
			memcpy(aAlmacenarEnSwap,(memoriaPrincipal + posicionDeMemoria),umcConfg.configuracionUMC.MARCO_SIZE);
			//La almaceno
			almacenarEnSwap(*idProcesoActual,paginaAReemplazar,aAlmacenarEnSwap);
			//Le doy el marco de la pagina reemplazada a la nueva pagina
			paginaInfo->nroMarco = paginaAReemplazar->nroMarco;
			paginaAReemplazar->nroMarco =-1;
			paginaAReemplazar->modif = 0;
			paginaAReemplazar->bit_uso = 0;
			//saco la pagina actual de la lista de clock ya q no la tengo en memoria y agrego la pagina memoria
			int anterior = (elem->puntero) - 1;
			relojElem* temp = list_get((elem->paginasMemoria),anterior);
			temp->marco = paginaInfo;
			temp->pag = pagina;
			aAlmacenarEnSwap = solicitarEnSwap(*idProcesoActual,pagina);
			memcpy((memoriaPrincipal + posicionDeMemoria),aAlmacenarEnSwap,umcConfg.configuracionUMC.MARCO_SIZE);
			//saco de tlb la pagina si estuviera
			removerDeTLB(*idProcesoActual,paginaARemplazar,-1);
			free(aAlmacenarEnSwap);
		}
		//agrego en la tlb marco
		//lo inserto al principio ya q es el ultimo usado
		insertarEnTLB(*idProcesoActual,pagina,paginaInfo,0);
	}
	int posicionDeMemoria = ((paginaInfo->nroMarco)*umcConfg.configuracionUMC.MARCO_SIZE) + offset;
	memcpy(obtenido,(memoriaPrincipal + posicionDeMemoria),tamanio);
	paginaInfo->bit_uso = USADO;
	pthread_mutex_unlock(&semaforoMemoria);
	return obtenido;
	//Esto es mas de la entrega 3
}

void almacenarBytes(int pagina, int offset, int tamanio, void* buffer){

	int recienAsignado = 0;
	int estaEnTlb = 0;
	infoPagina* paginaInfo;
	log_trace(log_memoria,"Almacenar - id: %d pag: %d offset: %d tamanio: %d",*idProcesoActual,pagina,offset,tamanio);
	//Me fijo si tiene clave q seria la pagina
	//Si no tiene le tengo q asignar un frame
	if(!dictionary_has_key(tabla_actual,&pagina)){
		int* paginaAPoner = malloc(sizeof(int));
		*paginaAPoner = pagina;
		//obtengo un marco y lo saco
		//ME fijo si hay marcos libres
		if (list_size(marcosLibres)>= 1) {
			paginaInfo = list_remove(marcosLibres, 0);
			//asigno marco a la pagina, DICCIONARIO YA DEBE ESTAR CREADO
			log_trace(log_memoria, "Pag: %d \tMarco: %d ", pagina, paginaInfo->nroMarco);
			dictionary_put(tabla_actual, paginaAPoner, paginaInfo);
			insertarEnTLB(*idProcesoActual,pagina,paginaInfo,0);
			//Insterto en lista para clock
			relojElem* nuevoElemento = malloc(sizeof(nuevoElemento));
			reloj* paraClock = dictionary_get(programas_paraClock,idProcesoActual);
			nuevoElemento->marco = paginaInfo;
			nuevoElemento->pag = pagina;
			list_add(paraClock->paginasMemoria,nuevoElemento);
			//Elemento insertado
		}
		//No hay marcos libres entonces hago reemplazo
		else{
			paginaInfo = malloc(sizeof(infoPagina));
			paginaInfo->bit_uso = 0;
			paginaInfo->modif = 0;
			reloj* elem =  dictionary_get(programas_paraClock,idProcesoActual);
			//Veo q pagina reemplzar
			int paginaARemplazar = buscarPagAReemplazar(elem->paginasMemoria,&(elem->puntero));
			//Agarro info de la pagina a reemplazar
			infoPagina* paginaAReemplazar = dictionary_get(tabla_actual,&paginaARemplazar);
			//Obtengo toda la pagina para almacenar en swap
			int posicionDeMemoria = ((paginaAReemplazar->nroMarco)*(umcConfg.configuracionUMC.MARCO_SIZE));
			void* aAlmacenarEnSwap = malloc(umcConfg.configuracionUMC.MARCO_SIZE);
			memcpy(aAlmacenarEnSwap,(memoriaPrincipal + posicionDeMemoria),umcConfg.configuracionUMC.MARCO_SIZE);
			//La almaceno
			almacenarEnSwap(*idProcesoActual,paginaARemplazar,aAlmacenarEnSwap);
			//Le doy el marco de la pagina reemplazada a la nueva pagina
			paginaInfo->nroMarco = paginaAReemplazar->nroMarco;
			paginaAReemplazar->nroMarco =-1;
			paginaAReemplazar->modif = 0;
			paginaAReemplazar->bit_uso = 0;
			//saco la pagina actual de la lista de clock ya q no la tengo en memoria y agrego la pagina memoria
			int anterior = (elem->puntero) - 1;
			relojElem* temp = list_get((elem->paginasMemoria),anterior);
			temp->marco = paginaInfo;
			temp->pag = pagina;
			//saco de tlb la pagina si estuviera
			removerDeTLB(*idProcesoActual,paginaARemplazar,-1);
			dictionary_put(tabla_actual, paginaAPoner, paginaInfo);
			free(aAlmacenarEnSwap);
			insertarEnTLB(*idProcesoActual, pagina, paginaInfo, 0);
		}
		recienAsignado = 1;
	}
	//Busco Marco, primero me fijo si fue recien asignado, si fue recien asignado, ya tengo el marco,
	//sino me fijo si esta en tlb, sino me fijo en memoria, y sino swap
	if (!recienAsignado) {
		paginaInfo = buscarFrameEnTLB(*idProcesoActual, pagina);
		if (paginaInfo != NULL) {
			estaEnTlb = 1;
		} else if (!estaEnTlb) {
			paginaInfo = dictionary_get(tabla_actual, &pagina);
			//-1 dice q esta en swap la pagina
			if(paginaInfo->nroMarco == -1){
				reloj* elem =  dictionary_get(programas_paraClock,idProcesoActual);
				//Veo q pagina reemplzar
				int paginaARemplazar = buscarPagAReemplazar(elem->paginasMemoria,&(elem->puntero));
				//Agarro info de la pagina a reemplazar
				infoPagina* paginaAReemplazar = dictionary_get(tabla_actual,&paginaARemplazar);
				//Obtengo toda la pagina para almacenar en swap
				int posicionDeMemoria = ((paginaAReemplazar->nroMarco)*umcConfg.configuracionUMC.MARCO_SIZE);
				void* aAlmacenarEnSwap = malloc(umcConfg.configuracionUMC.MARCO_SIZE);
				memcpy(aAlmacenarEnSwap,(memoriaPrincipal + posicionDeMemoria),umcConfg.configuracionUMC.MARCO_SIZE);
				//La almaceno
				almacenarEnSwap(*idProcesoActual,paginaARemplazar,aAlmacenarEnSwap);
				//Le doy el marco de la pagina reemplazada a la nueva pagina
				paginaInfo->nroMarco = paginaAReemplazar->nroMarco;
				paginaAReemplazar->nroMarco =-1;
				paginaAReemplazar->modif = 0;
				paginaAReemplazar->bit_uso = 0;
				//saco la pagina actual de la lista de clock ya q no la tengo en memoria y agrego la pagina memoria
				int anterior = (elem->puntero) - 1;
				relojElem* temp = list_get((elem->paginasMemoria),anterior);
				temp->marco = paginaInfo;
				temp->pag = pagina;
				//saco de tlb la pagina si estuviera
				removerDeTLB(*idProcesoActual,paginaAReemplazar,-1);
				aAlmacenarEnSwap = solicitarEnSwap(*idProcesoActual,pagina);
				memcpy((memoriaPrincipal + posicionDeMemoria),aAlmacenarEnSwap,umcConfg.configuracionUMC.MARCO_SIZE);
				free(aAlmacenarEnSwap);
			}
			insertarEnTLB(*idProcesoActual, pagina, paginaInfo, 0);
		}
	}
	int posicionDeMemoria = ((paginaInfo->nroMarco)*umcConfg.configuracionUMC.MARCO_SIZE) + offset;
	memcpy((memoriaPrincipal+posicionDeMemoria),buffer,tamanio);
	paginaInfo->bit_uso = USADO;
	paginaInfo->modif = 1;
	if(!alocandoPrograma)
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














































































