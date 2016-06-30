/*
 * procesosCPU.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef PROCESOSCPU_H_
#define PROCESOSCPU_H_

void *atender_conexion_CPU();

void *atender_CPU(int* socket_desc);

void enviarPCB(pcb_t* pcb,int cpu, int quantum, int quantum_sleep);

int estado_proc_es_Ansisop(int estado_proceso);

pcb_t* recibirPCBdeCPU(int socket,int pidLocal);
pcb_t* buscarYRemoverPCBporPID(int pidBuscado,t_list* lista);

void ansisop_entradaSalida(int socket_local, int pid_local);
void ansisop_obtenerValorCompartida(int socket_local, int pid_local);
void ansisop_asignarValorCompartida(int socket_local, int pid_local);
int ansisop_wait (int socket_local, int pid_local);
void ansisop_signal(int socket_local, int pid_local);

#endif /* PROCESOSCPU_H_ */
