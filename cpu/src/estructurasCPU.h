/*
 * estructuras.h
 *
 *  Created on: 16/5/2016
 *      Author: utnso
 */

#ifndef ESTRUCTURASCPU_H_
#define ESTRUCTURASCPU_H_
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>

#define ioSolID 1000
#define signalID 2000
#define waitID 3000
#define finalID 4000

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
	int* PCI;
	int* SP;
	int* paginasDisponible;
	t_dictionary* indice_codigo;
	t_list* indice_funciones;
	t_dictionary* indice_stack;
}pcb_t;

typedef struct{
 char* funcion;
 int* posicion_codigo;
}funcion_sisop;

typedef struct{
	int pagina;
	int offset;
	int tamanio;
}__attribute__((packed))
direccionMemoria;

typedef struct{
	t_list* args; //tiene direcciones de memoria
	t_list* vars; //tiene direcciones de stack
	int* pos_ret;
	direccionMemoria* memoriaRetorno;
}stack;

typedef struct{
	int puertoUMC;
	int puertoNucleo;
	char* IPUMC;
	char* IPNucleo;
}t_reg_config;



typedef struct{
	char id;
	direccionMemoria lugarUMC;
}__attribute__((packed))
direccionStack;

typedef struct{
	int PID;
	int PC;
	int PCI;
	int SP;
	int paginasDisponible;
	int tamanioIndiceCodigo;
	int tamanioStack;
	int tamanioIndiceDeFunciones;
}__attribute__((packed))
serializablePCB;


#endif /* ESTRUCTURASCPU_H_ */
