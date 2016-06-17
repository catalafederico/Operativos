/*
 * umcTlb.c
 *
 *  Created on: 10/6/2016
 *      Author: utnso
 */

//-------- comienzo esto es para el algoritmo lru de la tlb

 //tlb tablaPag[10];//seria la tlb con la cant de entradas del arch configuracion
//hay que inicializar las paginas en -1

#include "archivoConf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include "estructurasUMC.h"
#include <pthread.h>
#include "umcTlb.h"

//int buscarPaginaTLB(tlb tablaPag[],int cantEntradas,int* pagina);
tlb* removerDeTLB(int id, int pagina, int pos);

int cantidadDeEntradas;
t_list* tlb_tabla;

void inicializarTLB(t_list* tlb, int cant){
	tlb_tabla = tlb;
	cantidadDeEntradas = cant;
}

int tlbLLena(){
	return list_size(tlb_tabla)==cantidadDeEntradas;
}

int buscarPosicionEnTLB(int id, int pagina){
	int i;
	for(i=0;i<cantidadDeEntradas;i++){
		tlb* temp = list_get(tlb_tabla,i);
		if(temp->idProg == id && temp->pag == pagina){
			return i;
		}
	}
	return -1;
}

infoPagina* buscarFrameEnTLB(int id, int pagina){
	int i;
	for(i=0;i<cantidadDeEntradas;i++){
		tlb* temp = list_get(tlb_tabla,i);
		if(temp == NULL)
			return NULL;
		if(temp->idProg == id && temp->pag == pagina){
			//TLB HIT
			tlb* removido = removerDeTLB(-1,-1,i);
			insertarEnTLB(id,pagina,removido->marco,0);
			return temp->marco;
		}
	}
	return NULL;
}

int insertarEnTLB(int id, int pagina, infoPagina* marco, int posicion) {
	tlb* e_tlb = malloc(sizeof(tlb));
	e_tlb->idProg = id;
	e_tlb->pag = pagina;
	e_tlb->marco = marco;
	if(posicion >= cantidadDeEntradas){
		free(e_tlb);
		return -1;
	}
	else if (!puedeInsertar()) {
		list_remove(tlb_tabla,cantidadDeEntradas-1);
		list_add_in_index(tlb_tabla, posicion, e_tlb);
		return 0;
	} else {
		list_add_in_index(tlb_tabla, posicion, e_tlb);
		return 0;
	}
}

//Pos poner en -1 si quiero remover por id y pagina
tlb* removerDeTLB(int id, int pagina, int pos){
	tlb* temp = NULL;
	if(pos!=-1){
		temp =  list_remove(tlb_tabla,pos);
	}else if(pos==-1){
		pos = buscarPosicionEnTLB(id,pagina);
		if(pos!=-1){
			temp = list_remove(tlb_tabla,pos);
		}
	}
	return temp;
}

int puedeInsertar(){
	return list_size(tlb_tabla)+1 <= cantidadDeEntradas;
}


