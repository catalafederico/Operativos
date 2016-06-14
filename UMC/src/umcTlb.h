/*
 * umcTlb.h
 *
 *  Created on: 10/6/2016
 *      Author: utnso
 */

#ifndef SRC_UMCTLB_H_
#define SRC_UMCTLB_H_

void inicializarTLB(t_list* tlb, int cant);
int tlbLLena();
int buscarPosicionEnTLB(int id, int pagina);
frame* buscarFrameEnTLB(int id, int pagina);
int insertarEnTLB(int id, int pagina, frame* marco, int posicion);
tlb* removerDeTLB(int id, int pagina, int pos);
int puedeInsertar();
#endif /* SRC_UMCTLB_H_ */
