/*
 * umcTlb.h
 *
 *  Created on: 10/6/2016
 *      Author: utnso
 */

#ifndef SRC_UMCTLB_H_
#define SRC_UMCTLB_H_

int tablaEstaLlena(tlb tablaPag[],int cantEntradas);
void correrUnoAbajo(tlb tablaPag[],int pos);
void actualizarTablaPqEncontre(tlb tablaPag[],int i);
void actualizarTablaPqElimineUlt(tlb tablaPag[],int cantEntradas,int* pagina);
void actualizarPqNoEncontreYTablaNoLlena(tlb tablaPag[],int* pagina);
void actualizarTablaPqNoEncontre(tlb tablaPag[],int cantEntradas,int* pagina);
int buscarPaginaTLB(tlb tablaPag[],int cantEntradas,int* pagina);
#endif /* SRC_UMCTLB_H_ */
