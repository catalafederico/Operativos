/*
 * umcClock.c
 *
 *  Created on: 10/6/2016
 *      Author: utnso
 */

#include "archivoConf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include "estructurasUMC.h"
extern int clock;
//-------comienzo de manejo de lista circular para algoritmo clock


t_link_element* pasarDelUltAlPrimero(t_list* lista, t_link_element* ptr){
	ptr=lista->head;
	return ptr;
}

int listaEstaLlena(t_list* lista,int cantMaxElementos){
	if(lista->elements_count == cantMaxElementos){
		return 1;//la lista esta llena
	}
	return 0;// la lista no esta llena
}

t_link_element* avanzarPuntero(t_list* lista,int cantMaxElementos,t_link_element* ptr){
	if((ptr->next == NULL) & (lista->elements_count==cantMaxElementos)){ //estoy en el ultimo elemento
		ptr=lista.head;//le asigno el primer elemento de la lista
	}
	else{
		ptr=ptr->next;
	}
	return ptr;
}

t_link_element* buscarPaginaClk(t_list lista, t_link_element* ptr, int cantMaxElementos,frame* pag) {
	// no uso listfind pq retorna el valor que cumple y necesito retornar un puntero
	//comienzo del principio
	t_link_element* aux;
	aux = lista.head; //estoy en el primer elemento de la lista con aux
	while ((aux->next != NULL) & (aux->next->data != pag->nro)) // si es null deberia preguntar la cant de elementos, y no encuentre la pagina
	{
		aux = aux->next;
	}
	//aca no encuentro la pagina
	if ((aux->next == NULL) & (aux->next->data != &pag->nro)) {
		//pregunto si todavia  lllego a su maxima capacidad la lista
		if (listaEstaLlena(lista, cantMaxElementos)) {
			//solo puedo reemplazar el marco pq no tengo mas espacio
			//reemplazarMarco(lista,ptr,cantMaxElementos,pag);
			return NULL;			//no encontre
		}

	}
	// lo encontre
		frame* actual=ptr->data;
		actual->bit_uso=1;// deberia haberlo preguntado pero bueno lo pongo en uno de una
		return ptr;	// ya estaba la pagina referenciada debo devolver elmismo puntero que recibi

}

t_link_element* reemplazoMarco( t_list lista,t_link_element* ptr,int cantMaxElementos, frame* pag){//ptr apunta al nodo que debo cambiar
	t_link_element* puntero;
	frame* frameActual=ptr->data;
	//actualizo el frame actual
	frameActual->bit_uso = 1;//pongo el bit de uso en 1
	frameActual->nro = pag->nro;//asigno la nueva pagina
	//avanzo el puntero, la funcion avanza el puntero y si es el ult elemento al puntero lo coloca primero
	puntero= avanzarPuntero(lista,cantMaxElementos,ptr);
	return puntero;
}

t_link_element* agregarPagina(t_list lista,t_link_element* ptr,int cantMaxElementos, frame* pag){// la lista no esta llena list add agrega al final
	//deberia funcionar
	t_link_element* puntero;
	t_list* punteroALista;
	punteroALista=&lista;
	pag->bit_uso=1;//actualizo el bit de uso del frame antes de insertarlo a la lista
    list_add(punteroALista,(void *)&pag);
    puntero=avanzarPuntero(lista,cantMaxElementos,ptr);
    return puntero;

}
t_link_element* recorrerLstActualizandoBits(t_list lista, t_link_element* ptr, int cantMaxElementos, frame* pag){
	t_link_element* puntero;
	frame* frameActual =ptr->data;// frame en el cual estoy parado
	while(frameActual->bit_uso != 0){//lo pongo en 0
		frameActual->bit_uso =0;//actualizo
		avanzarPuntero(lista,cantMaxElementos,ptr);//avanzo puntero
		frameActual=ptr->data;//actualizo agarrado al valot que tiene el puntero qeu devolvio avanzarpuntero
	}
	//la unica forma de que haya salido es qeu encontre un bit de uso en 0 y tengo apuntado ese elemento con ptr
    //debo remplazar el marco
	puntero = reemplazoMarco(lista,ptr,cantMaxElementos,pag);
	return puntero;
}

