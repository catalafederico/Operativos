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
//-------comienzo de manejo de lista circular para algoritmo clock
/* primero busco la pagina que me piden
 * buscar la pagina:
 -si encuentro la pagina entonces ese elemento deberia tener el bit de uso en 1 si no lo tiene deberia ponerlo en
  1 y debo retornar  el puntero de lista donde lo recibi, no deberia actualizarlo
 -si no lo encuentro deberia agregar la pagina para eso retornaria un NULL y lo agarraria para llamar a (actualizarlista)
 *actualizar lista
  -si lalista no esta llena utilizo (agregarpagina)
  -si la lista esta llena  recorro y avanzo el puntero buscando el primero con bit de uso en 0 y mientras avanzo pongo a los bit
  de uso que estan en 1 en 0, cuando lo encuentro (reemplazomarco)
  *reemplazomarco asigna el valor que recibe al marco al cual apunta el puntero que recibe, pone su bit de estado en 1
  y retorna el puntero que recibe avanzado un elemento (en el siguiente del puntero que recibio)
  *agregarpagina agregaria un nuevo marco(es un elemento de la lista) a la lista con el numero de pagina que recibe y avanza
   el puntero a la siguiente posicion donde agrego*/

t_link_element* pasarDelUltAlPrimero(t_list* lista, t_link_element* ptr){
	ptr=lista->head;
	return ptr;
}
int listaEstaLlena(t_list lista,int cantMaxElementos){
	if(lista.elements_count == cantMaxElementos){
		return 1;//la lista esta llena
	}
	return 0;// la lista no esta llena
}
t_link_element* avanzarPuntero(t_list lista,int cantMaxElementos,t_link_element* ptr){
	if(ptr->next == NULL & lista.elements_count==cantMaxElementos){ //estoy en el ultimo elemento
		ptr=lista.head;//le asigno el primer elemento de la lista
	}
	else{
		ptr=ptr->next;
	}
	return ptr;
}
t_link_element* buscarPagina(t_list lista,t_link_element* ptr, int cantMaxElementos,int* pag){
// no uso listfind pq retorna el valor que cumple y necesito retornar un puntero
//comienzo del principio
	t_link_element* aux;
	aux= lista.head;//estoy en el primer elemento de la lista con aux
	while(aux->next != NULL & aux->next->data != &pag)// si es null deberia preguntar la cant de elementos, y no encuentre la pagina
	{
		aux= aux->next;
	}
	//aca no encuentro la pagina
	if(aux->next==NULL & aux->next->data != &pag){
		//pregunto si todavia  lllego a su maxima capacidad la lista
		if(listaEstaLlena(lista,cantMaxElementos)){
			//solo puedo reemplazar el marco pq no tengo mas espacio
			//reemplazarMarco(lista,ptr,cantMaxElementos,pag);
			return NULL;//no encontre
		}
	}
// lo encontre
	else{
		//deberia checkear el bit de uso
		return ptr;// ya estaba la pagina referenciada debo devolver elmismo puntero que recibi
	}
t_link_element* actualizarLista(t_list lista,t_link_element* ptr, int cantMaxElementos,int* pag){
//si la lista no esta llena
	if(!listaEstaLlena(lista,cantMaxElementos)){
agregarPagina(lista,ptr,pag);// el puntero esta apuntando al ult elemento de la lista pero todavia tiene capacidad la lista
//acordarme de cuando agrego el ultimo elemento el puntero o tengo que llevar al primer elemento (avanzarpuntero)
}

// aca deberia recorrer desde la pos del puntero buscando el primer bit de uso en 0  y modificando los
	// que vaya pasando poniendolo  en 1 acordarse que cuando llego al ultimo debo retornar al primero
}
t_link_element* reemplazoMarco( t_list lista,t_link_element* ptr,int cantMaxElementos, int* pag){//ptr apunta al nodo que debo cambiar
	ptr->data=&pag;
	//falta poner bit de estado de uso de pagina en 1
	//avanzo el puntero, la funcion avanza el puntero y si es el ult elemento al puntero lo coloca primero
	 avanzarPuntero(lista,cantMaxElementos,ptr);
}
t_link_element* agregarPagina( t_list lista,t_link_element* ptr,int cantMaxElementos, int* pag){// la lista no esta llena list add agrega al final
//deberia funcionar
	t_list* punteroALista;
	punteroALista=&lista;
    list_add(punteroALista,(void *)&pag);
    avanzarPuntero(lista,cantMaxElementos,ptr);

}

//---------fin
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














































































