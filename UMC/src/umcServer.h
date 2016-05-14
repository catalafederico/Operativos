/*
 * umcServer.h
 *
 *  Created on: 9/5/2016
 *      Author: utnso
 */

#ifndef SRC_UMCSERVER_H_
#define SRC_UMCSERVER_H_
#include <sockets/socketServer.h>
#include "archivoConf.h"

typedef struct{
	t_reg_config configuracionUMC;
	void* memoriaPrincipal;
	int socketSwap;
}umcNucleo;

void ponerUmcAEscuchar(struct server* procesosServer,umcNucleo* umcConfg);

#endif /* SRC_UMCSERVER_H_ */
