/*
 * procesosSwap.h
 *
 *  Created on: 18/6/2016
 *      Author: utnso
 */

#ifndef PROCESOSSWAP_H_
#define PROCESOSSWAP_H_

proceso* crearProceso(int pid, int cantidadDePaginas);
void eliminarProceso(int pid);
int procesoSeEncuentraEnSwap(int pid);
proceso obtenerProceso(int pid);
int entraProceso(proceso proceso);
void insertarProceso(proceso* proceso);
int hayHuecoDondeCabeProceso(proceso* proceso);
void moverAPrimeraPosicionProceso(void);
void unirProcesos(proceso* procesoAnterior, proceso* procesoAJuntar);
void agregarProcesoAListaSwap(proceso* procesoAInsertar);

#endif /* PROCESOSSWAP_H_ */
