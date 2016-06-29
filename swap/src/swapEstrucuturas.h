/*
 * swapEstrucuturas.h
 *
 *  Created on: 18/6/2016
 *      Author: utnso
 */

#ifndef SWAPESTRUCUTURAS_H_
#define SWAPESTRUCUTURAS_H_

// Estructuras

typedef struct {
	int PUERTO_ESCUCHA;
	char* NOMBRE_SWAP;
	int CANTIDAD_PAGINAS;
	int TAMANIO_PAGINA;
	int RETARDO_ACCESO;
	int RETARDO_COMPACTACION;
} t_reg_config;

typedef struct {
	int pid;
	int cantidadDePaginas;
	int comienzo;
	struct proceso* procesoSiguiente;
} proceso;

// Funciones

#endif /* SWAPESTRUCUTURAS_H_ */
