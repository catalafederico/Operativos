/*
 * umcClockV2.c
 *
 *  Created on: 17/6/2016
 *      Author: utnso
 */
#include <commons/collections/list.h>
#include "estructurasUMC.h"
extern int clockModificado;
int algoritmoClock(t_list* lista_pag_mry,int* elemento);
int algoritmoClockModificado(t_list* lista_pag_mry, int* elemento);

int buscarPagAReemplazar(t_list* lista_pag_mry,int* elemento){
		if(clockModificado){
			return algoritmoClockModificado(lista_pag_mry,elemento);
		}
		else{
			return algoritmoClock(lista_pag_mry,elemento);
		}
}

int algoritmoClock(t_list* lista_pag_mry,int* elemento){
	do {
		if(*elemento>=list_size(lista_pag_mry)){
			*elemento =0;
		}
		relojElem* temp =list_get(lista_pag_mry,*elemento);
		if (temp->marco->bit_uso == 0) {
			*elemento = *elemento + 1;
			return temp->pag;
		} else {
			temp->marco->bit_uso = 0;
			*elemento = *elemento + 1;
		}
	} while (1);
}


int algoritmoClockModificado(t_list* lista_pag_mry, int* elemento) {
	int nroVuelta = 1;
	int tamanioLista = list_size(lista_pag_mry);
	int i = 0;
	relojElem* temp;
	if(*elemento>=list_size(lista_pag_mry)){
		*elemento =0;
	}
	while (1) {
		switch (nroVuelta) {
		case 1:
			//BUsco pag con 0,0
			for (i = 0; i < tamanioLista; i++) {
				temp = list_get(lista_pag_mry, *elemento);
				if (temp->marco->bit_uso == 0 && temp->marco->modif == 0) {
					*elemento = *elemento + 1;
					return temp->pag;
				}
				*elemento = *elemento + 1;
				if(*elemento>=list_size(lista_pag_mry)){
					*elemento =0;
				}
			}
			nroVuelta++;
			break;
		case 2:
			//BUsco pag con 0,1
			for (i = 0; i < tamanioLista; i++) {
				temp = list_get(lista_pag_mry, *elemento);
				if (temp->marco->bit_uso == 0 && temp->marco->modif == 1) {
					*elemento = *elemento + 1;
					return temp->pag;
				} else {
					temp->marco->bit_uso = 0;
					*elemento = *elemento + 1;
				}
				if(*elemento>=list_size(lista_pag_mry)){
					*elemento =0;
				}
			}
			nroVuelta--;
			break;
		}
	}
}
