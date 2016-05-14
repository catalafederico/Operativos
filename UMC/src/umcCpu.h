/*
 * umcNucleo.h
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */

#ifndef SRC_UMCNUCLEO_H_
#define SRC_UMCNUCLEO_H_

typedef struct{
	t_reg_config configuracionUMC;
	void* memoriaPrincipal;
	int socketSwap;
}umcNucleo;


typedef struct{
	umcNucleo* umcConfig;
	int socket;
}tempStruct;

void* conexionCpu(tempStruct socketCpu);

#endif /* SRC_UMCNUCLEO_H_ */
