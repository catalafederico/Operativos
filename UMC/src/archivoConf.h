/*
 * archivoConf.h
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#ifndef SRC_ARCHIVOCONF_H_
#define SRC_ARCHIVOCONF_H_

typedef struct {
	int PUERTO;
	char IP_SWAP[16];
	int PUERTO_SWAP;
	int MARCOS;
	int MARCO_SIZE;
	int MARCO_X_PROC;
	int ENTRADAS_TLB;
	int RETARDO;
} t_reg_config;

t_reg_config get_config_params();

#endif /* SRC_ARCHIVOCONF_H_ */
