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
extern pthread_mutex_t memoriaLibre;;
t_log* log_memoria;
tlb* tlbCache; // liberar al desalojar
pthread_mutex_t semaforoMemoria;
void* memoriaPrincipal;
t_list* marcosLibres;
t_dictionary* tabla_actual;
t_dictionary* programas_ejecucion; // liberar al desalojar
t_dictionary* programas_paraClock; // liberar al desalojar
int* idProcesoActual;
int entradasTLB;
int clockModificado;


//---------fin
void* inicializarMemoria(t_reg_config* configuracionUMC){

	int cantidadDeMarcos = configuracionUMC->MARCOS;
	clockModificado = configuracionUMC->ALGORITMO;
	entradasTLB = configuracionUMC->ENTRADAS_TLB;
	tlbCache = list_create();
	inicializarTLB(tlbCache,entradasTLB);
	log_memoria = log_create("logs/logUmcMemoria.txt","UMC",0,LOG_LEVEL_TRACE);
	pthread_mutex_init(&semaforoMemoria,NULL);
	pthread_mutex_init(&memoriaLibre,NULL);
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
		int* tempFrame = malloc(sizeof(int));
		*tempFrame = i;
		list_add(marcosLibres,tempFrame);
	}
	log_trace(log_memoria,"Marcos libres cargados: %d:",i);
	return memoriaPrincipal;

}

int alocarPrograma(int paginasRequeridas, int id_proceso,
		t_dictionary* codigoPrograma) {

	//log_trace(log_memoria,"Lock on");
	pthread_mutex_lock(&semaforoMemoria);

	if (paginasRequeridas > umcConfg.configuracionUMC.MARCO_X_PROC) {
		log_trace(log_memoria,
				"Rechazo programa id: %d , paginas requeridas: %d \n",
				paginasRequeridas, id_proceso);
		pthread_mutex_unlock(&semaforoMemoria);
		return -1;
	}
	if (!notificarASwapPrograma(id_proceso, paginasRequeridas)) {
		log_trace(log_memoria,
				"Rechazo programa id: %d , paginas requeridas: %d \n",
				paginasRequeridas, id_proceso);
		pthread_mutex_unlock(&semaforoMemoria);
		return -1;
	}
	log_trace(log_memoria, "Comienza alocacion de programa id: %d", id_proceso);
	int i = 1;
	//Creo yable pag_marco
	t_dictionary* pag_frame = dictionary_create();
	for (i = 0; i < paginasRequeridas; i++) {
		infoPagina* pagina = malloc(sizeof(infoPagina));
		pagina->bit_uso = 0;
		pagina->nroMarco = -1;
		pagina->modif = 0;
		dictionary_put(pag_frame, &i, pagina);
	}
	//Creo Elemento para el manejo de clock
	reloj* nuevoElemClock = malloc(sizeof(reloj));
	nuevoElemClock->paginasMemoria = list_create();
	nuevoElemClock->puntero = 0;
	/*int* idClock = malloc(sizeof(int));
	*idClock = id_proceso;*/
	dictionary_put(programas_paraClock, &id_proceso, nuevoElemClock);
	//Agrego table de codigo o todas las tablas
	/*int* idProceso = malloc(sizeof(int));
	*idProceso = id_proceso;*/
	log_trace(log_memoria, "Agregado en tabla de proceso id: ");
	dictionary_put(programas_ejecucion, &id_proceso, pag_frame);
	log_trace(log_memoria, "Alocado programa id: %d", id_proceso);
	//Notifico a swap
	int paginasDeCodigo = dictionary_size(codigoPrograma);
	for (i = 0; i < paginasDeCodigo; i++) {
		void* aAlmacenarEnSwap = dictionary_get(codigoPrograma, &i);
		almacenarEnSwap(id_proceso, i, aAlmacenarEnSwap);
	}
	//log_trace(log_memoria,"Lock off");
	pthread_mutex_unlock(&semaforoMemoria);
	return 0;
}

