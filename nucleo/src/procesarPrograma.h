/*
 * procesarPrograma.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */
#include "estructurasNUCLEO.h"

#ifndef PROCESARPROGRAMA_H_
#define PROCESARPROGRAMA_H_


indiceCodigo* nuevoPrograma(char* instrucciones,t_list* instrucc);
t_dictionary* paginarIC(t_dictionary* codigoSinPaginar);
int cargarEnUMC(t_dictionary* codigoPaginado,t_list* instrucciones, int pagNecesarias, int socketUMC,int id);

#endif /* PROCESARPROGRAMA_H_ */
