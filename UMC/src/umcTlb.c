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

extern int* idProcesoActual;

int tablaEstaLlena(tlb tablaPag[],int cantEntradas){
	int i;
	for( i=0;i<=cantEntradas;i++){
		if (tablaPag[i].pag==-1){
			return 1;
		}
	}
	return 0;// verdadero esta llena
}
void correrUnoAbajo(tlb tablaPag[],int pos){// corro todos los elementos uno hacia abajo
	tlb aux;
	aux = tablaPag[pos-1];
	tablaPag[pos]=aux;
	pos--;
}
void actualizarTablaPqEncontre(tlb tablaPag[],int i){
	tlb ptr;
	//me guardo el contenido de la posicion en donde esta lo que necesito
	ptr = tablaPag[i];
	//hasta uqe llego a la posicion 0 que es en donde coloco lo que recibo
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
	int posEliminada=cantEntradas;// voy a eliminar el ultimo posicion del vector para correr todo uno abajo y colocar
	//el nuevo valor
	//hasta que llegue a la primer posicion
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
	//busco la primer pagina en la tabla con valor -1 esa pagina esta disponible
	while(tablaPag[i].pag != -1){
				i++;
	}
			correrUnoAbajo(tablaPag,i);//corro todos uno hacia abajo hasta i pq voy a insertar en la posicion 0
			//faltaria marco
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
//Tuve q cambiar nombre xq estaba lo de clock  tlb con el mismo nombre y tiraba error
int buscarPaginaTLB(tlb tablaPag[],int cantEntradas,int* pagina){
	int i;
	for( i=0;(tablaPag[i].pag != *pagina) & (tablaPag[i].idProg != *idProcesoActual) & (i<=cantEntradas);i++)
	{
		if ((tablaPag[i].pag == *pagina) & (tablaPag[i].idProg == *idProcesoActual))
		{
			actualizarTablaPqEncontre(tablaPag,i);
			return tablaPag[i].marco;
		}
	}
	return -1;
}

//-------------------- fin del algoritmo lru para tlb


