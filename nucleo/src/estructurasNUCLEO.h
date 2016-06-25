/*
 * estructuras.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef ESTRUCTURASNUCLEO_H_
#define ESTRUCTURASNUCLEO_H_

#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <semaphore.h>

typedef struct {
	int puerto_prog;
	int puerto_cpu;
	int quantum;
	int quantum_sleep;			// Estructura del archivo de configuracion
	t_dictionary* dic_IO;		 //clave nombre del dispositivo y datos la estructura t_datos_dicIO
	t_dictionary* dic_semaforos; //clave nombre del Semaforo y datos el int del valor
	t_dictionary* dic_variables; //clave el nombre de la variable y datos el int del valor
//	char ** io_id;
//	char ** io_sleep;
//	char ** sem_id;
//	char ** sem_init;
//	char ** shared_vars;
	unsigned int stack_size;
	char * ip_umc;
	int puerto_umc;
} t_reg_config;

typedef struct{
	int retardo;
	sem_t sem_dispositivo;
	t_list* cola_procesos;
}t_datos_dicIO;

typedef struct{
	int valor;
	t_list* cola_procesos;
}t_datos_samaforos;

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

typedef struct{
	int PID;
	char* instrucciones;
}programaNoCargado;

typedef struct{
	t_list* args; //tiene direcciones de memoria
	t_list* vars; //tiene direcciones de stack
	int* pos_ret;
	direccionMemoria* memoriaRetorno;
}stack;

typedef struct{
 char* funcion;
 int* posicion_codigo;
}funcion_sisop;

typedef struct {
    int socket_dest;
    char* mensaje;
}t_sock_mje;

typedef struct {
	int tamanioNombreFuncion;
	int posicionPID;
}__attribute__((packed))
funcionTemp;

typedef struct{
	char id;
	direccionMemoria lugarUMC;
}__attribute__((packed))
direccionStack;

typedef struct {
	pcb_t* pcb_bloqueado;
    int tipo_de_bloqueo;
    char * dispositivo;
    int unidades;
}t_pcb_bloqueado;

#endif /* ESTRUCTURASNUCLEO_H_ */
