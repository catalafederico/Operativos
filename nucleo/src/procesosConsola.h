/*
 * procesosConsola.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef PROCESOSCONSOLA_H_
#define PROCESOSCONSOLA_H_

void *atender_conexion_consolas();
void *atender_consola(int *socket_desc);
void eliminar_proceso_del_sistema(int* tempId);


#endif /* PROCESOSCONSOLA_H_ */
