/*
 * procesosSwap.c
 *
 *  Created on: 18/6/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sockets/socketServer.h>
#include <sockets/basicFunciones.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "swapEstrucuturas.h"

extern proceso* listaSwap;
extern int *bitMap;
extern t_reg_config swap_configuracion;

proceso* crearProceso(int pid, int cantidadDePaginas) {
	proceso* proces = malloc(sizeof(proceso));
	proces->pid = pid;
	proces->cantidadDePaginas = cantidadDePaginas;

	return proces;
}

void eliminarProceso(int pid) {
	proceso* auxiliar1 = listaSwap->procesoSiguiente;
	proceso* auxiliar2 = listaSwap;

	while (auxiliar1 != NULL) {
		if (listaSwap->pid == pid) {
			eliminarDelBitMapLasPaginasDelProceso(listaSwap);
			listaSwap = listaSwap->procesoSiguiente;
		} else {
			if (auxiliar1->pid == pid) {
				auxiliar2->procesoSiguiente = auxiliar1->procesoSiguiente;
				eliminarDelBitMapLasPaginasDelProceso(auxiliar1);
			} else {
				auxiliar2 = auxiliar1;
				auxiliar1 = auxiliar2->procesoSiguiente;
			}

		}
	}
}

int procesoSeEncuentraEnSwap(int pid) {
	proceso* auxiliar = listaSwap;
	while (auxiliar != NULL) {

		if (auxiliar->pid == pid) {
			return 1;
		} else {
			auxiliar = auxiliar->procesoSiguiente;
		}

		return 0;
	}
}

proceso obtenerProceso(int pid) {

	proceso* auxiliar = listaSwap;

	while (auxiliar != NULL) {

		if (auxiliar->pid == pid) {

			return *auxiliar;

		} else {

			auxiliar = auxiliar->procesoSiguiente;

		}
	}
}

int entraProceso(proceso proceso) {

	int paginasLibres = 0;
	int pag = 0;
	do {
		if (bitMap[pag] == 0)
			paginasLibres++;
		if (paginasLibres >= proceso.cantidadDePaginas) {
			return 1;
		}
		pag++;
	} while (pag < (swap_configuracion.CANTIDAD_PAGINAS));
	return 0;
	/*for ( ; pag < (swap_configuracion.CANTIDAD_PAGINAS); pag++) {
	 if(bitMap[pag]==0) paginasLibres++;
	 }

	 if(paginasLibres >= proceso.cantidadDePaginas){
	 return 1;
	 }else{
	 return 0;
	 }*/
}

void insertarProceso(proceso* proceso) {
	int comienzoDelHueco;
	comienzoDelHueco = hayHuecoDondeCabeProceso(proceso);
	if (comienzoDelHueco >= 0) {
		proceso->comienzo = comienzoDelHueco;
		actualizarBitMap(proceso, comienzoDelHueco);
		agregarProcesoAListaSwap(proceso);
	} else {
		compactar();
		comienzoDelHueco = hayHuecoDondeCabeProceso(proceso);
		proceso->comienzo = comienzoDelHueco;
		actualizarBitMap(proceso, comienzoDelHueco);
		agregarProcesoAListaSwap(proceso);
	}

}

//Se fija si hay un hueco, si lo hay devuelve donde comienza
int hayHuecoDondeCabeProceso(proceso* proceso) {

	int paginaActual = 0;
	int ultimaPagina = swap_configuracion.CANTIDAD_PAGINAS;
	int paginasLibresConsecutivas = 0;

	while (paginaActual <= ultimaPagina) {

		if (bitMap[paginaActual] == 0) {
			paginasLibresConsecutivas++;
			paginaActual++;

			if (proceso->cantidadDePaginas == paginasLibresConsecutivas) {
				return (paginaActual - proceso->cantidadDePaginas);
			} else {
			}

		} else {
			paginasLibresConsecutivas = 0;
			paginaActual++;
		}

	}

	if (paginasLibresConsecutivas == 0)
		return -1;
}

void moverAPrimeraPosicionProceso(void) {
	proceso* procesoAMover = listaSwap;
	moverPaginas(procesoAMover, 0);
	eliminarDelBitMapLasPaginasDelProceso(procesoAMover);
	procesoAMover->comienzo = 0;
	actualizarBitMap(procesoAMover, 0);
}

void unirProcesos(proceso* procesoAnterior, proceso* procesoAJuntar) {
	int nuevoComienzo = procesoAnterior->comienzo + 1;
	moverPaginas(procesoAJuntar, nuevoComienzo);
	eliminarDelBitMapLasPaginasDelProceso(procesoAJuntar);
	procesoAJuntar->comienzo = nuevoComienzo;
	actualizarBitMap(procesoAJuntar, nuevoComienzo);

}

void agregarProcesoAListaSwap(proceso* procesoAInsertar) {
	proceso* auxiliar1 = listaSwap;
	proceso* auxiliar2 = NULL;
	while (auxiliar1 != NULL
			&& (auxiliar1->comienzo < procesoAInsertar->comienzo)) {
		auxiliar2 = auxiliar1;
		auxiliar1 = auxiliar2->procesoSiguiente;
	}
	if (listaSwap == NULL) {
		listaSwap = procesoAInsertar;
		procesoAInsertar->procesoSiguiente = NULL;
	} else {

		if (procesoAInsertar->comienzo == 0) {

			procesoAInsertar->procesoSiguiente = listaSwap;
			listaSwap = procesoAInsertar;

		} else {

			auxiliar2->procesoSiguiente = procesoAInsertar;
			procesoAInsertar->procesoSiguiente = auxiliar1;

		}
	}

}
