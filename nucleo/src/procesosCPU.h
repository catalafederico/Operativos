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

#endif /* PROCESOSCPU_H_ */
