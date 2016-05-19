/*
 * estructuras.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef ESTRUCTURASNUCLEO_H_
#define ESTRUCTURASNUCLEO_H_

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
//No cambiar orden, si se cambia el orden, cambiar el recive de cpu
typedef struct{
	int id;
	int PID;
	int PC;
	int SP;
	int paginasDisponibles;
	t_dictionary* indicie_codigo;
}PCB;

typedef struct{
	int id;
	int ip;
	int sp;
	int paginasDisponible;
	int tamanioIC;
}__attribute__((packed))
serializablePCB;

typedef struct{
	int pagina;
	int offset;
	int tamanio;
}__attribute__((packed))
direccionMemoria;

typedef struct{
	int id;
	int pagnias;
}__attribute__((packed))
programa_id;

#endif /* ESTRUCTURASNUCLEO_H_ */
