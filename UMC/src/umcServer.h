/*
 * umcServer.h
 *
 *  Created on: 9/5/2016
 *      Author: utnso
 */

#ifndef SRC_UMCSERVER_H_
#define SRC_UMCSERVER_H_
#include "archivoConf.h"
#include "estructurasUMC.h"
#include <sockets/socketServer.h>


void ponerUmcAEscuchar(struct server* procesosServer,umcNucleo* umcConfg);

#endif /* SRC_UMCSERVER_H_ */
