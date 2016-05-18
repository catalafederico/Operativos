/*
 * estructuras.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <commons/collections/dictionary.h>

typedef struct {
	int puerto_prog;
	int puerto_cpu;
	int quantum;
	int quantum_sleep;			// Estructura del archivo de configuracion
	char ** io_id;
	int * io_sleep;
	char ** sem_id;
	int * sem_init;
	char ** shared_vars;
} t_reg_config;

//Pcb
typedef struct{
	t_dictionary* inst_tamanio;
}indiceCodigo;

//indice de etiquetas declaro en main
// indice del stack
typedef struct{
	struct t_list* args;
	struct l_list* vars;
	int retPos;
	int* retVar;
}indiceStack;
//creo PCB
typedef struct{
	int PID;
	int PC;
	int SP;
}PCB;


#endif /* ESTRUCTURAS_H_ */