int desalojarPrograma(int id){

	//log_trace(log_memoria,"Lock on");
	pthread_mutex_lock(&semaforoMemoria);
	log_trace(log_memoria,"Comienza desalojo de programa id: %d", id);
	t_dictionary* tabla_desalojar = dictionary_remove(programas_ejecucion,&id);
	int cant_paginas = dictionary_size(tabla_desalojar);
	int i;
	log_trace(log_memoria,"Agregado a marcos libres:");
	for(i=0;i <cant_paginas; i++){
		removerDeTLB(id,i,-1);
		infoPagina* marcoLibre = dictionary_remove(tabla_desalojar,&i);
		if(marcoLibre->nroMarco!=-1){
			int* nuevoMArco = malloc(sizeof(int));
			*nuevoMArco = marcoLibre->nroMarco;
			list_add(marcosLibres,nuevoMArco);
			log_trace(log_memoria,"Liberado Pag: %d \tMarco: %d ",i,marcoLibre->nroMarco);
		}
		free(marcoLibre);
	}
	dictionary_destroy(tabla_desalojar);
	//desalojo programas para clock9
	reloj* elemento = dictionary_get(programas_paraClock,&id);
	cant_paginas = list_size(elemento->paginasMemoria);
	for(i=0;i<cant_paginas;i++){
		relojElem* temp = list_remove(elemento->paginasMemoria,0);
		free(temp);
	}
	list_destroy(elemento->paginasMemoria);
	free(elemento);
	notificarASwapFinPrograma(id);
	pthread_mutex_unlock(&semaforoMemoria);
	log_trace(log_memoria,"Programa desalojado");
	return 0;
}

