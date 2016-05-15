/*
 * umcMemoria.h
 *
 *  Created on: 9/5/2016
 *      Author: utnso
 */

#ifndef SRC_UMCMEMORIA_H_
#define SRC_UMCMEMORIA_H_

void* inicializarMemoria(t_reg_config* configuracionUMC);
int alocarPrograma(int paginasRequeridas, proceso* proceso_alocar);

#endif /* SRC_UMCMEMORIA_H_ */
