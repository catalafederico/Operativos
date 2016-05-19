/*
 * estructuras.h
 *
 *  Created on: 16/5/2016
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_
#include <commons/collections/dictionary.h>

typedef struct {
	int pagina;
	int offset;
	int tamanio;
	int valor;
}__attribute__((packed))
almUMC;

typedef struct{
	int id;
	int ip;
	int sp;
	int paginasDisponible;
	t_dictionary* indiceDeCodigo;
}pcb;


typedef struct{
	int pagina;
	int offset;
	int tamanio;
}__attribute__((packed))
direccionMemoria;



#endif /* ESTRUCTURAS_H_ */
