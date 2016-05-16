/*
 * estructurasUMC.h
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */

#ifndef SRC_ESTRUCTURASUMC_H_
#define SRC_ESTRUCTURASUMC_H_

#include <commons/collections/dictionary.h>

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

typedef struct{
	int* id_programa;
	t_dictionary* pag_marco;
}proceso;

typedef struct{
	t_reg_config configuracionUMC;
	void* memoriaPrincipal;
	int socketSwap;
}umcNucleo;

typedef struct{
	umcNucleo* umcConfig;
	int socket;
}tempStruct;

typedef struct {
	int identificador;
	int paginas_requeridas;
} __attribute__((packed))
id_programa;



#endif /* SRC_ESTRUCTURASUMC_H_ */
