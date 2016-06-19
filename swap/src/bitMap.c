/*
 * bitMap.c
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

extern int *bitMap;
extern t_reg_config swap_configuracion;
extern t_log* logguerSwap;
//--Funciones del BitMap
void inicializarBitMap() {
	int pag;
	int cantidadPaginas = swap_configuracion.CANTIDAD_PAGINAS;
	bitMap = malloc(cantidadPaginas * sizeof(int));
	for (pag = 0; pag < cantidadPaginas; pag++) {
		bitMap[pag] = 0;
	}
	log_info(logguerSwap,"Bit map inicializado con: %d " , cantidadPaginas);
}

void eliminarDelBitMapLasPaginasDelProceso(proceso* proceso) {
	int paginasActualizadas = 0;
	int comienzo = proceso->comienzo;
	while(paginasActualizadas < proceso->cantidadDePaginas) {
		bitMap[comienzo] = 0;
		log_info(logguerSwap,"Bit Map cambiado,proceso: %d, nro: %d valor: %d ",proceso->pid,comienzo,0);
		comienzo++;
		paginasActualizadas++;
	}

}

void actualizarBitMap(proceso* proceso, int comienzoDelHueco) {
	int paginasActualizadas = 0;
	while (paginasActualizadas < proceso->cantidadDePaginas) {
		bitMap[comienzoDelHueco] = 1;
		log_info(logguerSwap,"Bit Map cambiado,proceso: %d, nro: %d valor: %d ",proceso->pid,comienzoDelHueco,1);
		comienzoDelHueco++;
		paginasActualizadas++;
	}
	//listarBitMap();
}

void listarBitMap() {
	int pag;
	int cantidadPaginas = swap_configuracion.CANTIDAD_PAGINAS;
	for (pag=0; pag < cantidadPaginas; pag++) {
		printf("Pagina %d: %d \n", pag, bitMap[pag]);
	}
}