void* obtenerBytesMemoria(int pagina,int offset,int tamanio){
	//Chekeo SEG FAULT
	if(pagina>=dictionary_size(tabla_actual)){
		pthread_mutex_unlock(&semaforoMemoria);
		return NULL;
	}
	log_trace(log_memoria,"Solcitud - id: %d pag: %d offset: %d tamanio: %d",*idProcesoActual,pagina,offset,tamanio);
	int estaEnTLB = 0;
	int posicionDeMemoria;
	infoPagina* paginaBuscada = buscarFrameEnTLB(*idProcesoActual,pagina);
	void* obtenido = malloc(tamanio);
	if(paginaBuscada!=NULL){
		estaEnTLB = 1;
	}
	else if(!estaEnTLB)
	{
		paginaBuscada = dictionary_get(tabla_actual,&pagina);
		reloj* pagEnMemoria = dictionary_get(programas_paraClock,idProcesoActual);
		if(!(paginaBuscada->nroMarco == -1)){

		}
		//Si hay marcos libres, le asigno uno, la pag no esta en mry
		else if(list_size(marcosLibres)>0 && list_size(pagEnMemoria->paginasMemoria)<umcConfg.configuracionUMC.MARCO_X_PROC){
			int* valor = list_remove(marcosLibres,0);
			paginaBuscada->nroMarco = *valor;
			free(valor);
			void* contenidoDeLaPagina = solicitarEnSwap(*idProcesoActual,pagina);
			//copio sin el offset ya que copio la pag entera
			int offsetTotal = paginaBuscada->nroMarco*umcConfg.configuracionUMC.MARCO_SIZE;
			memcpy(memoriaPrincipal + offsetTotal,contenidoDeLaPagina,umcConfg.configuracionUMC.MARCO_SIZE);
			relojElem* nuevoElem = malloc(sizeof(relojElem));
			nuevoElem->pag = pagina;
			nuevoElem->marco = paginaBuscada;
			list_add(pagEnMemoria->paginasMemoria,nuevoElem);
			free(contenidoDeLaPagina);
		}
		//Me fijo si hay alguna pagina en memoria, ya q no hay marcos libres, para sustituirla
		else if(list_size(pagEnMemoria->paginasMemoria)>0){
			reloj* elem =  dictionary_get(programas_paraClock,idProcesoActual);
			//Veo q pagina reemplzar
			int paginaARemplazar = buscarPagAReemplazar(elem->paginasMemoria,&(elem->puntero));
			//Agarro info de la pagina a reemplazar
			infoPagina* paginaAReemplazar = dictionary_get(tabla_actual,&paginaARemplazar);
			//Obtengo toda la pagina para almacenar en swap
			posicionDeMemoria = ((paginaAReemplazar->nroMarco)*umcConfg.configuracionUMC.MARCO_SIZE);
			void* aAlmacenarEnSwap = malloc(umcConfg.configuracionUMC.MARCO_SIZE);
			if (paginaAReemplazar->modif) {
				memcpy(aAlmacenarEnSwap, (memoriaPrincipal + posicionDeMemoria),
						umcConfg.configuracionUMC.MARCO_SIZE);
				//La almaceno
				almacenarEnSwap(*idProcesoActual, paginaARemplazar,
						aAlmacenarEnSwap);
			}
			//Le doy el marco de la pagina reemplazada a la nueva pagina
			paginaBuscada->nroMarco = paginaAReemplazar->nroMarco;
			paginaAReemplazar->nroMarco =-1;
			paginaAReemplazar->modif = 0;
			paginaAReemplazar->bit_uso = 0;
			//saco la pagina actual de la lista de clock ya q no la tengo en memoria y agrego la pagina memoria
			int anterior = (elem->puntero) - 1;
			relojElem* temp = list_get((elem->paginasMemoria),anterior);
			temp->marco = paginaBuscada;
			temp->pag = pagina;
			aAlmacenarEnSwap = solicitarEnSwap(*idProcesoActual,pagina);
			memcpy((memoriaPrincipal + posicionDeMemoria),aAlmacenarEnSwap,umcConfg.configuracionUMC.MARCO_SIZE);
			//saco de tlb la pagina si estuviera
			removerDeTLB(*idProcesoActual,paginaARemplazar,-1);
			free(aAlmacenarEnSwap);
		}
		// sino hay pag en memoria y no hay marcos libres me quede sin memoria
		//avisarle a cpu?
		else{
			//log_trace(log_memoria,"Lock off");
			pthread_mutex_unlock(&semaforoMemoria);
			return strdup("#");
		}
		//agrego en la tlb marco
		//lo inserto al principio ya q es el ultimo usado
		insertarEnTLB(*idProcesoActual,pagina,paginaBuscada,0);
	}
	posicionDeMemoria = ((paginaBuscada->nroMarco)*umcConfg.configuracionUMC.MARCO_SIZE) + offset;
	memcpy(obtenido,(memoriaPrincipal + posicionDeMemoria),tamanio);
	paginaBuscada->bit_uso = USADO;
	//log_trace(log_memoria,"Lock off");
	pthread_mutex_unlock(&semaforoMemoria);
	return obtenido;
	//Esto es mas de la entrega 3
}

