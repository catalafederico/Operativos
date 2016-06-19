/*
 * bitMap.h
 *
 *  Created on: 18/6/2016
 *      Author: utnso
 */

#ifndef BITMAP_H_
#define BITMAP_H_
#include "swapEstrucuturas.h"

void inicializarBitMap();
void eliminarDelBitMapLasPaginasDelProceso(proceso* proceso);
void actualizarBitMap(proceso* proceso, int comienzoDelHueco);
void listarBitMap();
#endif /* BITMAP_H_ */
