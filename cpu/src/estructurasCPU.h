/*
 * estructuras.h
 *
 *  Created on: 16/5/2016
 *      Author: utnso
 */

#ifndef ESTRUCTURASCPU_H_
#define ESTRUCTURASCPU_H_
#include <commons/collections/dictionary.h>

typedef struct {
	int pagina;
	int offset;
	int tamanio;
	int valor;
}__attribute__((packed))
almUMC;

typedef struct{
	int* PID;
	int* PC;
	int* SP;
	int* paginasDisponible;
	t_dictionary* indice_codigo;
}pcb_t;


typedef struct{
	int pagina;
	int offset;
	int tamanio;
}__attribute__((packed))
direccionMemoria;

typedef struct{
	int PID;
	int PC;
	int SP;
	int paginasDisponible;
	int tamanioIC;
}__attribute__((packed))
serializablePCB;


#endif /* ESTRUCTURASCPU_H_ */
