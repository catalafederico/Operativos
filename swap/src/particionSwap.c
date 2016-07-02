/*
 * particionSwap.c
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

extern t_reg_config swap_configuracion;
extern t_log* logguerSwap;
extern proceso* listaSwap;
void crearArchivo() {
	int tamanioComando = 80;
	char* datos = malloc(tamanioComando*sizeof(char));
	int resultado = snprintf(datos,tamanioComando,"dd if=/dev/zero of=%s bs=%d count=%d", swap_configuracion.NOMBRE_SWAP, swap_configuracion.TAMANIO_PAGINA, swap_configuracion.CANTIDAD_PAGINAS);
	if(resultado>tamanioComando){
		exit(-1);
	}
	//sprintf(datos,"dd if=/dev/zero of=%s bs=%d count=%d", swap_configuracion.NOMBRE_SWAP, swap_configuracion.TAMANIO_PAGINA, swap_configuracion.CANTIDAD_PAGINAS );
	//char* datos = strdup(("dd if=/dev/zero of=%s bs=%d count=%d", swap_configuracion.NOMBRE_SWAP, swap_configuracion.TAMANIO_PAGINA, swap_configuracion.CANTIDAD_PAGINAS));
	if(system(datos)==-1){
		perror("Error al crear el archivo:\n");
		exit(-1);
	}
	free(datos);
	log_info(logguerSwap,"Particion creado correctamente");
	//printf("El archivo se crea correctamente.\n");
}

void inicializarArchivo() {
	FILE* archivo = fopen(swap_configuracion.NOMBRE_SWAP, "r+");
	fseek(archivo, 0, SEEK_SET);
	char* texto = malloc(
			swap_configuracion.CANTIDAD_PAGINAS
					* swap_configuracion.TAMANIO_PAGINA);
	memset(texto, ' ',
			swap_configuracion.CANTIDAD_PAGINAS
					* swap_configuracion.TAMANIO_PAGINA);
	//fwrite(texto,sizeof(char),sizeof(texto), archivo );
	fprintf(archivo, "%s", texto);
	fclose(archivo);
	free(texto);

}

void escribirPagina(int paginaAEscribir, void* texto) {
	int tamanioPagina = swap_configuracion.TAMANIO_PAGINA;
	FILE* archivo;
	archivo = fopen(swap_configuracion.NOMBRE_SWAP, "rb+");
	if (archivo == NULL) {
		printf("ERROR\n");
	}
	if (fseek(archivo, tamanioPagina * paginaAEscribir, SEEK_SET) != 0) {
		printf("ERROR\n");
	}
	if (fwrite(texto, sizeof(char), tamanioPagina, archivo)
			!= swap_configuracion.TAMANIO_PAGINA) {
		printf("ERROR\n");
	}
	fclose(archivo);
}

void* leerPagina(int pagina) {
	FILE* archivo;
	int tamanioPagina = swap_configuracion.TAMANIO_PAGINA;
	archivo = fopen(swap_configuracion.NOMBRE_SWAP, "rb+");
	if (archivo == NULL) {
		printf("ERROR\n");
	}
	if (fseek(archivo, tamanioPagina * pagina, SEEK_SET) != 0) {
		printf("ERROR\n");
	}
	void* texto = malloc(tamanioPagina);
	fread(texto, tamanioPagina, 1, archivo);
	fclose(archivo);
	return texto;
}

void compactar(void) {
	printf("Ha comenzado la compactacion\n");
	dormir();
	proceso* auxiliar1 = listaSwap;
	proceso* auxiliar2 = listaSwap;
	if (auxiliar1->comienzo == 0) {
		auxiliar1 = auxiliar2->procesoSiguiente;
		while (auxiliar1 != NULL) {
			if ((auxiliar2->comienzo + auxiliar2->cantidadDePaginas) + 1
					== auxiliar1->comienzo) {
				auxiliar2 = auxiliar1;
				auxiliar1 = auxiliar2->procesoSiguiente;
			} else {
				unirProcesos(auxiliar2, auxiliar1);
				auxiliar2 = auxiliar1;
				auxiliar1 = auxiliar2->procesoSiguiente;
			}
		}

	} else {
		moverAPrimeraPosicionProceso(); //Mueve el proceso mas cercano a comienzo 0, a dicha posicion ( 0 )
		auxiliar1 = auxiliar2->procesoSiguiente;
		while (auxiliar1 != NULL) {
			if ((auxiliar2->comienzo + auxiliar2->cantidadDePaginas) + 1
					== auxiliar1->comienzo) {
				auxiliar2 = auxiliar1;
				auxiliar1 = auxiliar2->procesoSiguiente;
			} else {
				unirProcesos(auxiliar2, auxiliar1);
				auxiliar2 = auxiliar1;
				auxiliar1 = auxiliar2->procesoSiguiente;
			}
		}
	}
	printf("Ha finalizado la compactacion\n");
}

void moverPaginas(proceso* procesoAJuntar, int nuevoComienzo) {
	FILE* archivo;
	archivo = fopen(swap_configuracion.NOMBRE_SWAP, "rb+");
	int tamanioPagina = swap_configuracion.TAMANIO_PAGINA;
	fseek(archivo, tamanioPagina * (procesoAJuntar->comienzo), SEEK_SET);
	void* texto = malloc(tamanioPagina * (procesoAJuntar->cantidadDePaginas));
	fread(texto, tamanioPagina, procesoAJuntar->cantidadDePaginas, archivo);
	fseek(archivo, tamanioPagina * nuevoComienzo, SEEK_SET);
	fwrite(texto, sizeof(char), tamanioPagina, archivo);
	//fprintf(archivo,"%s",texto);
	fclose(archivo);
	free(texto);

}

