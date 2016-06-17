/*
 * estructurasUMC.h
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */

#ifndef SRC_ESTRUCTURASUMC_H_
#define SRC_ESTRUCTURASUMC_H_

#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <netinet/in.h>

#define USADO 1;
#define NOUSADO 0;

typedef struct {
	int PUERTO;
	char* IP_SWAP;
	int PUERTO_SWAP;
	int MARCOS;
	int MARCO_SIZE;
	int MARCO_X_PROC;
	int ENTRADAS_TLB;
	int RETARDO;
	int ALGORITMO;
} t_reg_config;

typedef struct{
	int* id_programa;
	t_dictionary* pag_marco;
}proceso;

typedef struct{
	t_reg_config configuracionUMC;
	void* memoriaPrincipal;
	int socketSwap;
	t_log* loguer;
}umcNucleo;

typedef struct{
	umcNucleo* umcConfig;
	int socket;
}tempStruct;

typedef struct {
	int identificador;
	int paginas_requeridas;
}__attribute__((packed))
id_programa;

typedef struct {
	int nroMarco;
	int bit_uso;
	int modif;
}__attribute__((packed))
infoPagina;

typedef struct{
	int idProg;
	int pag;
	infoPagina* marco;
}tlb;

typedef struct{
	t_list* paginasMemoria; //Va a tener tipos relojElem
	int puntero;
}reloj;

typedef struct{
	int pag;
	infoPagina* marco;
}relojElem;
#endif /* SRC_ESTRUCTURASUMC_H_ */
