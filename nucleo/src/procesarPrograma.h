/*
 * procesarPrograma.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef PROCESARPROGRAMA_H_
#define PROCESARPROGRAMA_H_

indiceCodigo* nuevoPrograma(char* instrucciones,t_list* instrucciones);
t_dictionary* paginarIC(t_dictionary* codigoSinPaginar);

#endif /* PROCESARPROGRAMA_H_ */
