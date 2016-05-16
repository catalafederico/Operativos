/*
 * umcCliente.h
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */

#ifndef SRC_UMCCLIENTE_H_
#define SRC_UMCCLIENTE_H_
#include "estructurasUMC.h"
void notificarASwapPrograma(id_programa* programaCreador, int socketSwap);
void notificarASwapFinPrograma(int id, int socketSwap);

#endif /* SRC_UMCCLIENTE_H_ */
