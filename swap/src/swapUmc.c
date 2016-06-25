/*
 * swapUmc.c
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
#include "procesosSwap.h"
#include "particionSwap.h"

extern t_reg_config swap_configuracion;

extern int socketAdministradorDeMemoria;

extern int *bitMap;

extern proceso* listaSwap;

extern pthread_mutex_t mutex;

extern t_log* logguerSwap;



void leer(void) {
	int* pid = recibirStream(socketAdministradorDeMemoria,sizeof(int));
	int* numeroPagina = recibirStream(socketAdministradorDeMemoria,sizeof(int));
	if (procesoSeEncuentraEnSwap(*pid)) {
		proceso proceso = obtenerProceso(*pid);
		if(*numeroPagina >= proceso.cantidadDePaginas) {
			printf(
					"La pagina %d no se encuentra en el proceso %d y no puede ser leida\n",
					*numeroPagina, *pid);
			int fracaso = 7;
			send(socketAdministradorDeMemoria, &fracaso, sizeof(int), 0);
		} else {
			int paginaALeer = proceso.comienzo + *numeroPagina;
			void* texto;
			texto = leerPagina(paginaALeer);
			int exito = 6;
			enviarStream(socketAdministradorDeMemoria,exito,swap_configuracion.TAMANIO_PAGINA,texto);
			free(texto);
		}
	} else {
		printf("Proceso %d no se encuentra en Swap y no puede ser leido\n",
				*pid);
		int fracaso = 7;
		send(socketAdministradorDeMemoria, &fracaso, sizeof(int), 0);
	}
	free(pid);
	free(numeroPagina);
}

void escribir() {
	int* pid = recibirStream(socketAdministradorDeMemoria,sizeof(int));
	int* numeroPagina = recibirStream(socketAdministradorDeMemoria,sizeof(int));
	void* texto = recibirStream(socketAdministradorDeMemoria,
			swap_configuracion.TAMANIO_PAGINA);
	if (procesoSeEncuentraEnSwap(*pid)) {
		proceso proceso = obtenerProceso(*pid);
		if (*numeroPagina >= proceso.cantidadDePaginas) {
			printf(
					"La pagina %d no se encuentra en el proceso %d y no puede ser escrita\n",
					*numeroPagina, *pid);
			int fracaso = 7;
			send(socketAdministradorDeMemoria, &fracaso, sizeof(int), 0);
		} else {
			int paginaAEscribir = proceso.comienzo + *numeroPagina;
			escribirPagina(paginaAEscribir, texto);
			int exito = 6;
			send(socketAdministradorDeMemoria, &exito, sizeof(int), 0);
		}
	} else {
		printf("Proceso %d no se encuentra en Swap y no puede ser escrito\n",
				*pid);
		int fracaso = 7;
		send(socketAdministradorDeMemoria, &fracaso, sizeof(int), 0);
	}
	free(texto);
	free(pid);
	free(numeroPagina);
}

void iniciar(void) {
	int* pid = recibirStream(socketAdministradorDeMemoria,sizeof(int));
	int* cantidadPaginas = recibirStream(socketAdministradorDeMemoria,sizeof(int));
	proceso* proceso = crearProceso(*pid, *cantidadPaginas);
	if (entraProceso(*proceso)) {
		//El proceso entra, realizar insercion
		pthread_mutex_lock(&mutex);
		insertarProceso(proceso);
		pthread_mutex_unlock(&mutex);
		//logIniciar(*proceso);
		int exito = 6;
		send(socketAdministradorDeMemoria, &exito, sizeof(int), 0);
	} else {
		//El proceso no entra, avisar rechazo
		//logRechazar(*proceso);
		printf(
				"No hay cantidad de paginas suficientes para alojar el proceso %d",
				proceso->pid);
		int fracaso = 7;
		send(socketAdministradorDeMemoria, &fracaso, sizeof(int), 0);
	}
	free(pid);
	free(cantidadPaginas);
}

void finalizar(void) {
	int* pid = recibirStream(socketAdministradorDeMemoria,sizeof(int));
	if (procesoSeEncuentraEnSwap(*pid)) {
		proceso procesoAEliminar = obtenerProceso(*pid);
		printf("Eliminando proceso\n");
		eliminarProceso(*pid);
		printf("Proceso eliminado\n");
		//Avisar a la UMC que se borro el proceso con exito
		int exito = 6;
		send(socketAdministradorDeMemoria, &exito, sizeof(int), 0);
	} else {
		printf("El proceso %d a finalizar no se encuentra en Swap\n", *pid);
		//Avisar a la UMC que se fracaso en el borrado del proceso
		int fracaso = 7;
		send(socketAdministradorDeMemoria, &fracaso, sizeof(int), 0);
	}
	free(pid);
}