t_link_element* actualizarLista(t_list lista, t_link_element* ptr,int cantMaxElementos, frame* pag) {
	t_link_element* puntero;
	//si la lista no esta llena
		if (!listaEstaLlena(lista, cantMaxElementos)) {
			puntero=agregarPagina(lista, ptr, cantMaxElementos ,pag);	// el puntero esta apuntando al ult elemento de la lista pero todavia tiene capacidad la lista
			//acordarme de cuando agrego el ultimo elemento el puntero o tengo que llevar al primer elemento (avanzarpuntero)
		}
		// aca deberia recorrer desde la pos del puntero buscando el primer bit de uso en 0  y modificando los
		// que vaya pasando poniendolo  en 1 acordarse que cuando llego al ultimo debo retornar al primero
		else{
			puntero=recorrerLstActualizandoBits(lista, ptr, cantMaxElementos,pag);
		}
		return puntero;
}
/*
t_link_element* reemplazoMarco( t_list lista,t_link_element* ptr,int cantMaxElementos, frame* pag){//ptr apunta al nodo que debo cambiar
	t_link_element* puntero;
	frame* frameActual=ptr->data;
	//actualizo el frame actual
	frameActual->bit_uso = 1;//pongo el bit de uso en 1
	frameActual->nro = pag->nro;//asigno la nueva pagina
	//avanzo el puntero, la funcion avanza el puntero y si es el ult elemento al puntero lo coloca primero
	 puntero=avanzarPuntero(lista,cantMaxElementos,ptr);
	 return puntero;
}*/

t_link_element* recorrerLstYActualizarClockMod(t_list lista, t_link_element* ptr, int cantMaxElementos, frame* pag){
	t_link_element* puntero;
	t_link_element* ptrAlSigDelRecibido;
	t_link_element* punteroRecibido;
	punteroRecibido=ptr;
	//paso 1 busco modificado en 0 y uso en 0
	ptrAlSigDelRecibido=ptr->next;
	frame* frameActual =ptr->data;// frame en el cual estoy parado
	while((frameActual->bit_uso != 0)&(frameActual->modif !=0)&(ptrAlSigDelRecibido!=ptr)){//la ult condicion es para que no se quede loopeando
		//si sali por la ultima condicion tengo que retroceder en 1 ptr
		avanzarPuntero(lista,cantMaxElementos,ptr);//avanzo puntero
		frameActual=ptr->data;//actualizo agarrado al valot que tiene el puntero qeu devolvio avanzarpuntero
		}
	if((frameActual->bit_uso == 0)&(frameActual->modif==0)){//si encontre un espacio vacio
	puntero=agregarPagina(lista,ptr,cantMaxElementos,pag);//agrego la pagina
	}

	//paso 2 busco bit modificado en 1 y bit de uso en 0, actualizando los bit de uso a 0
	if(ptrAlSigDelRecibido==ptr){//no encontre marco libre
		ptr=punteroRecibido;
		ptrAlSigDelRecibido=ptr->next;
		frameActual =ptr->data;// frame en el cual estoy parado
		while((frameActual->bit_uso != 0)&(frameActual->modif!= 1)&(ptrAlSigDelRecibido!=ptr)){//lo pongo en 0
			frameActual->bit_uso =0;//actualizo
			avanzarPuntero(lista,cantMaxElementos,ptr);//avanzo puntero
			frameActual=ptr->data;//actualizo agarrado al valot que tiene el puntero qeu devolvio avanzarpuntero
		}
		if((frameActual->bit_uso == 0)&(frameActual->modif==1)){//si encontre un espacio vacio
			puntero=reemplazoMarco(lista,ptr,cantMaxElementos,pag);//reemplazo el marco
			}
		//paso 3
		if(ptrAlSigDelRecibido==ptr){ //no encontre con modificado en 1 y bit de uso en 0,
			//ahora busco con modificado en 0 y bit de uso en 0
			ptr=punteroRecibido;
			frameActual=ptr->data;
			frame* frameActual =ptr->data;// frame en el cual estoy parado
			while((frameActual->bit_uso != 0)&(frameActual->modif !=0)){// aca si o si encuentro antes de volver al comienzo
					avanzarPuntero(lista,cantMaxElementos,ptr);//avanzo puntero
					frameActual=ptr->data;//actualizo agarrado al valot que tiene el puntero qeu devolvio avanzarpuntero
					}
				//si o si encuentro reemplazo el marco
				puntero=reemplazoMarco(lista,ptr,cantMaxElementos,pag);//agrego la pagina
				}
	return puntero;
}

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
t_link_element* UmcClock(t_list lista, t_link_element* ptr, int cantMaxElementos, frame* pag,clock){
	t_link_element* puntero;
	puntero=buscarPaginaClk(lista,ptr,cantMaxElementos,pag);//busco la pagina funciona para ambos algoritmos
	if (clock == 0){//algoritmo clock

	if(puntero==NULL){//no encontre
	puntero=actualizarLista(lista,ptr,cantMaxElementos,pag);
	}

}
	else{ // algoritmo clock modificado
	if(puntero==NULL){//no encontre
	puntero=recorrerLstYActualizarClockMod(lista,ptr,cantMaxElementos,pag);
		}
	}
	return puntero;
}



