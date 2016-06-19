/*
 * umcCliente.h
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */

#ifndef SRC_UMCCLIENTE_H_
#define SRC_UMCCLIENTE_H_
#include "estructurasUMC.h"

void inicializarSwap(int* socket);
int notificarASwapPrograma(int id,int paginas);
void notificarASwapFinPrograma(int id);
void almacenarEnSwap(int id, int pagina, void* buffer);
void* solicitarEnSwap(int id, int pagina);

#endif /* SRC_UMCCLIENTE_H_ */
