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
#include "umcMemoria.h"
#include <pthread.h>

void* memoriaPrincipal;
extern umcNucleo umcConfg;
t_log* log_memoria;

//Menajo de la memoria
t_list* marcosLibres;
t_dictionary* tabla_actual;
int* idProcesoActual;
t_dictionary* programas_ejecucion;
pthread_mutex_t semaforoMemoria;


//-------- comienzo esto es para el algoritmo lru de la tlb

 //tlb tablaPag[10];//seria la tlb con la cant de entradas del arch configuracion
//hay que inicializar las paginas en -1
int tablaEstaLlena(tlb tablaPag[],int cantEntradas){
	int i;
	for( i=0;i<=cantEntradas;i++){
		if (tablaPag[i].pag==-1){
			return 1;
		}
	}
	return 0;
}
void correrUnoAbajo(tlb tablaPag[],int pos){
	tlb aux;
	aux = tablaPag[pos-1];
	tablaPag[pos]=aux;
	pos--;
}
void actualizarTablaPqEncontre(tlb tablaPag[],int i){
	tlb ptr;
	//me guardo el contenido de la posicion en donde esta lo que necesito
	ptr = tablaPag[i];
	while(i>=0){
		if(i==0){
			tablaPag[0]=ptr;
		}
		else{
		correrUnoAbajo(tablaPag,i);
		}
	}

}

void actualizarTablaPqElimineUlt(tlb tablaPag[],int cantEntradas,int* pagina){
	tlb* aux;
	int posEliminada=cantEntradas;
	while(posEliminada>=0){
			if(posEliminada==0){
			//faltaria el marco
				tablaPag[0].pag=&pagina;
				tablaPag[0].idProg=&idProcesoActual;
			}
			else{
			correrUnoAbajo(tablaPag,posEliminada);
			}

}
void actualizarPqNoEncontreYTablaNoLlena(tlb tablaPag[],int* pagina){
	int i=0;
	while(tablaPag[i].pag != -1){
				i++;
	}
			correrUnoAbajo(tablaPag,i);
			//a la primer posicion le asigno lo que deberia recibir, faltaria marco
			tablaPag[0].pag=&pagina;
			tablaPag[0].idProg=&idProcesoActual;
}

	//podria recibir lo que quiero agregar faltaria marco
void actualizarTablaPqNoEncontre(tlb tablaPag[],int cantEntradas,int* pagina){
	if(tablaEstaLlena(tablaPag,cantEntradas)){

		//le paso la posicion que elimine y podria pasar lo que quiero actualizar corro todos uno hacia abajo
		actualizarTablaPqElimineUlt(tablaPag,cantEntradas,pagina);
	}
	else{

		actualizarPqNoEncontreYTablaNoLlena(tablaPag,pagina);
	}
}

int buscarPagina(tlb tablaPag[],int cantEntradas,int* pagina){
int i;
	for( i=0;(tablaPag[i].pag != &pagina) & (tablaPag[i].idProg != &idProcesoActual) & (i<=cantEntradas);i++)
	{
		if ((tablaPag[i].pag == &pagina) & (tablaPag[i].idProg == &idProcesoActual))
		{
			actualizarTablaPqEncontre(tablaPag,i);
			return tablaPag[i].marco;
		}
	}
	return -1;
}

//-------------------- fin del algoritmo lru para tlb

void* inicializarMemoria(t_reg_config* configuracionUMC){

//    int cantEntradas= umcConfg.configuracionUMC.ENTRADAS_TLB;
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
		int* tempMarcoNro = malloc(sizeof(int));
		*tempMarcoNro = i;
		list_add(marcosLibres,tempMarcoNro);
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
		int* idProceso = malloc(sizeof(int));
		*idProceso = id_proceso;
		int i = 1;
		t_dictionary* pag_frame = dictionary_create();
		pthread_mutex_lock(&semaforoMemoria);
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
		*idProcesoActual = id_proceso;
		pthread_mutex_unlock(&semaforoMemoria);
		dictionary_put(programas_ejecucion,idProceso,pag_frame);//habria que poner en 1 a los marcos de las pag asignadas
		// faltaria saber si ya no se pueden asignar mas paginas si esto no se puede hay que desalojar con el algoritmo clock
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
	pthread_mutex_lock(&semaforoMemoria);
	log_trace(log_memoria,"Agregado a marcos libres:");
	for(i=0;i <cant_paginas; i++){
		int* marcoLibre = dictionary_remove(tabla_desalojar,&i);
		list_add(marcosLibres,marcoLibre);
		log_trace(log_memoria,"Pag: %d \tMarco: %d ",i,*marcoLibre);
	}
	tabla_actual = NULL;
	pthread_mutex_unlock(&semaforoMemoria);
	dictionary_destroy(tabla_desalojar);
	int* idRemovido = dictionary_remove(programas_ejecucion,&id);
	free(idRemovido);
	log_trace(log_memoria,"Programa desalojado");
	return 0;
}


void* obtenerBytesMemoria(int pagina,int offset,int tamanio){
	solicitarEnSwap(*idProcesoActual,pagina);


	//Esto es mas de la entrega 3
	log_trace(log_memoria,"Solcitud - id: %d pag: %d offset: %d tamanio: %d",*idProcesoActual,pagina,offset,tamanio);
	void* obtenido = malloc(tamanio);
	int* marco = dictionary_get(tabla_actual,&pagina);
	int posicionDeMemoria = ((*marco)*umcConfg.configuracionUMC.MARCO_SIZE) + offset;
	pthread_mutex_lock(&semaforoMemoria);
	memcpy(obtenido,(memoriaPrincipal + posicionDeMemoria),tamanio);
	pthread_mutex_unlock(&semaforoMemoria);
	return obtenido;
}

void almacenarBytes(int pagina, int offset, int tamanio, void* buffer){
	//Esto es de la entrega 2
	log_trace(log_memoria,"Almacenar - id: %d pag: %d offset: %d tamanio: %d",*idProcesoActual,pagina,offset,tamanio);
	//Esto es mas de la entrega 3
	int* marco = dictionary_get(tabla_actual,&pagina);
	int posicionDeMemoria = ((*marco)*umcConfg.configuracionUMC.MARCO_SIZE) + offset;
	pthread_mutex_lock(&semaforoMemoria);
	memcpy((memoriaPrincipal+posicionDeMemoria),buffer,tamanio);
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
		*idProcesoActual = idProceso;
		log_trace(log_memoria,"CambiarProceso - idAnterior: %d , idNuevo: %d",*idProcesoActual,idProceso);
		//Esto es mas de la entrega 3
		/*
		 * pthread_mutex_lock(semaforoMemoria);
		tabla_actual = dictionary_get(programas_ejecucion,(char*)&idProceso);
		*idProcesoActual = idProceso;
		pthread_mutex_unlock(semaforoMemoria);
		*/

}














































