int almacenarBytes(int pagina, int offset, int tamanio, void* buffer) {
	//Chekeo SEG FAULT
	if(pagina>=dictionary_size(tabla_actual)){
		pthread_mutex_unlock(&semaforoMemoria);
		return 0;
	}
	int posicionDeMemoria;
	int recienAsignado = 0;
	int estaEnTlb = 0;
	infoPagina* paginaBuscada;
	log_trace(log_memoria, "Almacenar - id: %d pag: %d offset: %d tamanio: %d",
			*idProcesoActual, pagina, offset, tamanio);
	//Busco Marco, primero me fijo si fue recien asignado, si fue recien asignado, ya tengo el marco,
	//sino me fijo si esta en tlb, sino me fijo en memoria, y sino swap
	paginaBuscada = buscarFrameEnTLB(*idProcesoActual, pagina);
	if (paginaBuscada != NULL) {
		estaEnTlb = 1;
	} else if (!estaEnTlb) {
		paginaBuscada = dictionary_get(tabla_actual, &pagina);
		reloj* pagEnMemoria = dictionary_get(programas_paraClock,idProcesoActual);
		//-1 dice q esta en swap la pagina
		if(!(paginaBuscada->nroMarco == -1)){

		}
		else if (list_size(marcosLibres) > 0 && list_size(pagEnMemoria->paginasMemoria)<umcConfg.configuracionUMC.MARCO_X_PROC) {
			int* valor = list_remove(marcosLibres,0);
			paginaBuscada->nroMarco = *valor;
			free(valor);
			void* contenidoDeLaPagina = solicitarEnSwap(*idProcesoActual,
					pagina);
			//copio sin el offset ya que copio la pag entera
			int offsetTotal = paginaBuscada->nroMarco
					* umcConfg.configuracionUMC.MARCO_SIZE;
			memcpy(memoriaPrincipal + offsetTotal, contenidoDeLaPagina,
					umcConfg.configuracionUMC.MARCO_SIZE);

			relojElem* nuevoElem = malloc(sizeof(relojElem));
			nuevoElem->pag = pagina;
			nuevoElem->marco = paginaBuscada;
			list_add(pagEnMemoria->paginasMemoria,nuevoElem);
			free(contenidoDeLaPagina);
		}
		else if (list_size(pagEnMemoria->paginasMemoria)>0) {
			reloj* elem = dictionary_get(programas_paraClock, idProcesoActual);
			//Veo q pagina reemplzar
			int paginaARemplazar = buscarPagAReemplazar(elem->paginasMemoria,
					&(elem->puntero));
			//Agarro info de la pagina a reemplazar
			infoPagina* paginaAReemplazar = dictionary_get(tabla_actual,
					&paginaARemplazar);
			//Obtengo toda la pagina para almacenar en swap
			posicionDeMemoria = ((paginaAReemplazar->nroMarco)
					* umcConfg.configuracionUMC.MARCO_SIZE);
			void* aAlmacenarEnSwap = malloc(
					umcConfg.configuracionUMC.MARCO_SIZE);
			if (paginaAReemplazar->modif) {
				memcpy(aAlmacenarEnSwap, (memoriaPrincipal + posicionDeMemoria),
						umcConfg.configuracionUMC.MARCO_SIZE);
				//La almaceno
				almacenarEnSwap(*idProcesoActual, paginaARemplazar,
						aAlmacenarEnSwap);
			}
			//Le doy el marco de la pagina reemplazada a la nueva pagina
			paginaBuscada->nroMarco = paginaAReemplazar->nroMarco;
			paginaAReemplazar->nroMarco = -1;
			paginaAReemplazar->modif = 0;
			paginaAReemplazar->bit_uso = 0;
			//saco la pagina actual de la lista de clock ya q no la tengo en memoria y agrego la pagina memoria
			int anterior = (elem->puntero) - 1;
			relojElem* temp = list_get((elem->paginasMemoria), anterior);
			temp->marco = paginaBuscada;
			temp->pag = pagina;
			//saco de tlb la pagina si estuviera
			removerDeTLB(*idProcesoActual, paginaARemplazar, -1);
			aAlmacenarEnSwap = solicitarEnSwap(*idProcesoActual, pagina);
			memcpy((memoriaPrincipal + posicionDeMemoria), aAlmacenarEnSwap,
					umcConfg.configuracionUMC.MARCO_SIZE);
			free(aAlmacenarEnSwap);
		}
		insertarEnTLB(*idProcesoActual, pagina, paginaBuscada, 0);
	}
	posicionDeMemoria = ((paginaBuscada->nroMarco)
			* umcConfg.configuracionUMC.MARCO_SIZE) + offset;
	memcpy((memoriaPrincipal + posicionDeMemoria), buffer, tamanio);
	paginaBuscada->bit_uso = USADO;
	paginaBuscada->modif = 1;
	//log_trace(log_memoria,"Lock off");
	pthread_mutex_unlock(&semaforoMemoria);
	return 1;
}

void cambiarProceso(int idProceso){
	//Esto es de la entrega 2
		//log_trace(log_memoria,"Lock on");
		pthread_mutex_lock(&semaforoMemoria);
		*idProcesoActual = idProceso;
		tabla_actual = dictionary_get(programas_ejecucion,&idProceso);
		log_trace(log_memoria,"CambiarProceso - idAnterior: %d , idNuevo: %d",*idProcesoActual,idProceso);
		return;
}













































